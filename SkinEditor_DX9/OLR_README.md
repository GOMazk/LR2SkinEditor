# OLR package subsystem

Purpose: bridge the current LR2 workspace to an AI-readable `.olrskin`
container without losing commands that do not yet have a semantic model.

Entry points:

- `SEWriteOLRSkinPackage()` writes the V0.4 manifest, semantic/Object index,
  Simple Mode slots, source/path maps, merged LR2 script, captured virtual roots
  and fixed external images.
- `SEInspectOLRSkinPackage()` validates and lists a package without writing.
- `SECompileOLRSimpleMode()` validates V0.4 Simple Mode rows and compiles only
  source atlas fields while preserving every unsupported LR2 row and column.
- `SEExtractOLRSkinPackage()` extracts the `lr2/` subtree, compiles V0.4 Simple
  Mode in memory, then atomically replaces the new `main.lr2skin`.
- `SEExportOLRWorkspaceToLR2()` materializes an imported workspace into a new
  install-ready `LR2files/` tree and restores virtual CSV path fields.
- `SEResolveSkinResourcePath()` maps LR2-rooted declarations to either a real
  LR2 tree, a standalone theme folder or an imported `vfs/` workspace without
  mutating source rows.
- `WORKSPACE::ExportOlrSkin()` adapts authoritative workspace rows and Object
  model data into `SEOLRSkinDocument`.
- `WORKSPACE::ImportOlrSkinInteractive()` selects and validates a package,
  extracts it to a new folder, and rejoins the normal LR2 load path.
- `WORKSPACE::ExportLr2SkinInteractive()` owns the explicit materialization UI.

Important invariants:

- `skinfileLines` and `SEObjectEditorModel` remain authoritative while editing.
- V0.4 `skin.json.simple_mode` owns only `gr`, crop, division and cycle fields of
  validated source rows. Semantic `sections` and all other LR2 data remain
  descriptive/compatibility-owned.
- Export translates expanded Workspace row ids through source-map packaged rows;
  the compiler never guesses across omitted include or `$FILE` rows.
- runtime path resolution changes only derived Preview/load paths; the raw CSV,
  owner rows and save model remain authoritative.
- ZIP paths are relative and forward-slash separated. Case-insensitive duplicate,
  traversal, symlink and overlong entry handling stays at the package boundary.
- import never overwrites an existing directory and never uses original owner
  paths from the source map.
- LR2 export accepts only an imported V0.2+ workspace and a non-existing target;
  it copies both virtual roots and fixed `assets/` beside the compiled main skin,
  and never writes directly into a user's LR2 installation.
- archive writes use a temporary file followed by atomic replacement.

Tests: run `--self-test-olr-package`, normally through `scripts/test.ps1`.
Format and limitations: `../docs/OLRSKIN_FORMAT.md`.
