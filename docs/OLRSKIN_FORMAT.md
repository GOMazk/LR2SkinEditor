# OLR Skin package format

`.olrskin` is an AI-readable intermediate package for LR2 skins. It keeps a
human-facing semantic index next to the compatibility data needed to return to
LR2 without silently deleting commands the editor does not understand.

## V0.4 scope

V0.4 keeps the virtual LR2 filesystem introduced in V0.2 and promotes the V0.3
Simple Mode projection into the first deliberately small semantic compiler:

- export the currently loaded, fully expanded LR2 document as one ZIP package;
- classify editor Objects into semantic categories in `skin.json`;
- list number-font, judgement-font, gear, note and gauge source slots with their
  packaged LR2 row, command, graphic crop and animation grid in
  `skin.json.simple_mode`;
- compile edited Simple Mode `gr`, crop, division and cycle values back into the
  matching `#SRC_*` row during V0.4 import;
- retain all LR2 commands, comments, conditions, timers and editor metadata in
  `lr2/main.lr2skin`;
- map resolvable `LR2files/...` declarations to `vfs/LR2files/...` while the
  skin is edited or previewed;
- bundle each resolved logical LR2 root, including wildcard choices, fonts and
  archive-backed resources, under `lr2/vfs/LR2files/...`;
- keep fixed non-LR2-rooted images under `lr2/assets/`; and
- materialize a new install-ready `LR2files/...` tree only when the user invokes
  `File > Export LR2 folder` from an imported V0.2+ workspace.

Only `skin.json.simple_mode` has compile authority. Semantic `sections` remain
descriptive, and `lr2/main.lr2skin` remains the compatibility authority for all
destination placement, conditions, timers, ops, events, comments, metadata and
unknown commands. A consumer must not compile any field outside the V0.4 Simple
Mode contract merely because a similar value appears in `sections`.

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
be resolved.

All entry names use forward slashes and must be relative. Import rejects an
absolute path, drive prefix, empty path segment, `.` or `..` segment, backslash,
case-insensitive duplicate, encrypted entry, unsupported compression method,
CRC mismatch and ZIP64-sized entry. Extraction never overwrites an existing
target directory.

## `manifest.json`

Required V0.4 fields include:

```json
{
  "format": "olrskin",
  "version": 4,
  "profile": "lr2-simple-v0.4",
  "semantic_authority": "simple_mode",
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
- `sections`: semantic Objects grouped as `gear`, `notes`, `judge`, `combo`,
  `gauge`, `bga`, `effects`, `texts`, `ui` and `misc`;
- `simple_mode.slots`: source components grouped as `number-fonts`,
  `judgement-fonts`, `gear`, `notes` and `gauge`, with stable Object id, source command,
  source row, `gr`, crop rectangle, division grid and cycle;
- each Object's stable editor id when available, group, source/destination
  commands, source row ids, first destination rectangle, timer, loop and ops;
  and
- `compatibility`: the LR2 entry, source map and path map.

Rows are discovery aids for descriptive sections, not general long-term identity.
`$SE_OBJECT_ID` remains the stable object identifier when it exists. The V0.4
Simple Mode compiler uses `source_row` as a deliberately narrow package address,
paired with an exact `source_command` check. Export translates the expanded
Workspace row through `compatibility/source-map.json`; it never writes an
expanded row number directly when include flattening omitted rows.

`simple_mode.authority` is `lr2-source-v0.4`. Every slot requires `category`,
`source_command`, positive `source_row`, and all eight asset integers: `gr`, `x`,
`y`, `width`, `height`, `div_x`, `div_y`, and `cycle`. Import rejects duplicate
target rows, unsupported category/command pairs, command mismatches, invalid
ranges and incomplete assets. It assigns compiled output only after every slot
passes, then replaces the newly extracted script atomically.

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
new folder. For V0.4 it captures the validated `skin.json`, compiles Simple Mode
into the extracted `main.lr2skin`, and deletes the complete new folder if
validation or atomic replacement fails. V0.1-V0.3 keep their compatibility
script unchanged. A successful import creates `main.lr2skin`,
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

The reader accepts V0.1, V0.2, V0.3 and V0.4 packages. V0.1 imports remain loadable but
do not expose LR2 folder export because they have no path map or export marker.

V0.4 adds a tested source-asset compiler, but does not claim perfect lossless
coverage for every skin:

- includes are flattened into the authoritative compatibility script; their
  original files are also present when they belonged to a captured root;
- unresolved `LR2files/...` roots remain external and are counted;
- paths longer than the safe package-entry limit are skipped and counted;
- Windows ANSI/`MAX_PATH` constraints still apply to the legacy editor and ZIP
  filename encoding; and
- semantic `sections` are not compiler input yet. Only validated Simple Mode
  source asset fields, including gauge image sources, compile; destination
  geometry, animation/event models, gauge layout behavior and unsupported LR2
  features stay compatibility-owned.

## Versioning rules

- V0.4 readers must continue to accept V0.1, V0.2 and V0.3 extraction.
- A V0.2+ package requires both `compatibility/path-map.json` and
  `lr2/.olr-export-main.txt`.
- A change in which layer is authoritative requires a new format version.
- Unknown JSON fields must be preserved by tools that rewrite a package, or the
  tool must state that it only performs extraction.

## Verification

The `olr-package` self-test creates a synthetic virtual theme, writes and
inspects a V0.4 package with one Simple Mode slot, extracts and compiles it,
verifies virtual and fixed asset bytes, materializes a new LR2 tree, checks
restored script paths, rejects unsafe roots and CRC tampering, and tests
standalone LR2-root resolution. Its compiler fixture changes all eight supported
asset fields, preserves raw rows, trailing columns and mixed line endings, and
rejects a row/command mismatch without returning partial output. Manual verification
must additionally open a real standalone M.H/IIDX-style skin, exercise Preview
wildcards, notes, explosions, judge/combo, gauge and fonts, import the package,
repeat Preview, and inspect the LR2 export in an actual LR2 installation.
