"""Local LR2 authoring tools for an external AI. No model/API calls or installation."""
from __future__ import annotations

import argparse
from contextlib import contextmanager
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import uuid

from skin_agent_assets import AssetError, validate_assets, apply_assets
from skin_agent_preview import (PreviewError, validate_state, default_states,
                                require_capability, render_state, diagnose, preview_suite)

SCENES = {"play7": 0, "play5": 1, "double14": 2, "double10": 3, "pms9": 4,
          "select": 5, "decide": 6, "result": 7, "battle7": 12,
          "battle5": 13, "battle9": 14, "course-result": 15}
LIMITS = {"x": (-8192, 8192), "y": (-8192, 8192), "w": (1, 8192),
          "h": (1, 8192), "size": (1, 8192), "a": (0, 255), "r": (0, 255),
          "g": (0, 255), "b": (0, 255), "angle": (-3600, 3600)}


class AgentError(ValueError):
    pass


def integer(value, low, high, label):
    if type(value) is not int or not low <= value <= high:
        raise AgentError(f"{label} must be an integer in [{low}, {high}].")
    return value


def validate_recipe(recipe):
    if not isinstance(recipe, dict):
        raise AgentError("Recipe must be a JSON object.")
    unknown = set(recipe) - {"version", "scene", "width", "height", "title", "maker", "objects", "assets"}
    if unknown:
        raise AgentError(f"Unknown recipe fields: {sorted(unknown)}")
    if type(recipe.get("version", 1)) is not int or recipe.get("version", 1) != 1:
        raise AgentError("Only recipe version 1 is supported; OLRskin format is unchanged.")
    scene = recipe.get("scene", "play7")
    if not isinstance(scene, str) or scene not in SCENES:
        raise AgentError(f"scene must be one of {', '.join(SCENES)}")
    result = {"version": 1, "scene": scene,
              "width": integer(recipe.get("width", 1280), 320, 1920, "width"),
              "height": integer(recipe.get("height", 720), 240, 1080, "height")}
    for key, default in (("title", "AI starter"), ("maker", "SkinEditor agent")):
        value = recipe.get(key, default)
        if not isinstance(value, str) or not 1 <= len(value) <= 120 or any(c in value for c in ",\r\n\0"):
            raise AgentError(f"{key} must be one CSV field of 1..120 characters.")
        try:
            value.encode("cp932", errors="strict")
        except UnicodeEncodeError as error:
            raise AgentError(f"{key} cannot be stored losslessly in CP932.") from error
        result[key] = value
    objects = recipe.get("objects", [])
    if not isinstance(objects, list) or len(objects) > 512:
        raise AgentError("objects must be an array of at most 512 edits.")
    result["objects"] = []
    seen = set()
    for item in objects:
        if not isinstance(item, dict) or set(item) != {"id", "set"}:
            raise AgentError("Each Object edit must contain exactly id and set.")
        object_id = item["id"]
        if not isinstance(object_id, str) or not re.fullmatch(r"[A-Za-z0-9_-]{1,96}", object_id) or object_id in seen:
            raise AgentError("Object IDs must be unique existing preset IDs.")
        seen.add(object_id)
        fields = item["set"]
        if not isinstance(fields, dict) or not fields or set(fields) - LIMITS.keys():
            raise AgentError(f"set must contain supported layout/color fields: {', '.join(LIMITS)}")
        result["objects"].append({"id": object_id, "set": {
            key: integer(value, *LIMITS[key], f"{object_id}.{key}") for key, value in fields.items()}})
    if "assets" in recipe:
        try:
            result["assets"] = validate_assets(recipe["assets"])
        except AssetError as error:
            raise AgentError(str(error)) from error
    return result


@contextmanager
def scratch_directory(parent: Path):
    parent = parent.resolve()
    parent.mkdir(parents=True, exist_ok=True)
    directory = parent / ("run-" + uuid.uuid4().hex)
    directory.mkdir()
    try:
        yield directory
    finally:
        # Only remove the fresh, owned request directory; never a recipe path.
        if directory.exists() and directory.resolve().parent == parent:
            shutil.rmtree(directory)


