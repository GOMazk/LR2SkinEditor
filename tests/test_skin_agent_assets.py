import importlib.util
from pathlib import Path
import shutil
import unittest
from unittest.mock import patch
import uuid

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("skin_agent_assets", ROOT / "tools" / "skin_agent_assets.py")
assets = importlib.util.module_from_spec(spec)
spec.loader.exec_module(assets)

SRC = {"index": 1, "gr": 2, "x": 3, "y": 4, "w": 5, "h": 6, "div_x": 7,
       "div_y": 8, "cycle": 9, "timer": 10, "op1": 11, "op2": 12, "op3": 13}
DST = {"time": 2, "x": 3, "y": 4, "w": 5, "h": 6, "acc": 7, "a": 8, "r": 9,
       "g": 10, "b": 11, "blend": 12, "filter": 13, "angle": 14, "center": 15,
       "loop": 16, "timer": 17, "op1": 18, "op2": 19, "op3": 20}
COLUMNS = {"#SRC_NOTE": SRC, "#SRC_IMAGE": {key: value for key, value in SRC.items() if key != "index"},
           "#DST_IMAGE": DST}
PRESET = ("// 日本語\r\n#INFORMATION,0,Test,Author,preset.png,,1280,720\r\n#ENDOFHEADER\r\n"
          "#IMAGE,LR2files\\Theme\\Generated\\preset.png\r\n"
          "$SE_OBJECT_NAME,Note\r\n$SE_OBJECT_ID,note\r\n"
          "#SRC_NOTE,1,0,100,200,60,12,1,1,0,50,0,0,0\r\n"
          "#DST_NOTE,1,0,32,500,60,12,0,255,255,255,255,0,0,0,0,0,0,0,0,0\r\n")


