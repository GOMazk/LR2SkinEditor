# OLR Skin package format

`.olrskin` is an AI-readable intermediate package for LR2 skins. It keeps a
human-facing semantic index next to the compatibility data needed to return to
LR2 without silently deleting commands the editor does not understand.

## V0.7 scope

V0.7 keeps the V0.4 Simple Mode source compiler and promotes the first supported
`#DST_*` family for each Object into one destination semantic authority:

- save the currently loaded, fully expanded LR2 document as one ZIP package
  through `File > Save OLRskin`;
- classify editor Objects into semantic categories in `skin.json`;
- list number-font, judgement-font, gear, note and gauge source slots with their
  packaged LR2 row, command, graphic crop and animation grid in
  `skin.json.simple_mode`;
- compile edited Simple Mode `gr`, crop, division and cycle values back into the
  matching `#SRC_*` row during V0.4 import;
- expose the first destination rectangle as `layout.transform` (`x`, `y`,
  `width`, `height`, `rotation`, `blend`);
- expose the matching destination rows as ordered animation frames with
  `time_ms`, `alpha` and a full transform;
- expose the first destination's `timer`, `loop` and three OP slots as an
  all-mode condition;
- compile known OP/TIMER names through the LR2 name table, while keeping custom
  900-series or unknown values as numeric `raw` terms;
- retain every unsupported command, alternate destination command family,
  comment, IF/ELSE branch and editor metadata in `lr2/main.lr2skin`;
- map resolvable `LR2files/...` declarations to `vfs/LR2files/...` while the
  skin is edited or previewed;
- bundle each resolved logical LR2 root, including wildcard choices, fonts and
  archive-backed resources, under `lr2/vfs/LR2files/...`;
- keep fixed non-LR2-rooted images under `lr2/assets/`; and
- materialize a new install-ready `LR2files/...` tree only when the user invokes
  `File > Export LR2 folder` from an imported V0.2+ workspace.

`skin.json.objects` and `skin.json.simple_mode` are the only compiler inputs.
`sections` contains Object ids for navigation, not duplicate values. The raw LR2
script remains compatibility authority for unsupported SRC fields, alternate
DST command families, control flow, events, comments, metadata and unknown
commands. V0.8 variant linkage is deliberately outside this version.

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
folder. `File > Export LR2 folder` remains available only to imported V0.2+
workspaces and materializes a new non-existing install-ready LR2 tree.

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

All entry names use forward slashes and must be relative. Import rejects an
absolute path, drive prefix, empty path segment, `.` or `..` segment, backslash,
case-insensitive duplicate, encrypted entry, unsupported compression method,
CRC mismatch and ZIP64-sized entry. Extraction never overwrites an existing
target directory.

## `manifest.json`

Required V0.7 fields include:

```json
{
  "format": "olrskin",
  "version": 7,
  "profile": "lr2-semantic-v0.7",
  "semantic_authority": "objects + simple_mode",
  "lr2_entry": "lr2/main.lr2skin",
  "skin_entry": "skin.json",
  "path_map_entry": "compatibility/path-map.json"
}
```

The manifest also records Object, Simple Mode slot, asset, virtual-root,
virtual-file, skipped-path and unresolved-resource counts. Export reports
unresolved data instead of pretending it was bundled.

## `skin.json`

`skin.json` is UTF-8 and contains:

- `metadata`: title, maker and LR2 scene type;
- `canvas`: width, height and whether the value was explicit or inferred;
- `objects.items`: compiler-authoritative destination Objects;
- `sections`: Object ids grouped as `gear`, `notes`, `judge`, `combo`, `gauge`,
  `bga`, `effects`, `texts`, `ui` and `misc`;
- `simple_mode.slots`: source components grouped as `number-fonts`,
  `judgement-fonts`, `gear`, `notes` and `gauge`, with stable Object id, source command,
  source row, `gr`, crop rectangle, division grid and cycle;
- each authoritative Object's stable id, source/destination commands, Layout,
  Animation and Condition representations;
  and
- `compatibility`: the LR2 entry, source map and path map.

Rows are validated compiler addresses, not general long-term identity.
`$SE_OBJECT_ID` remains the stable object identifier when it exists. The V0.4
Simple Mode compiler uses `source_row` as a deliberately narrow package address,
paired with an exact `source_command` check. Export translates the expanded
Workspace row through `compatibility/source-map.json`; it never writes an
expanded row number directly when include flattening omitted rows.

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

### Destination Object contract

`objects.authority` is `lr2-destination-v0.7`. Each item owns exactly one
`destination_command` family. This is the first supported family found in the
editor Object; other families remain raw until V0.8 linkage exists.

```json
{
  "id": "judge_1p",
  "destination_command": "#DST_IMAGE",
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
        "destination_row": 120, "time_ms": 0, "alpha": 0,
        "transform": {
          "x": 300, "y": 200, "width": 64, "height": 32,
          "rotation": 0, "blend": 1
        }
      }
    ]
  },
  "condition": {
    "mode": "all",
    "timer": { "kind": "semantic", "lr2_name": "JUDGETIMER_1P" },
    "loop": 800,
    "all": [
      {
        "kind": "semantic", "key": "Gauge", "value": "HARD",
        "lr2_name": "CLEAROPTION_SURVIVAL", "negated": false
      },
      { "kind": "raw", "lr2_op": 948, "label": "Raw LR2 OP 948" }
    ]
  }
}
```