class NativeEditor:
    def __init__(self, executable: Path, scratch: Path | None = None):
        self.executable = executable.resolve()
        if not self.executable.is_file():
            raise AgentError("Pass --editor pointing to the AI_experimental utility build.")
        self.scratch = scratch or Path(__file__).resolve().parents[1] / ".build" / "agent-requests"

    def request(self, operation: str, *arguments) -> dict:
        with scratch_directory(self.scratch) as directory:
            report = directory / "report.json"
            process = subprocess.run(
                [str(self.executable), "--agent-" + operation, *(str(arg) for arg in arguments), str(report)],
                cwd=directory, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=60,
                creationflags=subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0)
            if not report.exists():
                raise AgentError(f"Native {operation} returned {process.returncode} without a report. Check the utility build and Direct3D availability.")
            result = json.loads(report.read_text(encoding="utf-8"))
            if process.returncode or not result.get("ok"):
                raise AgentError(result.get("error", f"Native {operation} failed: {process.returncode}"))
            return result


def field_columns(schema):
    return {command["name"]: {
        field["name"].lstrip("$").split("(", 1)[0].split("//", 1)[0].strip().lower(): field["column"]
        for field in command["fields"] if field["name"] and not field["name"].startswith("(")}
        for command in schema["commands"]}


def apply_object_edits(text: str, edits, columns) -> str:
    """Edit only the newly generated single-file preset, using native schema columns."""
    requested = {edit["id"]: edit["set"] for edit in edits}
    matched = {object_id: 0 for object_id in requested}
    lines = text.splitlines(keepends=True)
    current_id = None
    for index, line in enumerate(lines):
        body = line.rstrip("\r\n")
        if body.startswith("$SE_OBJECT_ID,"):
            current_id = body.split(",", 1)[1]
        elif body.startswith("#DST_") and current_id in requested:
            tokens = body.split(",")
            command = tokens[0]
            for field, value in requested[current_id].items():
                column = columns.get(command, {}).get(field)
                if column is None or column >= len(tokens):
                    raise AgentError(f"Native schema has no editable {field} column in {command} for {current_id}.")
                tokens[column] = str(value)
            lines[index] = ",".join(tokens) + line[len(body):]
            matched[current_id] += 1
    missing = [object_id for object_id, count in matched.items() if not count]
    if missing:
        raise AgentError(f"No destination rows found for Object IDs: {missing}")
    return "".join(lines)


def verify_edits(inspection, edits, columns):
    for edit in edits:
        destinations = [command for obj in inspection["objects"] if obj["id"] == edit["id"]
                        for command in obj["commands"] if command["command"].startswith("#DST_")]
        if not destinations:
            raise AgentError(f"Native readback lost Object {edit['id']}.")
        for row in destinations:
            for field, expected in edit["set"].items():
                column = columns[row["command"]][field] - 1
                if column >= len(row["values"]) or row["values"][column] != str(expected):
                    raise AgentError(f"Native readback disagrees with {edit['id']}.{field}.")


