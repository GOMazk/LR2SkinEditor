# OLR package subsystem

Purpose: bridge the current LR2 workspace to an AI-readable `.olrskin`
container without losing commands that do not yet have a semantic model.
The published format is locked as **OLRskin 0.9**; its JSON integer
`version: 9` is the established on-disk representation. Version or
format-feature changes need the user's explicit approval.

Entry points:

- `SEWriteOLRSkinPackage()` writes the V0.9 manifest, nested Object/part/
  destination index, Layout/Animation/Condition data, Simple Mode slots,
  source/path maps, merged LR2 script, captured virtual roots and fixed external
  images. When safe, it also records the unchanged compatibility baseline and
  original-main preservation marker. The package boundary writes the canvas to
  `#INFORMATION` fields 6/7 and one active `#RESOLUTION` row. If that row must be
  inserted, all affected compiler and source-map row addresses move with it.
- `SEInspectOLRSkinPackage()` validates and lists a package without writing. For
  V0.8+ it cross-checks semantic/Simple Mode counts with `skin.json`, asset/file
  counts with the archive, and the matching versioned document authorities.
- `SECompileOLRSimpleMode()` validates V0.4 Simple Mode rows and compiles only
  source atlas fields while preserving every unsupported LR2 row and column.
- `SECompileOLRSemantics()` dispatches the declared Objects authority. V0.8 and
  V0.9 validate nested source-bound parts; V0.9 changes only fields whose LR2
  integer value differs, so empty zero tokens and legacy spellings survive a
  no-edit round trip. V0.1-V0.7 keep their existing parser/authority behavior.
- `SEExtractOLRSkinPackage()` extracts the `lr2/` subtree, compiles the declared
  Simple Mode/Object authorities in memory, then atomically replaces the new
  `main.lr2skin`.
- `SEExportOLRWorkspaceToLR2()` materializes an imported workspace into a new
  install-ready `LR2files/` tree. An unchanged V0.9 workspace keeps the copied
  original include-based main after applying the same safe resolution-header
  and path rules to every copied `.lr2skin`, `.lr2ss`, and `.csv`; an edited or
  ineligible workspace writes the flattened compatibility script. Both paths
  remove case/slash variants of `vfs/LR2files`, root LR2 process-relative
  image/font/include/header/custom/help paths, and reject computer-local
  absolute declarations.
  Before writing a compatibility fallback, the materializer turns every
  `$OLR_FILE start/end` boundary back into a generated CSV `#INCLUDE`. The
  directive uses its full `LR2files\...` path because LR2 resolves includes
  from the process root rather than beside the declaring CSV. LR2 then
  evaluates the child with its own IF stack, so an inactive/right parent cannot
  be reactivated by a child `#IF` and overwrite the selected lane-0
  `#DST_NOTE`. This also preserves orphan or unclosed child controls exactly as
  LR2 handled them in the original file graph.
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
- OLR's unresolved canvas default is HD 1280x720. Explicit or TenRiff-inferred
  SD/HD/FHD remains unchanged. Both package and install-ready LR2 output use
  identical values in `#INFORMATION` fields 6/7 and one active `#RESOLUTION`
  row. Existing active or `$OLR_IGNORED_RESOLUTION` rows are reused; a missing
  row is inserted after `#INFORMATION` with compiler addresses adjusted.
- V0.9 `skin.json.objects` uses authority
  `lr2-destination-parts-v0.9`. Each item owns ordered `parts`; each part records
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
- semantic and Simple Mode compilers compare parsed LR2 integer values before
  writing. An empty numeric token is an unchanged zero, not a request to
  normalize the source row to textual `0`.
- original-main preservation is capability-based: both hidden baseline/marker
  files must exist, the current compatibility script must match the baseline,
  the recorded main must exist in `vfs/`, and no fixed `assets/` relocation may
  be required. Otherwise export deliberately falls back to the compatibility
  script so an edit is never discarded.
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
- V0.1-V0.8 imports keep their version-specific parser and authority; legacy
  flat Objects are never guessed into V0.9 parts.
- the current package writer serializes supported fields and does not preserve
  unknown manifest or `skin.json` extensions on `Save OLRskin`. Extension
  namespacing and passthrough remain outside the locked 0.9 contract unless the
  user explicitly approves a format change.
- import never overwrites an existing directory and never uses original owner
  paths from the source map.
- the source package association is local Workspace state. It is set only after
  successful import/save, cleared at the next document load and never embedded
  into the package or LR2 export.
- LR2 export accepts only an imported V0.2+ workspace and a non-existing target;
  it copies virtual roots and any fixed `assets/`, selects the safe original or
  compatibility main as described above, and never writes directly into a
  user's LR2 installation. The recorded main must be an `.lr2skin` or `.lr2ss`
  below `LR2files/Theme` or `LR2files/Sound`, matching LR2's actual skin-list
  scan roots. The output includes `INSTALL.txt` with the exact merge location.
- archive writes use a temporary file followed by atomic replacement.

Tests: run `--self-test-olr-package`, normally through `scripts/test.ps1`. The
package fixture round-trips an explicitly constructed two-part document and its
manifest counts, preserves an empty numeric zero token, and proves that an
unchanged original main plus include file survives LR2 materialization while an
edited compatibility script forces the safe fallback. The fallback fixture
also proves that generated child CSVs isolate an orphan `#ELSE` and an open
child `#IF` from the parent Scratch side branches. It also feeds an active
width/height `#RESOLUTION` row through package and original-main materialization
and verifies that it matches the `#INFORMATION` canvas. The main package fixture
starts without `#RESOLUTION` and verifies row-address correction after insertion. Mixed-case
virtual paths are removed from the main/include graph, and LR2's own
`MakeSkinList()` must enumerate the exported main from a relocated temporary
root. Direct
compiler fixtures cover explicit multi-source and
two-part addresses, partial condition ownership and the retained V0.7 flat
authority. They do not call `WORKSPACE::ExportOlrSkin()`, so real kamh BUTTON
and NOTE row-to-part derivation remains a manual regression. Set
`SKINEDITOR_TEST_OLR_PACKAGE` to an actual saved kamh package for the optional
production-import regression; the test extracts only to its temporary folder
and does not assert the skin-specific part layout.
Format and limitations: `../docs/OLRSKIN_FORMAT.md`.