class AssetTests(unittest.TestCase):
    def setUp(self):
        scratch = ROOT / ".build" / "asset-python-tests"
        scratch.mkdir(parents=True, exist_ok=True)
        self.directory = scratch / ("run-" + uuid.uuid4().hex)
        self.directory.mkdir()
        self.output = self.directory / "output"
        (self.output / assets.GENERATED).mkdir(parents=True)

    def tearDown(self):
        # Only the test-owned scratch child, never an input outside this directory.
        if self.directory.resolve().parent == (ROOT / ".build" / "asset-python-tests").resolve():
            shutil.rmtree(self.directory)

    def png(self, name="note.png", size=(60, 12), color=(1, 22, 199, 180)):
        path = self.directory / name
        with Image.new("RGBA", size, color) as image:
            image.putpixel((0, 0), (255, 31, 57, 0))
            image.save(path)
        return path

    def asset(self, name="note", path="note.png", **extra):
        return {"id": name, "path": path, "bind": [{"object": "note", "command": "#SRC_NOTE"}], **extra}

    def apply(self, requested, text=PRESET, columns=COLUMNS):
        return assets.apply_assets(text, requested, self.directory, self.output, columns)

    def assert_no_atlas(self):
        self.assertEqual(list((self.output / assets.GENERATED).glob("agent-atlas-*.png")), [])

    def test_validation_rejects_ambiguous_or_malformed_recipes(self):
        image = {"x": 0, "y": 0, "w": 1, "h": 1}
        invalid = [None, {}, [self.asset(name="../bad")], [self.asset(name="한글")],
                   [self.asset(path="note.jpg")], [self.asset(path="bad\0.png")],
                   [{"id": "unused", "path": "note.png"}], [self.asset(div_x=True)],
                   [self.asset(cycle=-1)], [self.asset(image={**image, "w": 0})],
                   [self.asset(image={**image, "layer": "between"})], [self.asset(unknown=2)],
                   [self.asset(bind=[{"object": "note", "command": "#DST_NOTE"}])],
                   [self.asset(bind=[{"object": "note", "command": "#SRC_NOTE", "index": True}])],
                   [self.asset(), self.asset()],
                   [self.asset(), self.asset(name="duplicate_binding")]]
        for requested in invalid:
            with self.subTest(requested=requested), self.assertRaises(assets.AssetError):
                assets.validate_assets(requested)
        normalized = assets.validate_assets([self.asset(bind=[{"object": "note", "command": "#src_note", "index": 1}])])
        self.assertEqual(normalized[0]["bind"][0]["command"], "#SRC_NOTE")
        self.assertEqual(normalized[0]["div_x"], 1)

    def test_schema_binding_preserves_pixels_and_unrelated_rows(self):
        path = self.png()
        original_png = path.read_bytes()
        output, manifest = self.apply([self.asset()])
        self.assertIn("#SRC_NOTE,1,1,0,0,60,12,1,1,0,50,0,0,0\r\n", output)
        self.assertIn(PRESET.split("#DST_NOTE")[1], output)
        self.assertIn("// 日本語\r\n", output)
        self.assertEqual(output.encode("cp932").decode("cp932"), output)
        self.assertEqual(manifest["added_object_ids"], [])
        self.assertEqual(path.read_bytes(), original_png)
        self.assertEqual(manifest["atlases"][0]["gr"], 1)
        with Image.open(self.output / manifest["atlases"][0]["path"]) as atlas, Image.open(path) as source:
            self.assertEqual(atlas.mode, "RGBA")
            self.assertEqual(atlas.tobytes(), source.tobytes())
            self.assertEqual(atlas.getpixel((0, 0)), (255, 31, 57, 0))

    def test_binding_uses_schema_columns_not_fixed_positions(self):
        self.png()
        columns = {"#SRC_NOTE": {**SRC, "gr": 3, "x": 2, "w": 6, "h": 5}}
        result, _ = self.apply([self.asset()], columns=columns)
        self.assertIn("#SRC_NOTE,1,0,1,0,12,60,1,1,0,50,0,0,0", result)

    def test_sheet_split_and_index_select_exact_source(self):
        self.png(size=(120, 24))
        extra_row = "#SRC_NOTE,2,0,100,200,60,12,1,1,0,51,0,0,0\r\n"
        text = PRESET.replace("#DST_NOTE", extra_row + "#DST_NOTE")
        selected = self.asset(div_x=2, div_y=2, cycle=280,
                              bind=[{"object": "note", "command": "#SRC_NOTE", "index": 2}])
        result, _ = self.apply([selected], text=text)
        self.assertIn("#SRC_NOTE,1,0,100,200,60,12,1,1,0,50,0,0,0", result)
        self.assertIn("#SRC_NOTE,2,1,0,0,120,24,2,2,280,51,0,0,0", result)

    def test_back_and_front_objects_and_original_gr_order(self):
        self.png("back.png", (10, 20), (33, 55, 77, 111))
        self.png("front.png", (4, 5), (99, 88, 77, 222))
        requested = [{"id": name, "path": name + ".png", "image": {
            "x": 10, "y": 20, "w": 30, "h": 40, "layer": name}} for name in ("back", "front")]
        text = PRESET.replace("$SE_OBJECT_NAME,Note", "#IMAGE,second.png\r\n$SE_OBJECT_NAME,Note")
        output, manifest = self.apply(requested, text=text)
        self.assertLess(output.index("#IMAGE,second.png"), output.index("#IMAGE,LR2files\\Theme\\Generated\\agent-atlas-000.png"))
        self.assertLess(output.index("$SE_OBJECT_ID,asset_back"), output.index("$SE_OBJECT_NAME,Note"))
        self.assertGreater(output.index("$SE_OBJECT_ID,asset_front"), output.index("#DST_NOTE"))
        self.assertIn("#SRC_NOTE,1,0,100,200", output)
        self.assertIn("#DST_IMAGE,0,0,10,20,30,40,0,255,255,255,255,1,0,0,0,0,0,0,0,0", output)
        self.assertEqual(manifest["added_object_ids"], ["asset_back", "asset_front"])
        for entry in manifest["assets"]:
            with Image.open(self.output / entry["atlas"]) as atlas, Image.open(self.directory / (entry["id"] + ".png")) as source:
                crop = atlas.crop((entry["x"], entry["y"], entry["x"] + entry["w"], entry["y"] + entry["h"]))
                self.assertEqual(crop.tobytes(), source.tobytes())
                self.assertEqual(entry["gr"], 2)
        with Image.open(self.output / manifest["atlases"][0]["path"]) as atlas:
            self.assertEqual(atlas.getpixel((10, 0)), (0, 0, 0, 0))
            self.assertEqual(atlas.getpixel((11, 0)), (0, 0, 0, 0))

    def test_multiple_pages_and_determinism(self):
        self.png("wide.png", (2048, 1024))
        self.png("tall.png", (2048, 1024), (33, 44, 55, 66))
        requested = [{"id": name, "path": name + ".png", "image": {"x": 0, "y": 0, "w": 1, "h": 1}}
                     for name in ("wide", "tall")]
        text, manifest = self.apply(requested)
        self.assertEqual(len(manifest["atlases"]), 2)
        self.assertEqual([entry["gr"] for entry in manifest["assets"]], [1, 2])
        with Image.open(self.output / manifest["assets"][1]["atlas"]) as atlas:
            self.assertEqual(atlas.getpixel((1, 0)), (33, 44, 55, 66))
        second_output = self.directory / "second"
        (second_output / assets.GENERATED).mkdir(parents=True)
        second_text, second_manifest = assets.apply_assets(PRESET, requested, self.directory, second_output, COLUMNS)
        self.assertEqual(text, second_text)
        self.assertEqual(manifest, second_manifest)
        for entry in manifest["atlases"]:
            self.assertEqual((self.output / entry["path"]).read_bytes(), (second_output / entry["path"]).read_bytes())

    def test_unicode_and_absolute_input_paths_stay_out_of_cp932_csv(self):
        source = self.png("한국어 배경.png")
        text, manifest = self.apply([self.asset(path=str(source.resolve()))])
        self.assertNotIn(source.name, text)
        self.assertNotIn(str(source), str(manifest))
        text.encode("cp932", errors="strict")

    def test_missing_ids_ambiguous_rows_and_overlapping_bindings_fail_before_writes(self):
        self.png()
        extra = "#SRC_NOTE,2,0,100,200,60,12,1,1,0,51,0,0,0\r\n"
        cases = [([self.asset(bind=[{"object": "missing", "command": "#SRC_NOTE"}])], PRESET),
                 ([self.asset()], PRESET.replace("#DST_NOTE", extra + "#DST_NOTE")),
                 ([self.asset()], PRESET + "$SE_OBJECT_ID,note\r\n"),
                 ([self.asset(), self.asset(name="overlap", bind=[{"object": "note", "command": "#SRC_NOTE", "index": 1}])], PRESET)]
        for requested, text in cases:
            with self.subTest(requested=requested), self.assertRaises(assets.AssetError):
                self.apply(requested, text=text)
            self.assert_no_atlas()

    def test_existing_atlas_and_generated_object_are_not_overwritten(self):
        self.png()
        target = self.output / assets.GENERATED / "agent-atlas-000.png"
        target.write_bytes(b"original data")
        with self.assertRaisesRegex(assets.AssetError, "already exists"):
            self.apply([self.asset()])
        self.assertEqual(target.read_bytes(), b"original data")
        target.unlink()
        with self.assertRaisesRegex(assets.AssetError, "Object ID.*already exists"):
            self.apply([self.asset(image={"x": 0, "y": 0, "w": 1, "h": 1})], text=PRESET + "$SE_OBJECT_ID,asset_note\r\n")
        self.assert_no_atlas()

    def test_bad_png_dimensions_frames_encoding_and_budgets(self):
        self.png(size=(60, 12))
        cases = [("missing.png", {}), ("note.png", {"div_x": 7})]
        for name, extra in cases:
            with self.subTest(name=name), self.assertRaises(assets.AssetError):
                self.apply([self.asset(path=name, **extra)])
        (self.directory / "fake.png").write_bytes(b"not a PNG")
        with self.assertRaisesRegex(assets.AssetError, "Cannot read PNG"):
            self.apply([self.asset(path="fake.png")])
        self.png("large.png", (2049, 1))
        with self.assertRaisesRegex(assets.AssetError, "atlas page"):
            self.apply([self.asset(path="large.png")])
        with patch.object(assets, "MAX_PIXELS", 100), self.assertRaisesRegex(assets.AssetError, "pixel budget"):
            self.apply([self.asset()])
        with patch.object(assets, "MAX_FILE_BYTES", 10), self.assertRaisesRegex(assets.AssetError, "file budget"):
            self.apply([self.asset()])
        with self.assertRaisesRegex(assets.AssetError, "CP932"):
            self.apply([self.asset()], text="// \U0001f600\r\n" + PRESET)
        self.assert_no_atlas()

    def test_missing_schema_and_conditional_inputs_fail_closed(self):
        self.png()
        for text, columns in ((PRESET, {}), ("#IF,1\r\n" + PRESET, COLUMNS),
                              (PRESET + "#IMAGE,late.png\r\n", COLUMNS)):
            with self.subTest(text=text, columns=columns), self.assertRaises(assets.AssetError):
                self.apply([self.asset()], text=text, columns=columns)
            self.assert_no_atlas()
        with self.assertRaisesRegex(assets.AssetError, "no index"):
            self.apply([self.asset(bind=[{"object": "note", "command": "#SRC_IMAGE", "index": 0}])])
        self.assert_no_atlas()

    def test_no_assets_returns_exact_original(self):
        result, manifest = self.apply([])
        self.assertEqual(result, PRESET)
        self.assertEqual(manifest, {"atlases": [], "assets": [], "added_object_ids": []})

    def test_animated_png_rejected_and_partial_atlas_failure_cleaned(self):
        with Image.new("RGBA", (60, 12), "red") as frame1, Image.new("RGBA", (60, 12), "blue") as frame2:
            frame1.save(self.directory / "animation.png", save_all=True, append_images=[frame2], duration=100)
        with self.assertRaisesRegex(assets.AssetError, "static PNG"):
            self.apply([self.asset(path="animation.png")])
        self.assert_no_atlas()
        self.png("first.png", (2048, 1024))
        self.png("second.png", (2048, 1024))
        requested = [{"id": name, "path": name + ".png", "image": {"x": 0, "y": 0, "w": 1, "h": 1}}
                     for name in ("first", "second")]
        save = Image.Image.save
        calls = 0

        def fail_second(image, stream, *args, **kwargs):
            nonlocal calls
            calls += 1
            if calls == 2:
                raise OSError("simulated full disk")
            return save(image, stream, *args, **kwargs)

        with patch.object(Image.Image, "save", fail_second), self.assertRaisesRegex(assets.AssetError, "full disk"):
            self.apply(requested)
        self.assert_no_atlas()


if __name__ == "__main__":
    unittest.main()
