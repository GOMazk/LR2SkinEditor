# OLR Skin package format

`.olrskin` is an AI-readable intermediate package for LR2 skins. It keeps a
human-facing semantic index next to the compatibility data needed to return to
LR2 without silently deleting commands the editor does not understand.

## V0.9 scope

V0.9 keeps V0.8's source-bound nested Object parts and V0.4 Simple Mode source
compiler, then makes LR2 fidelity the release focus:

- save the currently loaded, fully expanded LR2 document as one ZIP package
  through `File > Save OLRskin`, retaining path-free `$OLR_FILE start/end`
  markers around flattened include bodies so each file keeps its own IF/ELSE
  scope;
- classify editor Objects into semantic categories in `skin.json`;
- list number-font, judgement-font, gear, note and gauge source slots with their
  packaged LR2 row, command, graphic crop and animation grid in
  `skin.json.simple_mode`;
- compile edited Simple Mode `gr`, crop, division and cycle values back into the
  matching `#SRC_*` row using the retained V0.4 authority during import;
- split each Object into source-bound `parts`, keeping consecutive source rows
  before the first destination in one part and starting a new part when a new
  source appears after a destination;
- expose every consecutive supported destination-command run inside each part
  as a destination with `layout.transform` (`x`, `y`, `width`, `height`,
  `rotation`, `blend`), ordered animation frames and a condition;
- represent `timer` and `loop` as optional owned values, and identify each OP
  term with its original slot so omitted values remain compatibility-owned;
- compile known OP/TIMER names through the LR2 name table, while keeping custom
  900-series or unknown values as numeric `raw` terms;
- retain every unsupported command, unowned condition field, comment, IF/ELSE
  branch and editor metadata in `lr2/main.lr2skin`;
- map resolvable `LR2files/...` declarations to `vfs/LR2files/...` while the
  skin is edited or previewed;
- bundle each resolved logical LR2 root, including wildcard choices, fonts and
  archive-backed resources, under `lr2/vfs/LR2files/...`;
- keep fixed non-LR2-rooted images under `lr2/assets/`;
- materialize a new install-ready `LR2files/...` tree only when the user invokes
  `File > Export install-ready LR2 folder` from an imported V0.2+ workspace;
- use HD 1280x720 as the OLR canvas only when the loaded source has no explicit
  or TenRiff-inferred resolution, and write the selected canvas to LR2's
  `#INFORMATION` fields 6 and 7;
- turn every active `#RESOLUTION` row in package/materialized mains into an
  inert `$OLR_IGNORED_RESOLUTION` row while preserving its physical row address,
  avoiding the LR2 skin-list parser path that can corrupt the next skin slot;
- compare each owned numeric field by its parsed LR2 value before writing, so an
  unchanged empty zero, whitespace or leading-zero spelling is not normalized;
  and
- when the compatibility script is still byte-identical to its V0.9 baseline,
  materialize the original captured main and include graph instead of replacing
  it with the flattened compatibility script.

`skin.json.objects` and `skin.json.simple_mode` are the only compiler inputs.
`sections` contains Object ids for navigation, not duplicate values. The raw LR2
script remains compatibility authority for unsupported SRC fields, unowned
condition slots, control flow, events, comments, metadata and unknown commands.
Parts describe source/DST adjacency only. V0.9 does not infer 1P/2P/DP variants
or link parts across IF/ELSE branches.

## Save, import and LR2 export

`Save OLRskin` is deliberately separate from ordinary LR2 `Save`. For a normal
LR2 workspace it projects the current in-memory document into a package without
modifying the source CSV files or changing the active script path. For a
workspace created by OLR import, it first saves dirty script edits into the
extracted workspace and then writes the package through a temporary file and
atomic replacement. If package replacement fails after the script save, the UI
reports the two outcomes separately.

A successful import/save associates that package path with the current
Workspace only so the next `Save OLRskin` dialog can suggest it. The path is not
portable data: a new document load clears it, and neither the package nor LR2
folder export serializes it.

Import still validates the complete archive before creating a new extraction
folder. `File > Export install-ready LR2 folder` remains available only to
imported V0.2+ workspaces and materializes a new non-existing install-ready LR2
tree.

