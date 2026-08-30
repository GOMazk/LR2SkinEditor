# OLR package subsystem

Purpose: bridge the current LR2 workspace to an AI-readable `.olrskin`
container without losing commands that do not yet have a semantic model.

Entry points:

- `SEWriteOLRSkinPackage()` writes the V0.8 manifest, nested Object/part/
  destination index, Layout/Animation/Condition data, Simple Mode slots,
  source/path maps, merged LR2 script, captured virtual roots and fixed external
  images. The V0.8 manifest records semantic, Simple Mode, asset, virtual-root,
  virtual-file, skipped and unresolved counts.
- `SEInspectOLRSkinPackage()` validates and lists a package without writing. For
  V0.8 it cross-checks semantic/Simple Mode counts with `skin.json`, asset/file
  counts with the archive, and the version-8 document authorities.
- `SECompileOLRSimpleMode()` validates V0.4 Simple Mode rows and compiles only
  source atlas fields while preserving every unsupported LR2 row and column.
- `SECompileOLRSemantics()` dispatches the declared Objects authority. V0.8
  validates nested source-bound parts and compiles each part destination while
  V0.1-V0.7 keep their existing parser/authority behavior.
- `SEExtractOLRSkinPackage()` extracts the `lr2/` subtree, compiles the declared
  Simple Mode/Object authorities in memory, then atomically replaces the new
  `main.lr2skin`.
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
- V0.8 `skin.json.objects` uses authority
  `lr2-destination-parts-v0.8`. Each item owns ordered `parts`; each part records
  exact source rows and independently compiled consecutive destination-command
  runs. V0.4
  `skin.json.simple_mode` still owns only `gr`, crop, division and cycle fields
  of validated source rows. Unsupported rows and control flow remain
  compatibility-owned.
- part boundaries follow packaged row order: consecutive sources before the
  first destination stay together, while a source after a destination starts a
  new part. This keeps NOTE multi-source declarations together and splits the
  real kamh BUTTON `SRC/DST/SRC/DST` sequence into two parts. It does not infer
  variants or link IF/ELSE branches. A supported destination before any source
  may create a source-less part; parts without a supported destination stay raw
  and are not serialized as empty semantic items.
- condition `timer` and `loop` are compatibility-preserved when null. Every
  owned OP term names one unique slot 1..3, and an omitted slot is preserved.
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
- V0.1-V0.7 imports keep their version-specific parser and authority; legacy
  flat Objects are never guessed into V0.8 parts.
- the current package writer serializes supported fields and does not preserve
  unknown manifest or `skin.json` extensions on `Save OLRskin`. Extension
  namespacing and passthrough are a future format-policy decision.
- import never overwrites an existing directory and never uses original owner
  paths from the source map.
- the source package association is local Workspace state. It is set only after
  successful import/save, cleared at the next document load and never embedded
  into the package or LR2 export.
- LR2 export accepts only an imported V0.2+ workspace and a non-existing target;
  it copies both virtual roots and fixed `assets/` beside the compiled main skin,
  and never writes directly into a user's LR2 installation.
- archive writes use a temporary file followed by atomic replacement.

Tests: run `--self-test-olr-package`, normally through `scripts/test.ps1`. The
package fixture round-trips an explicitly constructed two-part document and its
manifest counts. Direct compiler fixtures cover explicit multi-source and
two-part addresses, partial condition ownership and the retained V0.7 flat
authority. They do not call `WORKSPACE::ExportOlrSkin()`, so real kamh BUTTON
and NOTE row-to-part derivation remains a manual regression. Set
`SKINEDITOR_TEST_OLR_PACKAGE` to an actual saved kamh package for the optional
production-import regression; the test extracts only to its temporary folder
and does not assert the skin-specific part layout.
Format and limitations: `../docs/OLRSKIN_FORMAT.md`.