Frame 0 must use the same row and transform as `layout`; conflicting data is
rejected instead of choosing one silently. Destination rows must be unique,
match the exact command, and expose the required fields through
`skinHelper.txt`. The compiler changes only `time/x/y/w/h(or size)/a/angle/blend`
for each frame and `loop/timer/op1/op2/op3` on frame 0. It preserves acceleration,
RGB, filter, center, extra columns, unrelated rows and original line endings.
Optional zero OPs also preserve an originally empty CSV field. The
`#DST_BARGRAPH` contract includes its leading `(NULL)` field and distinct
`op1/op2/op3` columns; this is required for M.H-style 1P/2P visibility gates to
survive import.
Known semantic names are resolved back to LR2 ids. Raw values are emitted
verbatim. Validation completes before the extracted script is atomically
replaced, so an invalid Object cannot leave partial output.

## Compatibility maps

`compatibility/source-map.json` lists the expanded row number, privacy-safe
owner label and packaged LR2 row number. Owner labels inside the main directory
are relative. An external include is recorded only as
`<external>/<filename>`. Absolute local paths are never packaged or reopened
from this map.

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
new folder. For V0.7 it captures the validated `skin.json`, compiles Simple Mode
and destination Objects into the extracted `main.lr2skin`, and deletes the
complete new folder if validation or atomic replacement fails. V0.1-V0.3 keep
their compatibility script unchanged; V0.4 documents still compile only Simple
Mode because they have no `objects` authority. A successful import creates `main.lr2skin`,
`.olr-export-main.txt`, `vfs/` and any flat `assets/`; Preview then follows the
normal workspace load path.

`File > Export LR2 folder` is enabled only for an imported V0.2+ workspace. It:

1. requires a new, non-existing output directory;
2. copies `vfs/LR2files/**` to `<output>/LR2files/**` without following
   symlinks;
3. copies flat `assets/**` beside the compiled main skin so relative fixed-image
   declarations remain valid;
4. restores virtual CSV fields from `vfs/LR2files/...` to Windows-style
   `LR2files\...`; and
5. writes the current edited compatibility script to the recorded main-skin
   location, replacing only the copied file inside that new output tree.

It never modifies an installed LR2 tree or the original source skin. The
exported folder can be inspected first and then copied into LR2 by the user.

## Compatibility and lossless boundary

The reader accepts V0.1 through V0.7 packages. V0.1 imports remain loadable but
do not expose LR2 folder export because they have no path map or export marker.

V0.7 adds tested destination geometry, animation and condition compilers, but
does not claim perfect lossless
coverage for every skin:

- includes are flattened into the authoritative compatibility script; their
  original files are also present when they belonged to a captured root;
- unresolved `LR2files/...` roots remain external and are counted;
- paths longer than the safe package-entry limit are skipped and counted;
- nested `.olrskin` containers are skipped and counted instead of recursively
  becoming LR2 assets;
- Windows ANSI/`MAX_PATH` constraints still apply to the legacy editor and ZIP
  filename encoding; and
- only the first supported destination command family per Object is semantic;
  1P/2P/DP variants are not linked yet;
- IF/ELSE control flow, per-frame shared-field oddities, event logic and
  unsupported commands stay compatibility-owned; and
- `sections` remains a semantic navigation index and never compiles values.

## Versioning rules

- V0.7 readers must continue to accept V0.1-V0.6 extraction.
- A V0.2+ package requires both `compatibility/path-map.json` and
  `lr2/.olr-export-main.txt`.
- A change in which layer is authoritative requires a new format version.
- Unknown JSON fields must be preserved by tools that rewrite a package, or the
  tool must state that it only performs extraction.

## Verification

The `olr-package` self-test creates a synthetic virtual theme, writes and
inspects a V0.7 package with one Simple Mode slot and a two-frame semantic
Object, extracts and compiles it,
verifies virtual and fixed asset bytes, materializes a new LR2 tree, checks
restored script paths, rejects unsafe roots and CRC tampering, and tests
standalone LR2-root resolution. Its compiler fixture changes all eight supported
asset fields. Its destination fixture changes Layout, frame time/alpha/geometry,
a named timer, a known semantic OP and raw OP 948; it preserves raw rows,
trailing columns, empty optional OP fields and mixed line endings. A dedicated
M.H regression fixture keeps `#DST_BARGRAPH` loop/timer/op1/op2/op3 in columns
16-20, and a virtual-root fixture proves that a previous `.olrskin` is not
embedded. A legacy crop fixture verifies that negative `#SRC_IMAGE` dimensions
remain raw rather than aborting Import. The test also rejects Layout/frame-0
divergence without returning partial output. Manual verification
must additionally open a real standalone M.H/IIDX-style skin, exercise Preview
wildcards, notes, explosions, judge/combo, gauge and fonts, import the package,
repeat Preview, and inspect the LR2 export in an actual LR2 installation.