The original-main shortcut is deliberately invalidated by evidence, not by a
guess. Import removes its marker if semantic compilation changes
`main.lr2skin`. A later save carries the marker forward only when the generated
compatibility script still equals the stored baseline, the recorded original
main is present in a captured root and no relocated fixed `assets/` are needed.
All other cases use the compatibility-script materializer so editor changes
cannot be silently discarded.

Resolution is normalized at the package boundary, not by mutating the opened
source skin. `skin.json.canvas` and `lr2/main.lr2skin` therefore agree. An
explicit or inferred SD/FHD source keeps that family; only the otherwise
unresolved OLR default becomes HD. LR2 output has one executable authority:
`#INFORMATION,...,width,height`. A legacy `#RESOLUTION` row remains visible only
behind `$OLR_IGNORED_RESOLUTION` so semantic and source-map row numbers do not
shift.

## Runtime path model

The editor does not rewrite source CSV just to make Preview work. At resource
open boundaries it resolves paths in this order:

1. the matching standalone or real skin folder found from the declaring include or
   main skin, for example `LR2files/Theme/IIDX/...` to an opened `IIDX` folder;
2. an existing process-side `LR2files/...` tree as a compatibility fallback;
3. normal paths relative to the declaring include, then the main skin; and
4. the legacy sibling-play fallback.

This document-owned order keeps a same-named `LR2files/Theme/<skin>` beside the
editor from shadowing the standalone skin that is actually open. A skin opened
inside a real LR2 installation still resolves to that installation through its
owner/main ancestors.

Wildcard patterns stay wildcard patterns so the existing LR2 selection flow
chooses the concrete file. `#INCLUDE`, `#IMAGE`, `#CUSTOMFILE`,
`#CUSTOMFOLDER`, `#LR2FONT` and `#HELPFILE` use the same resolver. The raw
CSV, row ownership, selection and save model remain unchanged.

An imported V0.2+ workspace already contains `vfs/LR2files/...`; those paths
resolve relative to its extracted `main.lr2skin`. The virtual prefix is removed
only by LR2 folder export.

## Container

An `.olrskin` file is a standard ZIP container using the stored method. PNG and
other already-compressed assets do not benefit meaningfully from another
compression layer, and the stored method keeps the native implementation small
and deterministic.

```text
example.olrskin
|-- manifest.json
|-- skin.json
|-- compatibility/
|   |-- source-map.json
|   `-- path-map.json
`-- lr2/
    |-- main.lr2skin
    |-- .olr-export-main.txt
    |-- .olr-compatibility-baseline.lr2skin  (optional V0.9)
    |-- .olr-preserve-original-main          (optional V0.9)
    |-- vfs/
    |   `-- LR2files/
    |       `-- Theme/
    |           `-- IIDX/
    |               |-- play_single.lr2skin
    |               |-- csv/
    |               |-- note/
    |               `-- Font/
    `-- assets/
        `-- image_0000.png
```

`lr2/assets/` is optional. It contains fixed images that were resolved outside a
captured LR2 root. `lr2/vfs/` is optional when no LR2-rooted declaration could
be resolved. Existing `.olrskin` containers and interrupted
`.olrskin.skineditor.tmp` files inside a captured virtual root are transport
artifacts, not LR2 assets, so export skips and counts them. This prevents a
save/import/save cycle from recursively embedding the previous package.
The two V0.9 original-main entries are a pair: validation rejects a package that
contains only one. The baseline is the exact compatibility script at package
creation; the marker grants only the conditional materialization behavior
described above.

All entry names use forward slashes and must be relative. Import rejects an
absolute path, drive prefix, empty path segment, `.` or `..` segment, backslash,
case-insensitive duplicate, encrypted entry, unsupported compression method,
CRC mismatch and ZIP64-sized entry. Extraction never overwrites an existing
target directory.

## `manifest.json`

The current V0.9 writer emits:

```json
{
  "format": "olrskin",
  "version": 9,
  "profile": "lr2-semantic-v0.9",
  "semantic_authority": "change-aware object parts + simple_mode",
  "lr2_entry": "lr2/main.lr2skin",
  "skin_entry": "skin.json",
  "path_map_entry": "compatibility/path-map.json",
  "object_count": 1,
  "part_count": 1,
  "destination_count": 1,
  "simple_slot_count": 1,
  "asset_count": 12,
  "virtual_root_count": 1,
  "virtual_file_count": 10,
  "skipped_virtual_file_count": 0,
  "unresolved_image_count": 0,
  "unresolved_resource_count": 0,
  "materialization": "original-main-if-unchanged"
}
```

