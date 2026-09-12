"""Fixed-state preview suites for external coding agents. No model calls."""
from __future__ import annotations

import html
import json
from pathlib import Path
import re
import shutil
import subprocess
import uuid

JUDGES = {"none": -1, "poor": 0, "miss": 1, "bad": 2, "good": 3, "great": 4, "perfect": 5}
NOTES = {"all": 0, "normal": 1, "long": 2, "mine": 3, "none": 4}
NUMBERS = {"time_ms": (0, 3600000, 1000), "gauge": (0, 100, 80),
           "score": (0, 999999999, 123456), "exscore": (0, 99999999, 1234),
           "combo": (0, 9999999, 123), "max_combo": (0, 9999999, 321), "bpm": (1, 9999, 150)}


class PreviewError(ValueError):
    pass


def validate_state(state):
    if not isinstance(state, dict) or set(state) - (set(NUMBERS) | {"name", "judge", "notes", "timers"}):
        raise PreviewError("State fields: name, time_ms, gauge, score, exscore, combo, max_combo, bpm, judge, notes, timers.")
    result = {"name": state.get("name", "preview")}
    if not isinstance(result["name"], str) or not re.fullmatch(r"[A-Za-z0-9_-]{1,64}", result["name"]):
        raise PreviewError("State name must be 1..64 ASCII letters, digits, underscores or hyphens.")
    if result["name"].upper() in {"CON", "PRN", "AUX", "NUL", *(f"COM{i}" for i in range(1, 10)), *(f"LPT{i}" for i in range(1, 10))}:
        raise PreviewError("State name is a reserved Windows device name.")
    for field, (low, high, default) in NUMBERS.items():
        value = state.get(field, default)
        if type(value) is not int or not low <= value <= high:
            raise PreviewError(f"State {field} must be an integer in [{low}, {high}].")
        result[field] = value
    for field, choices, default in (("judge", JUDGES, "none"), ("notes", NOTES, "all")):
        value = state.get(field, default)
        if not isinstance(value, str) or value not in choices:
            raise PreviewError(f"State {field} must be one of {', '.join(choices)}.")
        result[field] = value
    timers = state.get("timers", {})
    if not isinstance(timers, dict) or len(timers) > 128:
        raise PreviewError("timers must be an object of at most 128 timer IDs to elapsed milliseconds.")
    result["timers"] = {}
    for key, value in timers.items():
        if not isinstance(key, str) or not key.isascii() or not key.isdecimal() or str(int(key)) != key or not 0 <= int(key) <= 499:
            raise PreviewError("Timer IDs must be canonical strings in 0..499.")
        if type(value) is not int or not -1 <= value <= 3600000:
            raise PreviewError("Timer elapsed must be -1 (inactive) or 0..3600000 ms.")
        result["timers"][key] = value
    return result


def state_wire(state):
    state = validate_state(state)
    values = [state[field] for field in NUMBERS] + [JUDGES[state["judge"]], NOTES[state["notes"]]]
    timers = ";".join(f"{key}={value}" for key, value in sorted(state["timers"].items(), key=lambda item: int(item[0])))
    return ",".join(map(str, values)), timers


def default_states():
    states = [{"name": f"gauge-{gauge}", "gauge": gauge} for gauge in (0, 80, 100)]
    states.append({"name": "max-digits", "score": 999999999, "exscore": 99999999,
                   "combo": 9999999, "max_combo": 9999999, "bpm": 9999, "judge": "perfect"})
    states.extend({"name": "judge-"+judge, "judge": judge} for judge in JUDGES if judge != "none")
    states.extend({"name": "notes-"+notes, "notes": notes} for notes in ("normal", "long", "mine"))
    states.append({"name": "keys-down", "timers": {str(key): 100 for key in range(100, 108)}})
    return [validate_state(state) for state in states]


def validate_states(value):
    if not isinstance(value, list) or not 1 <= len(value) <= 32:
        raise PreviewError("A preview suite must contain 1..32 states.")
    states = [validate_state(state) for state in value]
    if len({state["name"].casefold() for state in states}) != len(states):
        raise PreviewError("State names must be unique, ignoring Windows filename case.")
    return states


