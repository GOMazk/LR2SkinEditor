# OLR package subsystem

Purpose: bridge the current LR2 workspace to an AI-readable `.olrskin`
container without losing commands that do not yet have a semantic model.

Entry points:

- `SEWriteOLRSkinPackage()` writes the V0.7 manifest, semantic/Object index,
  Layout/Animation/Condition data, Simple Mode slots, source/path maps, merged
  LR2 script, captured virtual roots and fixed external images.
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
- `WORKSPACE::ExportOlrSkin()` is the low-level projection/package boundary. It
  adapts authoritative workspace rows and Object model data into
  `SEOLRSkinDocument` without changing the active script path.
- `WORKSPACE::SaveOlrSkin()` owns the user-facing `File > Save OLRskin` policy.
  A normal LR2 workspace is packaged without modifying its source files. An
  imported OLR workspace first saves dirty script edits, then atomically
  replaces the chosen package so LR2 folder export sees the same edits.
- `WORKSPACE::ImportOlrSkinInteractive()` selects and validates a package,
  extracts it to a new folder, and rejoins the normal LR2 load path.
- `WORKSPACE::ExportLr2SkinInteractive()` owns the explicit materialization UI.

Important invariants:

- `skinfileLines` and `SEObjectEditorModel` remain authoritative while editing.
- V0.7 `skin.json.objects` owns the first supported destination family Layout,
  Animation and Condition fields. V0.4 `skin.json.simple_mode` owns only `gr`,
  crop, division and cycle fields of validated source rows. Alternate families,
  unsupported rows and control flow remain compatibility-owned.
- legacy source crops with non-positive dimensions are compatibility-owned:
  new exports omit them from `simple_mode`, and import of an older package
  leaves their original LR2 row raw instead of failing the whole document.
- Export translates expanded Workspace row ids through source-map packaged rows;
  the compiler never guesses across omitted include or `$FILE` rows.
- runtime path resolution changes only derived Preview/load paths; the raw CSV,
  owner rows and save model remain authoritative.
- ZIP paths are relative and forward-slash separated. Case-insensitive duplicate,
  traversal, symlink and overlong entry handling stays at the package boundary.
- captured virtual roots exclude nested `.olrskin` containers and their
  interrupted writer temporaries so repeated package round trips cannot grow
  recursively.
- destination schemas are compiler contracts. In particular,
  `#DST_BARGRAPH` keeps `(NULL),time,...,loop,timer,op1,op2,op3`; changing that
  order breaks M.H-style player visibility conditions.
- import never overwrites an existing directory and never uses original owner
  paths from the source map.
- the source package association is local Workspace state. It is set only after
  successful import/save, cleared at the next document load and never embedded
  into the package or LR2 export.
- LR2 export accepts only an imported V0.2+ workspace and a non-existing target;
  it copies both virtual roots and fixed `assets/` beside the compiled main skin,
  and never writes directly into a user's LR2 installation.
- archive writes use a temporary file followed by atomic replacement.

Tests: run `--self-test-olr-package`, normally through `scripts/test.ps1`.
Format and limitations: `../docs/OLRSKIN_FORMAT.md`.