`object_count` is the number of `objects.items`, `part_count` is the sum of all
item `parts`, and `destination_count` is the sum of all part `destinations`.
Every shown count is a required non-negative integer in V0.8+. Inspection checks
the four semantic/Simple Mode counts against the validated `skin.json` arrays
and checks `asset_count` and `virtual_file_count` against the ZIP entries.
Virtual-root, skipped-path and unresolved counts remain declared export-time
diagnostics; inspection validates their type and range but does not currently
recompute them. Export reports unresolved data instead of pretending it was
bundled.

V0.9 requires the manifest and `skin.json` document to both declare version 9
and the V0.9 authorities. V0.8 retains its matching version-8 authority. Legacy
inspection keeps the historical exception in
which a V0.4 manifest can contain a version-3 `skin.json`; it rejects a
version-8 document under any pre-V0.8 manifest instead of imposing version
equality on older package pairs.

## `skin.json`

`skin.json` is UTF-8 and contains:

- `metadata`: title, maker and LR2 scene type;
- `canvas`: width, height and whether the value was explicit or inferred;
- `objects.items`: compiler-authoritative Objects containing nested source-bound
  parts and their destinations;
- `sections`: Object ids grouped as `gear`, `notes`, `judge`, `combo`, `gauge`,
  `bga`, `effects`, `texts`, `ui` and `misc`;
- `simple_mode.slots`: source components grouped as `number-fonts`,
  `judgement-fonts`, `gear`, `notes` and `gauge`, with stable Object id, source command,
  source row, `gr`, crop rectangle, division grid and cycle;
- each authoritative Object's stable id, name, group, parts, source addresses
  and destination Layout, Animation and Condition representations;
  and
- `compatibility`: the LR2 entry, source map and path map.

Rows are validated compiler addresses, not general long-term identity.
`$SE_OBJECT_ID` remains the stable object identifier when it exists. The V0.4
Simple Mode compiler uses `source_row` as a deliberately narrow package address,
paired with an exact `source_command` check. Export translates the expanded
Workspace row through `compatibility/source-map.json`; it never writes an
expanded row number directly when include flattening omitted rows.

`objects.items` is serialized by the first packaged LR2 row owned by each
Object, not by the Object Editor's command-group order. `sections` keeps the
same row order within each category. This makes the semantic index follow LR2
draw order while Import still patches the byte-preserved compatibility script
at explicit `source_row` and `destination_row` addresses instead of rebuilding
or reordering its lines.

`simple_mode.authority` remains `lr2-source-v0.4`. Every slot requires `category`,
`source_command`, positive `source_row`, and all eight asset integers: `gr`, `x`,
`y`, `width`, `height`, `div_x`, `div_y`, and `cycle`. Import rejects duplicate
target rows, unsupported category/command pairs, command mismatches and
incomplete assets. A complete slot whose crop is outside the editable Simple
Mode range, including legacy LR2 negative `w/h` sentinels, is not compiled; its
original `#SRC_*` row stays byte-preserved in the compatibility script. Export
omits these rows from new `simple_mode.slots`. Valid slots are assigned only
after all structural checks pass, then the newly extracted script is replaced
atomically.

### Object part and destination contract

`objects.authority` is `lr2-destination-parts-v0.9`. An item contains its
stable identity and an ordered `parts` array. A part contains the exact source
row addresses that establish that part and one or more independently compiled
destination families.

```json
{
  "id": "skin_select_button",
  "name": "Skin select",
  "group": "BUTTON",
  "parts": [
    {
      "id": "skin_select_button:part:0",
      "sources": [
        { "source_row": 118, "source_command": "#SRC_BUTTON" }
      ],
      "destinations": [
        {
          "id": "skin_select_button:part:0:#DST_BUTTON",
          "destination_command": "#DST_BUTTON",
          "layout": {
            "destination_row": 120,
            "transform": {
              "x": 300, "y": 200, "width": 64, "height": 32,
              "rotation": 0, "blend": 1
            }
          },
          "animation": {
            "frames": [
              {
                "destination_row": 120, "time_ms": 0, "alpha": 255,
                "transform": {
                  "x": 300, "y": 200, "width": 64, "height": 32,
                  "rotation": 0, "blend": 1
                }
              }
            ]
          },
          "condition": {
            "mode": "all",
            "timer": null,
            "loop": null,
            "all": [
              {
                "slot": 1, "kind": "semantic", "key": "Gauge",
                "value": "HARD", "lr2_name": "CLEAROPTION_SURVIVAL",
                "negated": false
              },
              {
                "slot": 3, "kind": "raw", "lr2_op": 948,
                "label": "Raw LR2 OP 948"
              }
            ]
          }
        }
      ]
    }
  ]
}
```

