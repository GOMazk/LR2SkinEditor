# OLR package subsystem

Purpose: bridge the current LR2 workspace to an AI-readable `.olrskin`
container without losing commands that do not yet have a semantic model.

Entry points:

- `SEWriteOLRSkinPackage()` writes `manifest.json`, `skin.json`, source map,
  merged LR2 and bundled concrete images.
- `SEInspectOLRSkinPackage()` validates and lists a package without writing.
- `SEExtractOLRSkinPackage()` extracts only the `lr2/` subtree to a new folder.
- `WORKSPACE::ExportOlrSkin()` adapts authoritative workspace rows and Object
  model data into `SEOLRSkinDocument`.
- `WORKSPACE::ImportOlrSkinInteractive()` selects and validates a package,
  extracts it to a new folder, and rejoins the normal LR2 load path.

Important invariants:

- `skinfileLines` and `SEObjectEditorModel` remain authoritative while editing.
- V0.1 `skin.json` is descriptive; `lr2/main.lr2skin` owns round-trip output.
- ZIP paths are ASCII, relative and forward-slash separated.
- import never overwrites an existing directory and never uses original owner
  paths from the source map.
- archive writes use a temporary file followed by atomic replacement.

Tests: run `--self-test-olr-package`, normally through `scripts/test.ps1`.
Format and limitations: `../docs/OLRSKIN_FORMAT.md`.