def write_json(path: Path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def build_skin(editor: NativeEditor, recipe, output: Path, preview=True, recipe_dir: Path | None = None):
    recipe = validate_recipe(recipe)
    output = output.absolute()
    if output.exists() or output.is_symlink():
        raise AgentError("Output already exists. Choose a new folder; there is no overwrite mode.")
    output.parent.mkdir(parents=True, exist_ok=True)
    stage = output.parent / (".skin-agent-" + uuid.uuid4().hex)
    schema = editor.request("schema")
    columns = field_columns(schema)
    try:
        created = editor.request("create", SCENES[recipe["scene"]], recipe["width"], recipe["height"],
                                 recipe["title"], recipe["maker"], stage)
        skin = (stage / created["skin"]).resolve()
        if not skin.is_relative_to(stage.resolve()) or not skin.is_file():
            raise AgentError("Native creator returned an invalid output path.")
        before = editor.request("inspect", skin)
        known_ids = {obj["id"] for obj in before["objects"]}
        missing = {item["id"] for item in recipe["objects"]} - known_ids
        if missing:
            raise AgentError(f"Unknown preset Object IDs: {sorted(missing)}; inspect a base preset first.")
        original = skin.read_bytes().decode("cp932", errors="strict")
        modified = apply_object_edits(original, recipe["objects"], columns)
        try:
            modified, assets = apply_assets(modified, recipe.get("assets", []), recipe_dir or Path.cwd(), stage, columns)
        except AssetError as error:
            raise AgentError(str(error)) from error
        skin.write_bytes(modified.encode("cp932", errors="strict"))
        inspection = editor.request("inspect", skin)
        if inspection["object_count"] != before["object_count"] + len(assets["added_object_ids"]):
            raise AgentError("Object count changed unexpectedly after layout/color edits.")
        verify_edits(inspection, recipe["objects"], columns)
        missing_assets = set(assets["added_object_ids"]) - {obj["id"] for obj in inspection["objects"]}
        if missing_assets:
            raise AgentError(f"Native parser did not register imported image Objects: {sorted(missing_assets)}")
        verify_asset_readback(inspection, assets, columns, modified, skin)
        if preview:
            editor.request("render", skin, stage / "preview.png")
            if not (stage / "preview.png").is_file():
                raise AgentError("Preview was not produced.")
        write_json(stage / "recipe.json", recipe)
        # Reports leave the staging directory together with the skin. Their
        # locators must point at the final files, not the soon-renamed stage.
        for obj in inspection["objects"]:
            for row in obj["commands"]:
                if row.get("file"):
                    owner = Path(row["file"]).resolve()
                    if owner.is_relative_to(stage.resolve()):
                        row["file"] = str(output / owner.relative_to(stage.resolve()))
        write_json(stage / "objects.json", inspection)
        if assets["assets"]:
            write_json(stage / "assets.json", assets)
        relative_skin = skin.relative_to(stage.resolve()).as_posix()
        result = {"ok": True, "api": "lr2-agent/v1", "skin": str(output / relative_skin),
                  "object_count": inspection["object_count"], "parser_checked": True,
                  "preview": str(output / "preview.png") if preview else None,
                  "recipe": str(output / "recipe.json"), "objects": str(output / "objects.json")}
        if assets["assets"]:
            result["assets"] = str(output / "assets.json")
        write_json(stage / "build-report.json", result)
        # Windows rename is atomic and refuses an existing destination, including
        # one created concurrently. Staging never changes the caller's original.
        if output.exists() or output.is_symlink():
            raise AgentError("Output appeared during generation; it was not replaced.")
        stage.rename(output)
        if not (output / relative_skin).is_file():
            raise AgentError("Final output readback failed.")
        return result
    finally:
        if stage.exists() and stage.resolve().parent == output.parent.resolve():
            shutil.rmtree(stage)


def verify_asset_readback(inspection, assets, columns, text, skin):
    # Native Objects split some indexed variants even though their authoring
    # annotation has one ID. Match the real file/line back to that annotation.
    row_owners, owner = {}, None
    for line, contents in enumerate(text.splitlines(), 1):
        if contents.startswith("$SE_OBJECT_ID,"):
            owner = contents.split(",", 1)[1]
        row_owners[line] = owner
    sources = [row for obj in inspection["objects"] for row in obj["commands"] if row["command"].startswith("#SRC_")]
    for asset in assets["assets"]:
        expected = {key: asset[key] for key in ("gr", "x", "y", "w", "h", "div_x", "div_y", "cycle")}
        for binding in asset["bindings"]:
            fields = dict(expected)
            if "index" in binding:
                fields["index"] = binding["index"]
            matches = []
            for row in sources:
                if row["command"] != binding["command"]:
                    continue
                if row_owners.get(row.get("line")) != binding["object"] or Path(row.get("file", "")).resolve() != skin.resolve():
                    continue
                native = columns[row["command"]]
                if all(key in native and native[key]-1 < len(row["values"]) and
                       row["values"][native[key]-1] == str(value) for key, value in fields.items()):
                    matches.append(row)
            if len(matches) != 1:
                raise AgentError(f"Native asset binding readback failed for {asset['id']}/{binding['command']}.")


def read_json(path):
    if path.stat().st_size > 1024 * 1024:
        raise AgentError("JSON request exceeds the 1 MiB limit.")
    return json.loads(path.read_text(encoding="utf-8-sig"))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--editor", type=Path, required=True, help="AI_experimental SkinEditor_DX9.exe")
    commands = parser.add_subparsers(dest="operation", required=True)
    schema = commands.add_parser("schema", help="Get native command/column definitions as JSON")
    schema.add_argument("--command", help="Return one command, e.g. #DST_NOTE")
    schema.add_argument("--values", action="store_true", help="Include native symbolic value tables")
    inspect = commands.add_parser("inspect", help="Read Objects with the native parser")
    inspect.add_argument("skin", type=Path)
    inspect.add_argument("--object", dest="object_id")
    build = commands.add_parser("build", help="Create a new preset from a JSON recipe")
    build.add_argument("recipe", type=Path)
    build.add_argument("--out", type=Path, required=True)
    build.add_argument("--no-preview", action="store_true")
    render = commands.add_parser("render", help="Render an existing skin to a new PNG")
    render.add_argument("skin", type=Path)
    render.add_argument("--out", type=Path, required=True)
    render.add_argument("--state", type=Path, help="Fixed PLAY preview state JSON")
    suite = commands.add_parser("preview-suite", help="Render PLAY states, diagnostic JSON and an HTML gallery")
    suite.add_argument("skin", type=Path)
    suite.add_argument("--states", type=Path, help="JSON array; default is the 14-state validation suite")
    suite.add_argument("--out", type=Path, required=True)
    diagnostic = commands.add_parser("diagnose", help="Explain visibility with native owner files and source line numbers")
    diagnostic.add_argument("skin", type=Path)
    diagnostic.add_argument("--state", type=Path, help="Optional fixed PLAY state JSON")
    target = diagnostic.add_mutually_exclusive_group()
    target.add_argument("--object", dest="object_id")
    target.add_argument("--object-index", type=int)
    commands.add_parser("states", help="Print the default fixed-state suite as JSON")
    args = parser.parse_args(argv)
    try:
        editor = NativeEditor(args.editor)
        if args.operation == "schema":
            result = editor.request("schema")
            if args.command:
                result["commands"] = [entry for entry in result["commands"] if entry["name"].upper() == args.command.upper()]
                if not result["commands"]:
                    raise AgentError("Command is not in the native schema.")
            if not args.values:
                result.pop("value_sets", None)
        elif args.operation == "inspect":
            result = editor.request("inspect", args.skin.resolve())
            if args.object_id:
                result["objects"] = [entry for entry in result["objects"] if entry["id"] == args.object_id]
                if not result["objects"]:
                    raise AgentError("Object ID was not found.")
        elif args.operation == "build":
            result = build_skin(editor, read_json(args.recipe), args.out, not args.no_preview, args.recipe.resolve().parent)
        elif args.operation == "states":
            result = default_states()
        elif args.operation == "preview-suite":
            result = preview_suite(editor, args.skin, read_json(args.states) if args.states else default_states(), args.out)
        elif args.operation == "diagnose":
            result = diagnose(editor, args.skin, read_json(args.state) if args.state else None, args.object_id, args.object_index)
        else:
            target = args.out.absolute()
            if target.exists() or target.is_symlink():
                raise AgentError("Preview output already exists.")
            target.parent.mkdir(parents=True, exist_ok=True)
            with scratch_directory(editor.scratch) as temporary:
                image = temporary / "preview.png"
                if args.state:
                    require_capability(editor, "state_render")
                    result = render_state(editor, args.skin, read_json(args.state), image)
                else:
                    result = editor.request("render", args.skin.resolve(), image)
                with image.open("rb") as source, target.open("xb") as destination:
                    shutil.copyfileobj(source, destination)
            result = {"ok": True, "preview": str(target), "object_count": result["object_count"],
                      **({"state": result["state"], "diagnostics": result["diagnostics"]} if args.state else {})}
        print(json.dumps(result, ensure_ascii=False, indent=2))
        return 1 if isinstance(result, dict) and not result.get("ok", True) else 0
    except (AgentError, AssetError, PreviewError, OSError, UnicodeError, json.JSONDecodeError, subprocess.SubprocessError) as error:
        print(json.dumps({"ok": False, "error": str(error)}, ensure_ascii=False))
        return 1


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