Part boundaries are derived only from packaged LR2 row order:

1. the first `#SRC_*` row starts a part; a supported `#DST_*` that appears
   before any source starts an initial source-less part;
2. additional source rows before that part's first `#DST_*` stay in the same
   part, including NOTE Objects that declare multiple sources before their DST;
3. after a part has seen any destination row, including one whose schema stays
   compatibility-owned, the next source row starts a new part; and
4. consecutive destination rows with the same exact `destination_command` form
   one destination and its ordered animation frames; a command change starts a
   new destination run, even if an earlier command appears again later.

Only parts with at least one supported destination are serialized. A source-only
part or a part whose destinations do not expose the complete contract stays in
`lr2/main.lr2skin` without producing an empty semantic item.

This rule makes a real kamh BUTTON sequence shaped as
`SRC_BUTTON -> DST_BUTTON -> SRC_BUTTON -> DST_BUTTON` produce two parts rather
than attaching both destinations to the first source. It is a structural rule,
not variant inference: player variants and IF/ELSE ownership are left exactly
as compatibility rows.

Each destination frame follows the field-level contract introduced in V0.7:

- frame 0 must use the same row and transform as `layout`; conflicting data is
  rejected instead of choosing one silently;
- source and destination rows must be unique at their respective compiler
  boundaries, match the exact command, and expose the required fields through
  `skinHelper.txt`;
- the compiler changes only `time/x/y/w/h(or size)/a/angle/blend` for each
  frame; and
- acceleration, RGB, filter, center, extra columns, unrelated rows and original
  line endings remain compatibility-owned.

Condition ownership is intentionally partial. `timer: null` or `loop: null`
means preserve the corresponding frame-0 CSV field verbatim. Every term in
`condition.all` must name one unique `slot` from 1 through 3. Only listed slots
are compiled; an absent slot is preserved rather than cleared. Known semantic
names are resolved back to LR2 ids and raw values are emitted verbatim. The
`#DST_BARGRAPH` contract still includes its leading `(NULL)` field and distinct
`op1/op2/op3` columns, which is required for M.H-style 1P/2P visibility gates.
Validation completes before the extracted script is atomically replaced, so an
invalid Object, part or destination cannot leave partial output.

## Compatibility maps

`compatibility/source-map.json` lists the expanded row number, privacy-safe
owner label and packaged LR2 row number. Owner labels inside the main directory
are relative. An external include is recorded only as
`<external>/<filename>`. Absolute local paths are never packaged or reopened
from this map.

`lr2/main.lr2skin` omits executable `#INCLUDE` rows because the include bodies
are already flattened. It places `$OLR_FILE start` and `$OLR_FILE end` around
each included body instead. These path-free SkinEditor comments are ignored as
ordinary non-LR2 commands by compatibility readers, while the Preview runtime
uses them to reset the conditional stack at the same file boundaries as the
source document. The physical wrapper for `main.lr2skin` is never serialized,
so repeated Import and Save OLRskin operations do not accumulate markers.
When flattening, a file-local `#ELSE`, `#ELSEIF` or `#ENDIF` with no matching
file-local `#IF` is preserved as a non-executable `$OLR_IGNORED_CONTROL`
comment. Any file-local `#IF` still open at the end marker is closed with a
synthetic `#ENDIF`. This keeps the editor's flat Preview stack file-local.
When a compatibility fallback is materialized for LR2, the marker bodies are
instead written as generated sibling CSVs and replaced by `#INCLUDE` rows.
This physical boundary is required because LR2 checks only its innermost nested
IF switch; comments alone cannot stop an active child `#IF` from reactivating
rows inside an inactive parent branch.

The same compatibility main also normalizes resolution without removing a
physical row. The first `#INFORMATION` receives the OLR canvas in fields 6 and
7. Every active `#RESOLUTION` becomes
`$OLR_IGNORED_RESOLUTION,#RESOLUTION,...`. This is deliberate LR2 compatibility:
the affected legacy parser reads `#INFORMATION` correctly but can reuse stale
CSV data and write a standalone `#RESOLUTION` into the next skin-list slot.

