import importlib.util
import json
import os
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("skin_agent", ROOT / "tools" / "skin_agent.py")
agent = importlib.util.module_from_spec(spec)
spec.loader.exec_module(agent)


class RecipeTests(unittest.TestCase):
    def test_defaults_and_scene_validation(self):
        self.assertEqual(agent.validate_recipe({})["scene"], "play7")
        for recipe in ({"scene": "unknown"}, {"version": 9}, {"version": True},
                       {"width": True}, {"width": 100000}, {"execute": "anything"}):
            with self.subTest(recipe=recipe), self.assertRaises(agent.AgentError):
                agent.validate_recipe(recipe)

    def test_csv_encoding_and_metadata(self):
        self.assertEqual(agent.validate_recipe({"title": "日本語"})["title"], "日本語")
        for title in ("a,b", "a\nb", "a\0b", "\U0001f600"):
            with self.subTest(title=title), self.assertRaises(agent.AgentError):
                agent.validate_recipe({"title": title})

    def test_strict_object_fields(self):
        for edit in ({"id": "../skin", "set": {"x": 1}},
                     {"id": "note", "set": {"r": 256}},
                     {"id": "note", "set": {"x": 1.5}},
                     {"id": "note", "set": {"timer": 3}}):
            with self.subTest(edit=edit), self.assertRaises(agent.AgentError):
                agent.validate_recipe({"objects": [edit]})
        with self.assertRaises(agent.AgentError):
            agent.validate_recipe({"objects": [{"id": "a", "set": {"x": 1}}] * 2})

    def test_dynamic_schema_and_unmodified_fields(self):
        # The X column deliberately differs from LR2's usual order: use the schema.
        source = "$SE_OBJECT_ID,note\r\n#DST_TEST,keep,20,99\r\n#OTHER,raw\r\n"
        edits = [{"id": "note", "set": {"x": -5}}]
        result = agent.apply_object_edits(source, edits, {"#DST_TEST": {"x": 2}})
        self.assertEqual(result, source.replace(",20,", ",-5,"))
        with self.assertRaises(agent.AgentError):
            agent.apply_object_edits(source, edits, {})
        with self.assertRaises(agent.AgentError):
            agent.apply_object_edits(source, [{"id": "missing", "set": {"x": 1}}], {})

    def test_readback_rejects_mismatch(self):
        inspection = {"objects": [{"id": "note", "commands": [{"command": "#DST_TEST", "values": ["2"]}]}]}
        with self.assertRaises(agent.AgentError):
            agent.verify_edits(inspection, [{"id": "note", "set": {"x": 1}}], {"#DST_TEST": {"x": 1}})

    def test_existing_output_is_untouched(self):
        with agent.scratch_directory(ROOT / ".build" / "agent-python-tests") as directory:
            sentinel = directory / "keep.txt"
            sentinel.write_text("original")
            with self.assertRaises(agent.AgentError):
                agent.build_skin(None, {}, directory)
            self.assertEqual(sentinel.read_text(), "original")


@unittest.skipUnless(os.environ.get("LR2_AGENT_EDITOR"), "Set LR2_AGENT_EDITOR for native integration")
class NativeIntegrationTests(unittest.TestCase):
    def test_generate_inspect_render_and_no_overwrite(self):
        editor = agent.NativeEditor(Path(os.environ["LR2_AGENT_EDITOR"]), ROOT / ".build" / "agent-test-requests")
        recipe = json.loads((ROOT / "examples" / "agent" / "blue-play7.json").read_text())
        with agent.scratch_directory(ROOT / ".build" / "agent-python-tests") as directory:
            output = directory / "generated"
            result = agent.build_skin(editor, recipe, output)
            self.assertGreater(result["object_count"], 10)
            skin = Path(result["skin"])
            before = skin.read_bytes()
            png = Path(result["preview"]).read_bytes()
            self.assertEqual(png[:8], b"\x89PNG\r\n\x1a\n")
            self.assertEqual(int.from_bytes(png[16:20], "big"), 1280)
            self.assertEqual(int.from_bytes(png[20:24], "big"), 720)
            self.assertGreater(len(png), 1000)
            inspection = editor.request("inspect", skin)
            self.assertEqual(inspection["object_count"], result["object_count"])
            with self.assertRaises(agent.AgentError):
                agent.build_skin(editor, recipe, output)
            self.assertEqual(skin.read_bytes(), before)
            bad = dict(recipe, objects=[{"id": "missing", "set": {"x": 1}}])
            with self.assertRaises(agent.AgentError):
                agent.build_skin(editor, bad, directory / "bad")
            self.assertFalse((directory / "bad").exists())
            self.assertFalse(list(directory.glob(".skin-agent-*")))


if __name__ == "__main__":
    unittest.main()