def require_capability(editor, name):
    if name not in editor.request("schema").get("capabilities", []):
        raise PreviewError(f"This executable lacks {name}; build the current AI_experimental utility.")


def render_state(editor, skin, state, image):
    state = validate_state(state)
    result = editor.request("render-state", skin.resolve(), image, *state_wire(state))
    result["state"] = state
    return result


def diagnose(editor, skin, state=None, object_id=None, object_index=None):
    require_capability(editor, "visibility_diagnostics")
    result = editor.request("diagnose", skin.resolve(), *(state_wire(state) if state is not None else ("-", "")))
    if state is not None:
        result["state"] = validate_state(state)
    diagnostics = result["diagnostics"]
    if object_id is not None or object_index is not None:
        selected = [obj for obj in diagnostics["objects"] if
                    (object_id is None or obj["id"] == object_id) and
                    (object_index is None or obj["index"] == object_index)]
        if not selected:
            raise PreviewError("Object was not found; use inspect for IDs or object indices.")
        indices = {obj["index"] for obj in selected}
        diagnostics["objects"] = selected
        diagnostics["issues"] = [issue for issue in diagnostics["issues"] if issue["object_index"] in indices]
        result["objects"] = [obj for obj in result["objects"] if obj["index"] in indices]
    return result


def preview_suite(editor, skin, states, output):
    states = validate_states(states)
    output = output.absolute()
    if output.exists() or output.is_symlink():
        raise PreviewError("Preview output folder already exists. Choose a new folder.")
    require_capability(editor, "state_render")
    output.parent.mkdir(parents=True, exist_ok=True)
    stage = output.parent / (".preview-suite-"+uuid.uuid4().hex)
    stage.mkdir()
    reports = []
    try:
        for state in states:
            name = state["name"]
            try:
                result = render_state(editor, skin, state, stage / (name+".png"))
                # Every state's diagnostic evidence travels with its actual render.
                (stage / (name+".json")).write_text(json.dumps(result, ensure_ascii=False, indent=2)+"\n", encoding="utf-8")
                reports.append({"name": name, "ok": True, "image": name+".png", "report": name+".json",
                                "issues": len(result["diagnostics"]["issues"])})
            except (ValueError, OSError, subprocess.SubprocessError) as error:
                reports.append({"name": name, "ok": False, "error": str(error)})
        report = {"ok": all(item["ok"] for item in reports), "api": "lr2-agent/v1", "kind": "fixed-state-preview",
                  "skin": str(skin.resolve()), "states": reports, "lr2_gameplay_checked": False}
        (stage / "states.json").write_text(json.dumps(states, indent=2)+"\n", encoding="utf-8")
        (stage / "report.json").write_text(json.dumps(report, ensure_ascii=False, indent=2)+"\n", encoding="utf-8")
        cards = []
        for item in reports:
            name = html.escape(item["name"])
            if item["ok"]:
                body = f'<a href="{item["image"]}"><img src="{item["image"]}" alt="{name}"></a><p><a href="{item["report"]}">Diagnostics ({item["issues"]})</a></p>'
            else:
                body = '<p class="error">'+html.escape(item["error"])+"</p>"
            cards.append(f"<article><h2>{name}</h2>{body}</article>")
        (stage / "index.html").write_text('''<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>Skin preview states</title><style>body{background:#11171d;color:#e4ebf2;font:16px system-ui;margin:24px}main{display:grid;grid-template-columns:repeat(auto-fit,minmax(480px,1fr));gap:20px}article{background:#1b2630;padding:16px;border:1px solid #395064}h2{font-size:18px}img{width:100%;height:auto}a{color:#8cd5ff}.error{color:#ffad9e}@media(max-width:540px){main{grid-template-columns:1fr}}</style>
<h1>Fixed-state skin previews</h1><p>Synthetic editor states. Click an image for full size; diagnostics link to source files and lines. This is not an LR2 gameplay recording.</p><main>'''+"".join(cards)+"</main>", encoding="utf-8")
        if output.exists():
            raise PreviewError("Output appeared during rendering; it was not replaced.")
        stage.rename(output)
        report["gallery"] = str(output / "index.html")
        report["report"] = str(output / "report.json")
        return report
    finally:
        if stage.exists() and stage.resolve().parent == output.parent.resolve():
            shutil.rmtree(stage)
