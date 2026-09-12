import json
import os
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import skin_agent as agent
import skin_agent_preview as preview


class StateTests(unittest.TestCase):
    def test_defaults_and_native_wire_contract(self):
        wire, timers = preview.state_wire({"gauge": 0, "bpm": 180, "judge": "great", "notes": "long", "timers": {"100": 200, "46": -1}})
        self.assertEqual(wire, "1000,0,123456,1234,123,321,180,4,2")
        self.assertEqual(timers, "46=-1;100=200")
        states = preview.validate_states(preview.default_states())
        self.assertEqual(len(states), 14)
        self.assertEqual({s["judge"] for s in states}, set(preview.JUDGES))

    def test_rejects_unsafe_and_invalid_states(self):
        for state in ({"name": "../x"}, {"name": "CON.txt"}, {"name": "CON"}, {"name": "한글"}, {"gauge": 101}, {"bpm": 0},
                      {"combo": True}, {"time_ms": -1}, {"judge": "PG"}, {"notes": "none;rm"},
                      {"timers": {"500": 0}}, {"timers": {"01": 0}}, {"timers": {"1": -2}},
                      {"unknown": 2}):
            with self.subTest(state=state), self.assertRaises(preview.PreviewError):
                preview.validate_state(state)
        with self.assertRaises(preview.PreviewError):
            preview.validate_states([{"name": "ONE"}, {"name": "one"}])

    def test_suite_failure_is_visible_and_existing_folder_preserved(self):
        class Unavailable:
            def request(self, operation, *args):
                if operation == "schema":
                    return {"capabilities": ["state_render"]}
                raise ValueError("graphics_initialization_failed")
        with agent.scratch_directory(ROOT / ".build/agent-python-tests") as scratch:
            output = scratch / "failed"
            report = preview.preview_suite(Unavailable(), scratch / "skin.lr2skin", [{"name": "failed"}], output)
            self.assertFalse(report["ok"])
            self.assertFalse(json.loads((output / "report.json").read_text(encoding="utf-8"))["ok"])
            self.assertIn("graphics_initialization_failed", (output / "index.html").read_text(encoding="utf-8"))
            original = (output / "report.json").read_bytes()
            with self.assertRaises(preview.PreviewError):
                preview.preview_suite(Unavailable(), scratch / "skin.lr2skin", [{}], output)
            self.assertEqual((output / "report.json").read_bytes(), original)

    def test_asset_readback_cannot_match_a_different_object(self):
        fields = {key: i+1 for i, key in enumerate(("gr", "x", "y", "w", "h", "div_x", "div_y", "cycle"))}
        asset = dict(zip(fields, (1, 0, 0, 16, 8, 1, 1, 0)), id="sample", bindings=[{"object": "wanted", "command": "#SRC_IMAGE"}])
        skin = ROOT / "fixture.lr2skin"
        text = "$SE_OBJECT_ID,wanted\n#SRC_IMAGE\n$SE_OBJECT_ID,wrong\n#SRC_IMAGE\n"
        row = {"command": "#SRC_IMAGE", "file": str(skin), "line": 4, "values": [str(asset[key]) for key in fields]}
        report = {"objects": [{"id": "wrong", "commands": [row]}]}
        with self.assertRaises(agent.AgentError):
            agent.verify_asset_readback(report, {"assets": [asset]}, {"#SRC_IMAGE": fields}, text, skin)
        row["line"] = 2
        agent.verify_asset_readback(report, {"assets": [asset]}, {"#SRC_IMAGE": fields}, text, skin)


