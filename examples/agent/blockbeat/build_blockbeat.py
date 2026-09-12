"""Build the BLOCKBEAT 7K example with the native agent, then author its PNG atlas.

The landscape is copied unchanged. Pixel UI/sprites are drawn from code; LR2 does
the final compositing. This is an example authoring script, not a new skin format.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
import random
import shutil
import sys

from PIL import Image, ImageDraw

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(ROOT / "tools"))
from skin_agent import NativeEditor, build_skin, field_columns, write_json

THEME = "LR2files/Theme/BLOCKBEAT-Overworld-v01"
GREEN = "#b5e77d"
WHITE = "#edf3dc"
CYAN = "#6fe2db"
GOLD = "#f2c878"

# Original 5x7 bitmap lettering, kept in source so no system font is required.
GLYPHS = dict(zip("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:-/+.%", [
    "01110/10001/10001/11111/10001/10001/10001", "11110/10001/10001/11110/10001/10001/11110",
    "01111/10000/10000/10000/10000/10000/01111", "11110/10001/10001/10001/10001/10001/11110",
    "11111/10000/10000/11110/10000/10000/11111", "11111/10000/10000/11110/10000/10000/10000",
    "01111/10000/10000/10111/10001/10001/01111", "10001/10001/10001/11111/10001/10001/10001",
    "11111/00100/00100/00100/00100/00100/11111", "00111/00010/00010/00010/10010/10010/01100",
    "10001/10010/10100/11000/10100/10010/10001", "10000/10000/10000/10000/10000/10000/11111",
    "10001/11011/10101/10101/10001/10001/10001", "10001/11001/10101/10011/10001/10001/10001",
    "01110/10001/10001/10001/10001/10001/01110", "11110/10001/10001/11110/10000/10000/10000",
    "01110/10001/10001/10001/10101/10010/01101", "11110/10001/10001/11110/10100/10010/10001",
    "01111/10000/10000/01110/00001/00001/11110", "11111/00100/00100/00100/00100/00100/00100",
    "10001/10001/10001/10001/10001/10001/01110", "10001/10001/10001/10001/10001/01010/00100",
    "10001/10001/10001/10101/10101/10101/01010", "10001/10001/01010/00100/01010/10001/10001",
    "10001/10001/01010/00100/00100/00100/00100", "11111/00001/00010/00100/01000/10000/11111",
    "01110/10001/10011/10101/11001/10001/01110", "00100/01100/00100/00100/00100/00100/01110",
    "01110/10001/00001/00010/00100/01000/11111", "11110/00001/00001/01110/00001/00001/11110",
    "00010/00110/01010/10010/11111/00010/00010", "11111/10000/10000/11110/00001/00001/11110",
    "01110/10000/10000/11110/10001/10001/01110", "11111/00001/00010/00100/01000/01000/01000",
    "01110/10001/10001/01110/10001/10001/01110", "01110/10001/10001/01111/00001/00001/01110",
    "00000/00100/00100/00000/00100/00100/00000", "00000/00000/00000/11111/00000/00000/00000",
    "00001/00001/00010/00100/01000/10000/10000", "00000/00100/00100/11111/00100/00100/00000",
    "00000/00000/00000/00000/00000/00100/00100", "11001/11010/00010/00100/01000/01011/10011",
]))


def label(draw, x, y, text, scale=2, color=WHITE, shadow=True):
    if shadow:
        label(draw, x + scale, y + scale, text, scale, "#080f0d", False)
    for character in text:
        if character != " ":
            for yy, row in enumerate(GLYPHS[character].split("/")):
                for xx, value in enumerate(row):
                    if value == "1":
                        draw.rectangle((x + xx * scale, y + yy * scale,
                                        x + (xx + 1) * scale - 1, y + (yy + 1) * scale - 1), fill=color)
        x += 6 * scale


def box(draw, x, y, w, h, fill, light="#677567", dark="#101815", edge=3):
    draw.rectangle((x, y, x+w-1, y+h-1), fill=dark)
    draw.rectangle((x, y, x+w-edge-1, y+h-edge-1), fill=light)
    draw.rectangle((x+edge, y+edge, x+w-edge-1, y+h-edge-1), fill=fill)


def cube(draw, x, y, s=4):
    draw.polygon([(x, y+3*s), (x+6*s, y), (x+12*s, y+3*s), (x+6*s, y+6*s)], fill="#91cf55")
    draw.polygon([(x, y+3*s), (x+6*s, y+6*s), (x+6*s, y+13*s), (x, y+10*s)], fill="#715235")
    draw.polygon([(x+6*s, y+6*s), (x+12*s, y+3*s), (x+12*s, y+10*s), (x+6*s, y+13*s)], fill="#443925")
    draw.polygon([(x, y+3*s), (x+6*s, y+6*s), (x+6*s, y+8*s), (x, y+5*s)], fill="#568c35")
    draw.polygon([(x+6*s, y+6*s), (x+12*s, y+3*s), (x+12*s, y+5*s), (x+6*s, y+8*s)], fill="#396834")
    for xx, yy in [(2, 2), (6, 2), (8, 3)]:
        draw.rectangle((x+xx*s, y+yy*s, x+(xx+1)*s, y+(yy+1)*s), fill="#a6dd6d")


def torch(draw, x, y):
    draw.rectangle((x+5, y+17, x+11, y+42), fill="#704b2b")
    draw.rectangle((x+5, y+17, x+7, y+37), fill="#b68b50")
    draw.rectangle((x+2, y+6, x+14, y+19), fill="#d5672f")
    draw.rectangle((x+5, y, x+11, y+15), fill="#f9c464")
    draw.rectangle((x+7, y+6, x+9, y+13), fill="#fff0b7")


def make_ui():
    ui = Image.new("RGBA", (1280, 720), "#14241d")
    d = ImageDraw.Draw(ui)
    rng = random.Random(721)
    # Quiet block masonry around the play field; no texture inside the lanes.
    for y in range(0, 720, 24):
        for x in range(-24 if y % 48 else 0, 1280, 48):
            shade = rng.randrange(0, 6)
            d.rectangle((x+1, y+1, x+46, y+22), fill=(22+shade, 36+shade, 28+shade))
    d.rectangle((0, 0, 1279, 88), fill="#172b20")
    d.rectangle((0, 88, 1279, 91), fill="#477d3f")
    d.rectangle((0, 92, 1279, 95), fill="#0b1814")
    cube(d, 32, 17)
    label(d, 99, 20, "BLOCKBEAT", 5, GREEN)
    label(d, 101, 65, "OVERWORLD / 7KEY + SCRATCH", 1, "#a7b89c")
    label(d, 590, 23, "MINE THE RHYTHM.", 3, WHITE)
    label(d, 591, 59, "GRASS / STONE / DIAMOND", 1, "#9cae94")
    box(d, 1117, 24, 128, 42, "#293e2b", "#819963")
    label(d, 1136, 37, "7K + S", 2, GOLD)
    box(d, 28, 102, 530, 522, "#344035", "#83907a", "#0a130e", 5)
    # Chiseled blocks on each edge make the highway read as a mine shaft.
    for y in range(129, 611, 32):
        box(d, 31, y, 15, 30, "#465044", "#626d58", "#1a251c", 2)
        box(d, 540, y, 14, 30, "#465044", "#626d58", "#1a251c", 2)
    d.rectangle((50, 123, 531, 569), fill="#080f10")
    starts = [52] + [122 + 58*i for i in range(7)]
    widths = [70] + [58]*7
    for i, (x, w) in enumerate(zip(starts, widths)):
        fill = "#211f19" if i == 0 else ("#172525" if i % 2 else "#102023")
        d.rectangle((x+1, 126, x+w-2, 553), fill=fill)
        d.line((x+w-1, 126, x+w-1, 553), fill="#34483e")
        for y in (218, 312, 406, 500):
            d.line((x+2, y, x+w-3, y), fill="#263631")
        keyfill = "#a57839" if i == 0 else ("#a5b59e" if i%2 else "#287b7c")
        box(d, x+2, 576, w-4, 32, keyfill, "#d0d8b6" if i%2 else "#4d9c91", "#101815", 3)
        word = "SCR" if i == 0 else str(i)
        label(d, x+(w-len(word)*12)//2, 584, word, 2, "#14221d", False)
    label(d, 56, 109, "7K / MINING SHAFT", 1, WHITE)
    label(d, 400, 109, "PLAYER 01", 1, GREEN)
    # Landscape is a separate immutable image below this transparent viewport.
    box(d, 574, 102, 678, 424, "#25382c", "#728362", "#0a130e", 5)
    label(d, 596, 121, "01  OVERWORLD", 2, GREEN)
    label(d, 1177, 124, "BGA", 1, "#a4b896")
    box(d, 587, 151, 650, 367, "#12211b", "#101b14", "#68825a", 4)
    d.rectangle((592, 156, 1233, 516), fill=(0, 0, 0, 0))
    # Small root/grass pixels break the industrial frames without obscuring art.
    for x in range(586, 1240, 9):
        h = rng.choice([2, 3, 5, 7])
        d.rectangle((x, 103, x+5, 103+h), fill=rng.choice(["#638c45", "#7ca54e", "#466939"]))
    box(d, 28, 634, 530, 66, "#223326", "#657452", "#0a130e", 4)
    label(d, 50, 646, "GROOVE", 2, GREEN)
    label(d, 524, 648, "%", 1, GREEN)
    # 50 live gauge cells use 9px spacing; every five cells has a hotbar divider.
    box(d, 49, 669, 456, 20, "#0b160e", "#102016", "#526644", 2)
    for i in range(11):
        d.line((51+45*i, 670, 51+45*i, 687), fill="#34482b")
    box(d, 574, 540, 678, 160, "#1b2c23", "#657452", "#0a130e", 4)
    d.line((856, 554, 856, 634), fill="#41503a", width=2)
    d.line((1046, 554, 1046, 634), fill="#41503a", width=2)
    label(d, 596, 558, "EX SCORE", 2, GREEN)
    label(d, 880, 558, "BPM", 2, GOLD)
    label(d, 1070, 558, "COMBO", 2, CYAN)
    d.line((594, 642, 1231, 642), fill="#44573c")
    label(d, 596, 659, "FAST", 1, CYAN)
    label(d, 813, 659, "SLOW", 1, GOLD)
    label(d, 1038, 659, "MAX", 1, GREEN)
    torch(d, 563, 552)
    return ui


class Atlas:
    def __init__(self):
        self.image = Image.new("RGBA", (1280, 1536))
        self.image.paste(make_ui(), (0, 0))
        self.x, self.y, self.row_h = 0, 724, 0
        self.rects = {"ui": (0, 0, 1280, 720)}

    def add(self, name, image):
        w, h = image.size
        if self.x + w > 1280:
            self.x, self.y, self.row_h = 0, self.y + self.row_h + 2, 0
        if self.y + h > self.image.height:
            raise ValueError("Atlas overflow")
        self.image.paste(image, (self.x, self.y))
        self.rects[name] = (self.x, self.y, w, h)
        self.x += w + 2
        self.row_h = max(self.row_h, h)


def sprite(w, h, fill, highlight, shadow):
    img = Image.new("RGBA", (w, h))
    box(ImageDraw.Draw(img), 0, 0, w, h, fill, highlight, shadow, 2)
    return img


def make_atlas():
    atlas = Atlas()
    for name, fill, high, low in [
        ("quartz", "#c8dcca", "#f5ffe4", "#5f817c"),
        ("diamond", "#41bfcc", "#a9fff2", "#236376"),
        ("gold", "#e4a844", "#ffdfa0", "#845831"),
        ("mine", "#b83f44", "#fa8670", "#591f2d"),
        ("ln_start", "#58ca76", "#caffb5", "#255a44"),
        ("ln_end", "#a8e286", "#e0ffc0", "#367048"),
    ]:
        img = sprite(64, 14, fill, high, low)
        d = ImageDraw.Draw(img)
        if name == "mine":
            label(d, 29, 3, "X", 1, "#381725", False)
        else:
            for x in (8, 27, 45):
                d.rectangle((x, 4, x+5, 6), fill=high)
        atlas.add(name, img)
    img = sprite(64, 14, "#327b58", "#65be7c", "#204535")
    ImageDraw.Draw(img).rectangle((27, 0, 36, 13), fill="#65be7c")
    atlas.add("ln_body", img)
    atlas.add("line", Image.new("RGBA", (476, 1), "#3d5147"))
    atlas.add("judge_line", sprite(476, 5, "#ddaa57", "#ffeac0", "#644b30"))
    bomb = Image.new("RGBA", (60*6, 60))
    d = ImageDraw.Draw(bomb)
    for frame in range(6):
        r = 5 + frame*4
        for k in range(8):
            a = math.pi*k/4
            x, y = frame*60+30+int(r*math.cos(a)), 30+int(r*math.sin(a))
            sz = max(2, 5-frame//2)
            d.rectangle((x, y, x+sz, y+sz), fill=(150, 245, 175, 255-frame*30))
    atlas.add("bomb", bomb)
    for i, (word, color) in enumerate(zip(["POOR", "POOR", "BAD", "GOOD", "GREAT", "PERFECT"],
                                          ["#e0ac99", "#e0ac99", "#efb466", GREEN, CYAN, GOLD])):
        img = Image.new("RGBA", (280, 30))
        label(ImageDraw.Draw(img), (280-len(word)*24)//2, 0, word, 4, color)
        atlas.add(f"judge_{i}", img)
    digits = Image.new("RGBA", (240, 30))
    for n in range(10):
        label(ImageDraw.Draw(digits), n*24, 0, str(n), 4, WHITE)
    atlas.add("digits", digits)
    gauge = Image.new("RGBA", (36, 14))
    for i, (fill, high, low) in enumerate([
        ("#799c40", "#c2e47c", "#466137"), ("#a8de62", "#ddffa0", "#5c8036"),
        ("#263925", "#385135", "#14231c"), ("#35482c", "#4d6338", "#1c2b1d")]):
        box(ImageDraw.Draw(gauge), i*9, 0, 8, 14, fill, high, low, 1)
    atlas.add("gauge", gauge)
    beam = Image.new("RGBA", (64, 104))
    d = ImageDraw.Draw(beam)
    for y in range(104):
        d.line((0, y, 63, y), fill=(112, 239, 182, int(90*(y/103)**2)))
    atlas.add("key_beam", beam)
    return atlas


def dst(command, x, y, w, h, **extra):
    values = [0, 0, x, y, w, h, 0, 255, 255, 255, 255, 1, 0, 0, 0, 0, 0, 0, 0, 0]
    columns = {"blend": 11, "timer": 16, "loop": 15, "time": 1}
    for key, value in extra.items():
        values[columns[key]] = value
    return command + "," + ",".join(map(str, values))


def image_object(identity, gr, rect, target):
    return [f"$SE_OBJECT_NAME,{identity}", f"$SE_OBJECT_ID,{identity}",
            "#SRC_IMAGE,0," + ",".join(map(str, (gr, *rect, 1, 1, 0, 0, 0, 0, 0))),
            dst("#DST_IMAGE", *target)]


def build(editor_path, output):
    editor = NativeEditor(editor_path)
    schema = field_columns(editor.request("schema"))
    starts, widths = [52] + [122+58*i for i in range(7)], [70] + [58]*7
    edits = [{"id": "preset_bga", "set": {"x": 592, "y": 156, "w": 642, "h": 361}}]
    for i, (x, w) in enumerate(zip(starts, widths)):
        edits += [{"id": f"preset_note_{i}", "set": {"x": x+2, "y": 554, "w": w-4, "h": 14}},
                  {"id": f"preset_bomb_{i}", "set": {"x": x+(w-60)//2, "y": 526}}]
    for identity, values in {
        "preset_line_0": {"x": 52, "y": 554, "w": 476, "h": 1},
        "preset_judgeline_0": {"x": 52, "y": 554, "w": 476, "h": 5},
        "preset_nowjudge_0": {"x": 150, "y": 447, "w": 280, "h": 30},
        # NOWCOMBO coordinates are relative to NOWJUDGE in the LR2 runtime.
        "preset_nowcombo_0": {"x": 128, "y": 34, "w": 24, "h": 30},
        "preset_gauge_0": {"x": 52, "y": 672, "w": 9, "h": 14},
        "preset_fast": {"x": 742, "y": 655, "w": 12, "h": 15},
        "preset_slow": {"x": 956, "y": 655, "w": 12, "h": 15},
    }.items():
        edits.append({"id": identity, "set": values})
    recipe = {"version": 1, "scene": "play7", "width": 1280, "height": 720,
              "title": "BLOCKBEAT - Overworld 7K v0.1", "maker": "AI experimental", "objects": edits}
    base = build_skin(editor, recipe, output, preview=False)
    base_skin = Path(base["skin"])
    lines = base_skin.read_text(encoding="cp932").splitlines()
    target = output / THEME
    target.mkdir(parents=True)
    shutil.copyfile(HERE / "overworld.png", target / "overworld.png")
    atlas = make_atlas()
    atlas.image.save(target / "blockbeat-atlas.png")
    with Image.open(target / "overworld.png") as landscape:
        art_size = landscape.size
    image_path = THEME.replace("/", "\\")
    result, identity = [], ""
    for line in lines:
        if line == "$SE_OBJECT_NAME,Note P1 Lane 0":
            for i, (x, w) in enumerate(zip(starts, widths)):
                beam = image_object(f"blockbeat_key_beam_{i}", 0, atlas.rects["key_beam"], (x+2, 450, w-4, 104))
                # Timers 100..107 are 1P scratch/key-on, independent of hit bombs.
                beam[-1] = dst("#DST_IMAGE", x+2, 450, w-4, 104, timer=100+i)
                result += beam
        if line.startswith("#INFORMATION,"):
            parts = line.split(",")
            parts[4] = image_path + "\\overworld.png"
            result.append(",".join(parts))
            continue
        if line.startswith("#IMAGE,"):
            result += ["#IMAGE,"+image_path+"\\blockbeat-atlas.png", "#IMAGE,"+image_path+"\\overworld.png"]
            # A fallback landscape behind the standard BGA object. Playback BGA
            # replaces this window when a chart supplies one.
            result += image_object("overworld_landscape", 1, (0, 0, *art_size), (592, 156, 642, 361))
            continue
        if line.startswith("$SE_OBJECT_ID,"):
            identity = line.split(",", 1)[1]
        if line.startswith("#SRC_") and not line.startswith("#SRC_BGA,"):
            p = line.split(",")
            cmd = p[0]
            key = None
            if identity == "preset_background": key = "ui"
            elif identity.startswith("preset_bomb_"): key = "bomb"
            elif identity.startswith("preset_note_"):
                i = int(identity.rsplit("_", 1)[1])
                key = {"#SRC_NOTE": "gold" if i==0 else "quartz" if i%2 else "diamond",
                       "#SRC_MINE": "mine", "#SRC_LN_BODY": "ln_body", "#SRC_LN_START": "ln_start", "#SRC_LN_END": "ln_end"}[cmd]
            elif identity == "preset_line_0": key = "line"
            elif identity == "preset_judgeline_0": key = "judge_line"
            elif identity == "preset_nowjudge_0": key = "judge_"+p[1]
            elif identity == "preset_gauge_0": key = "gauge"
            else: key = "digits"
            # SRC image columns come from the native schema; preserve runtime
            # indices, timer, gauge spacing and variant selection from the preset.
            for field, value in zip(("gr", "x", "y", "w", "h"), (0, *atlas.rects[key])):
                p[schema[cmd][field]] = str(value)
            if cmd == "#SRC_NOWJUDGE_1P":
                p[schema[cmd]["noshift"]] = "1"
                if int(p[1]) > 0:
                    result += [f"$SE_OBJECT_NAME,Judge variant {p[1]}", f"$SE_OBJECT_ID,blockbeat_judge_{p[1]}"]
            if cmd == "#SRC_NOWCOMBO_1P" and int(p[1]) > 3:
                result += [f"$SE_OBJECT_NAME,Combo variant {p[1]}", f"$SE_OBJECT_ID,blockbeat_judge_combo_{p[1]}"]
            line = ",".join(p)
        result.append(line)
    # Explicit live LR2 number bindings, not decorative fake score text.
    for identity, number, x, y, w, h, count in [
        # Right alignment pads inside a fixed-width field beginning at x.
        ("ex_score", 101, 658, 589, 24, 30, 7),
        ("bpm", 160, 926, 589, 24, 30, 4),
        ("combo", 104, 1110, 589, 24, 30, 5),
        ("max_combo", 105, 1170, 655, 12, 15, 5),
        ("gauge_percent", 107, 464, 644, 18, 22, 3),
    ]:
        result += [f"$SE_OBJECT_NAME,{identity}", f"$SE_OBJECT_ID,blockbeat_{identity}",
                   "#SRC_NUMBER,0,0,"+",".join(map(str, atlas.rects["digits"]))+f",10,1,0,0,{number},0,{count}",
                   dst("#DST_NUMBER", x, y, w, h)]
    # LR2 notes originate at y=0. The foreground cap clips notes above the lane
    # entrance without changing their timing or travel coordinates.
    result += image_object("blockbeat_highway_cap", 0, (28, 0, 530, 126), (28, 0, 530, 126))
    skin = target / "BLOCKBEAT-Overworld-7K.lr2skin"
    skin.write_bytes(("\r\n".join(result)+"\r\n").encode("cp932"))
    # Only the temporary preset created by this invocation is discarded.
    generated = base_skin.parent.resolve()
    if generated != (output / "LR2files/Theme/Generated").resolve():
        raise ValueError("Unexpected preset path")
    shutil.rmtree(generated)
    inspection = editor.request("inspect", skin)
    validate_artifacts(output, skin, inspection)
    write_json(output / "objects.json", inspection)
    editor.request("render", skin, output / "preview.png")
    shutil.copyfile(HERE / "README.md", output / "README.md")
    shutil.copyfile(HERE / "art-prompt.txt", output / "art-prompt.txt")
    write_json(output / "build-report.json", {"ok": True, "theme": "BLOCKBEAT Overworld v0.1",
               "skin": skin.relative_to(output).as_posix(), "object_count": inspection["object_count"],
               "row_count": inspection["row_count"], "preview": "preview.png",
               "native_parser_checked": True, "native_render_checked": True, "lr2_gameplay_checked": False})
    files = sorted(p for p in output.rglob("*") if p.is_file())
    (output / "SHA256SUMS.txt").write_text("\n".join(hashlib.sha256(p.read_bytes()).hexdigest()+"  "+p.relative_to(output).as_posix() for p in files)+"\n", encoding="utf-8")
    print(json.dumps({"ok": True, "skin": str(skin), "preview": str(output / "preview.png"), "objects": inspection["object_count"]}))


def validate_artifacts(output, skin, inspection):
    """Check packaged assets and runtime-critical declarations, not just parsing."""
    rows = [line.split(",") for line in skin.read_text(encoding="cp932").splitlines()]
    images = []
    for row in rows:
        if row[0] == "#IMAGE":
            path = output / row[1].replace("\\", "/")
            if not path.resolve().is_relative_to(output.resolve()):
                raise ValueError("Image reference escaped the output")
            with Image.open(path) as img:
                images.append(img.size)
                img.verify()
        elif row[0].startswith("#SRC_") and row[0] != "#SRC_BGA":
            gr, x, y, w, h, dx, dy = map(int, row[2:9])
            iw, ih = images[gr]
            if not (0 <= x < iw and 0 <= y < ih and 0 < w <= iw-x and 0 < h <= ih-y
                    and dx > 0 and dy > 0 and w % dx == 0 and h % dy == 0):
                raise ValueError(f"Invalid atlas rectangle: {row}")
    for command in ("#SRC_NOTE", "#SRC_MINE", "#SRC_LN_START", "#SRC_LN_BODY", "#SRC_LN_END", "#DST_NOTE"):
        if sorted(int(row[1]) for row in rows if row[0] == command) != list(range(8)):
            raise ValueError(f"Incomplete 7K + scratch mapping: {command}")
    if len([row for row in rows if row[0] == "#DST_LINE"]) != 1:
        raise ValueError("Missing runtime measure-line origin")
    bindings = {int(row[11]) for row in rows if row[0] == "#SRC_NUMBER"}
    if bindings != {101, 104, 105, 107, 160, 212, 214}:
        raise ValueError("Unexpected live numeric bindings")
    ids = [obj["id"] for obj in inspection["objects"]]
    if len(ids) != 47 or len(set(ids)) != len(ids):
        raise ValueError("Missing or duplicate native Objects")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--editor", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    build(args.editor, args.out.resolve())