`compatibility/path-map.json` records:

- the workspace prefix, currently `vfs/`;
- the logical path of the main skin after LR2 export;
- every logical root and its packaged virtual root;
- file and skipped-file counts per root; and
- the unresolved resource count.

Physical source directories are deliberately absent. `.olr-export-main.txt`
contains only the validated relative LR2 destination for the compiled main
skin.

## Import and LR2 export

Import validates the whole archive and extracts only the `lr2/` subtree into a
new folder. For V0.8+ it first requires the matching semantic document header,
authorities and manifest/`skin.json`/archive counts. It then compiles Simple Mode
and nested Object part destinations into the extracted `main.lr2skin`, and
deletes the complete new folder if validation or atomic replacement fails. V0.9
leaves an LR2 field token untouched when its parsed numeric value already equals
the semantic value. It also removes the original-main marker if the compiled
script differs from the stored baseline.
V0.1-V0.7 packages continue through their existing version-specific parser and
authority rules; a legacy flat Object is never reinterpreted as a V0.8/V0.9 part.
A successful V0.2+ import creates an explicitly named `*-olr-workspace`
containing `main.lr2skin`, `.olr-export-main.txt`, `vfs/` and any flat `assets/`;
Preview then follows the normal workspace load path. This extracted workspace
is not an install-ready LR2 tree. Its successful import dialog offers the LR2
export action directly so the portable `vfs/` layout is not mistaken for a
folder that can be copied into LR2 as-is. V0.1 remains Preview-loadable but does
not show that unavailable export action.

`File > Export install-ready LR2 folder` is enabled only for an imported V0.2+
workspace. It:

1. requires a new, non-existing output directory;
2. copies `vfs/LR2files/**` to `<output>/LR2files/**` without following
   symlinks;
3. copies flat `assets/**` beside the compiled main skin so relative fixed-image
   declarations remain valid;
4. requires the recorded main to be an `.lr2skin` or `.lr2ss` below the only
   roots LR2 enumerates, `LR2files/Theme` and `LR2files/Sound`;
5. if the V0.9 marker and byte-identical baseline remain valid, the recorded
   original main exists in the copied tree and no fixed assets require
   relocation, keeps its include/customization structure while normalizing the
   copied main's resolution header; otherwise it writes the current
   compatibility script to the recorded main-skin location; and
6. rewrites path-bearing CSV fields in the selected main and copied include
   scripts. `vfs/LR2files` matching is case-insensitive and accepts slash or
   backslash plus leading `.\`; rooted output uses Windows-style
   `LR2files\...`. `#INFORMATION` thumbnails, `#CUSTOMFILE/#CUSTOMFOLDER`, and
   `#HELPFILE` are rooted to the declaring script's logical LR2 folder because
   LR2 consumes them relative to its process directory. Computer-local absolute
   path fields are rejected instead of producing an export that works only on
   the source PC.

It never modifies an installed LR2 tree or the original source skin. The
exported folder can be inspected first and then copied into LR2 by the user.
`INSTALL.txt` says to merge its `LR2files` directory into the folder containing
`LR2.exe` and records the normalized main-skin path.

## Compatibility and lossless boundary

The reader accepts V0.1 through V0.9 packages. V0.1 imports remain loadable but
do not expose LR2 folder export because they have no path map or export marker.

V0.9 adds change-aware field compilation and the unchanged original-main path on
top of V0.8's nested parts. It still does not claim perfect lossless coverage for
every edited skin:

- includes are flattened into the editable compatibility script with path-free
  file-scope markers. Their original files are also present when they belonged
  to a captured root, and V0.9 uses that original graph for an unchanged LR2
  materialization;
- after a semantic or raw script edit, V0.9 safely falls back to the flattened
  compatibility main. Splitting arbitrary edited rows back into their original
  owner files is deferred to the V1.0 lossless LR2skin compiler;
- exact main-header bytes are not an invariant: active `#RESOLUTION` rows are
  intentionally neutralized and `#INFORMATION` fields 6/7 are normalized for
  LR2 safety. Other original-main rows and the include graph remain preserved;
- unresolved `LR2files/...` roots remain external and are counted;
- paths longer than the safe package-entry limit are skipped and counted;
- nested `.olrskin` containers are skipped and counted instead of recursively
  becoming LR2 assets;