@unittest.skipUnless(os.environ.get("LR2_AGENT_EDITOR"), "Set LR2_AGENT_EDITOR for native integration")
class AgentV02Integration(unittest.TestCase):
    def test_png_import_fixed_frames_and_unchanged_skin(self):
        from PIL import Image, ImageChops
        editor = agent.NativeEditor(Path(os.environ["LR2_AGENT_EDITOR"]))
        with agent.scratch_directory(ROOT / ".build/agent-python-tests") as scratch:
            Image.new("RGBA", (16, 8), (23, 231, 79, 255)).save(scratch / "green.png")
            recipe = {"scene": "play7", "assets": [{"id": "green", "path": "green.png",
                      "bind": [{"object": "preset_note_1", "command": "#SRC_NOTE"}],
                      "image": {"x": 1100, "y": 50, "w": 64, "h": 32}}]}
            created = agent.build_skin(editor, recipe, scratch / "built", recipe_dir=scratch)
            skin = Path(created["skin"])
            original = skin.read_bytes()
            self.assertEqual(created["object_count"], 33)
            saved = json.loads(Path(created["objects"]).read_text(encoding="utf-8"))
            self.assertTrue(all(Path(row["file"]).resolve() == skin.resolve()
                                for obj in saved["objects"] for row in obj["commands"]))
            with Image.open(Path(created["preview"])) as image:
                self.assertEqual(image.convert("RGB").getpixel((1120, 60)), (23, 231, 79))
            report = preview.preview_suite(editor, skin, [{"name": "empty", "gauge": 0, "notes": "none"},
                {"name": "full", "gauge": 100, "notes": "long", "judge": "perfect"}], scratch / "states")
            self.assertTrue(report["ok"], report)
            with Image.open(scratch / "states/empty.png") as a, Image.open(scratch / "states/full.png") as b:
                self.assertIsNotNone(ImageChops.difference(a.convert("RGB"), b.convert("RGB")).getbbox())
            preview.render_state(editor, skin, {"name": "full", "gauge": 100, "notes": "long", "judge": "perfect"}, scratch / "repeat.png")
            with Image.open(scratch / "repeat.png") as a, Image.open(scratch / "states/full.png") as b:
                self.assertIsNone(ImageChops.difference(a.convert("RGB"), b.convert("RGB")).getbbox())
            self.assertEqual(skin.read_bytes(), original)

    def test_diagnosis_uses_native_conditions_timers_and_owner_lines(self):
        from PIL import Image
        editor = agent.NativeEditor(Path(os.environ["LR2_AGENT_EDITOR"]))
        with agent.scratch_directory(ROOT / ".build/agent-python-tests") as scratch:
            folder = scratch / "LR2files/Theme/Checks"
            folder.mkdir(parents=True)
            Image.new("RGBA", (80, 8), (255, 255, 255, 255)).save(folder / "tiny.png")
            lines = ["#INFORMATION,0,Visibility,Tests,,,,", "#RESOLUTION,1280,720", "#ENDOFHEADER",
                     "#IMAGE,LR2files\\Theme\\Checks\\tiny.png", "#IMAGE,LR2files\\Theme\\Checks\\missing.png"]
            def obj(identity, x=20, alpha=255, blend=1, timer=0, time=0, loop=0):
                return ["$SE_OBJECT_ID,"+identity, "#SRC_IMAGE,0,0,0,0,16,8,1,1,0,0,0,0,0",
                        f"#DST_IMAGE,0,{time},{x},20,16,8,0,{alpha},255,255,255,{blend},0,0,0,{loop},{timer},0,0,0"]
            lines += obj("alpha", alpha=0) + obj("opaque_blend", alpha=0, blend=0)
            lines += obj("timer", timer=30) + obj("future", time=2000) + obj("outside", x=2000)
            lines += obj("expired", loop=-1)
            lines += ["#DST_IMAGE,0,100,20,20,16,8,0,255,255,255,255,1,0,0,0,-1,0,0,0,0"]
            lines += ["$SE_OBJECT_ID,digits", "#SRC_NUMBER,0,0,0,0,80,8,10,1,0,0,101,0,2",
                      "#DST_NUMBER,0,0,20,40,8,8,0,255,255,255,255,1,0,0,0,0,0,0,0,0"]
            lines += [line.replace("#SRC_IMAGE,0,0,", "#SRC_IMAGE,0,1,") for line in obj("missing")]
            lines += ["#SETOPTION,900,0", "#IF,900", "#INCLUDE,child.csv", "#ENDIF"]
            (folder / "child.csv").write_bytes(("\r\n".join(obj("child"))+"\r\n").encode("cp932"))
            skin = folder / "skin.lr2skin"
            skin.write_bytes(("\r\n".join(lines)+"\r\n").encode("cp932"))
            result = preview.diagnose(editor, skin, {})
            issues = result["diagnostics"]["issues"]
            codes = lambda identity: {i["code"] for i in issues if i["object_id"] == identity}
            self.assertIn("alpha_zero", codes("alpha"))
            self.assertTrue(any(i["confirmed_hidden"] for i in issues if i["object_id"] == "alpha"))
            self.assertFalse(any(i["confirmed_hidden"] for i in issues if i["object_id"] == "opaque_blend"))
            self.assertIn("timer_inactive", codes("timer"))
            self.assertIn("offscreen", codes("outside"))
            self.assertIn("before_animation", codes("future"))
            self.assertIn("after_animation", codes("expired"))
            self.assertIn("source_asset_unavailable", codes("missing"))
            self.assertIn("number_truncated", codes("digits"))
            child = [i for i in issues if i["object_id"] == "child" and i["code"] == "if_inactive"]
            self.assertTrue(child, result)
            self.assertEqual(child[0]["line"], 3)
            self.assertEqual(Path(child[0]["file"]).name, "child.csv")
            self.assertEqual(Path(child[0]["file"]).resolve(), (folder / "child.csv").resolve())
            selected = preview.diagnose(editor, skin, {}, object_id="alpha")
            self.assertEqual([o["id"] for o in selected["diagnostics"]["objects"]], ["alpha"])


if __name__ == "__main__":
    unittest.main()