- Windows ANSI/`MAX_PATH` constraints still apply to the legacy editor and ZIP
  filename encoding; and
- SkinEditor's current bundled DxLib cannot Preview legacy DXA 1.02 image-font
  archives. LR2 export preserves those archives byte-for-byte because converting
  them to DXA 1.10 would break LR2beta3 compatibility; and
- parts reflect only SRC-before/after-DST row boundaries; 1P/2P/DP or other
  variants are not inferred or linked automatically;
- the Object Inspector currently edits only the first destination command
  family in the selected Object that exposes the complete semantic contract. It
  has no nested part/destination-family selector; other families remain in
  `Advanced LR2` even though export and import support the nested V0.9 shape;
- IF/ELSE control flow, per-frame shared-field oddities, event logic and
  unsupported commands stay compatibility-owned; and
- `sections` remains a semantic navigation index and never compiles values; and
- the current SkinEditor package writer does not preserve unknown manifest or
  `skin.json` fields when `Save OLRskin` rewrites a package. Producers must not
  rely on unknown extension fields surviving an editor save.

## Versioning rules

- V0.9 readers must continue to dispatch V0.1-V0.8 packages to their existing
  parser and authority behavior.
- A V0.2+ package requires both `compatibility/path-map.json` and
  `lr2/.olr-export-main.txt`.
- A change in which layer is authoritative requires a new format version.
- Unknown-field round trips are not supported by the current writer. A future
  extension policy must define namespacing and either passthrough preservation
  or an explicit rewrite boundary before optional third-party fields are treated
  as portable.

## Verification

The `olr-package` self-test first checks an M.H-shaped 1P/2P wrapper whose two
flattened include bodies contain file-local orphan `#ELSE` rows. It verifies
that `$OLR_FILE` boundaries keep only the selected side active and remain
stable when a package is saved again. It then creates a synthetic virtual theme
and an `SEOLRSkinDocument` whose two parts are already explicit. It writes and
inspects that V0.9 package with one Simple Mode slot and two destinations,
extracts and compiles it,
verifies virtual and fixed asset bytes, preserves an empty numeric zero token,
materializes both compatibility and unchanged original-include LR2 trees, checks
that their executable resolution authority is `#INFORMATION`, checks that an
edited compatibility script disables original-main reuse, checks restored
script paths including mixed-case/slash virtual aliases and direct LR2 path
fields, enumerates the relocated export through LR2's own `MakeSkinList()`,
rejects non-discoverable main destinations, unsafe roots and CRC tampering, and tests
standalone LR2-root resolution. Its compiler fixture changes all eight supported
asset fields. Direct compiler fixtures validate multiple source bindings in one
explicit part and an explicitly described two-part structure. Destination
fixtures verify writer-produced `object_count`, `part_count` and
`destination_count`, change Layout and frame time/alpha/geometry, compile named
and raw OP terms by explicit slot, and preserve a null timer, null loop and an
omitted OP slot. They also preserve raw rows, trailing columns and mixed line
endings. The retained V0.7 flat-Object fixture verifies its legacy authority
without converting it to parts. A dedicated M.H regression keeps `#DST_BARGRAPH`
`loop/timer/op1/op2/op3` in columns 16-20, and a virtual-root fixture proves that
a previous `.olrskin` is not embedded. A legacy crop fixture verifies that
negative `#SRC_IMAGE` dimensions remain raw rather than aborting Import. The
test also rejects invalid nested row ownership and Layout/frame-0 divergence
without returning partial output. It does not call `WORKSPACE::ExportOlrSkin()`,
so it does not automatically prove that real Workspace row order derives the
intended part boundaries.

Manual verification must therefore additionally open the actual standalone
kamh skin and an M.H/IIDX-style skin. For kamh, inspect every BUTTON Object in
`skin.json` and
confirm that a source after a destination starts a new part, then Import and
compare both BUTTON variants in Preview. For NOTE Objects, confirm that multiple
sources before the first DST stay together. In both skins exercise Preview
wildcards, notes, explosions, judge/combo, gauge and fonts, repeat Preview after
Import, and inspect the LR2 export in an actual LR2 installation. The optional
`SKINEDITOR_TEST_OLR_PACKAGE` hook may point at the saved kamh package to run the
production import core in a temporary directory without modifying the package.
