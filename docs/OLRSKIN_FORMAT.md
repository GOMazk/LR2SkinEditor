# OLR Skin package format

`.olrskin` is an AI-readable intermediate package for LR2 skins. It keeps a
human-facing semantic index next to the compatibility data needed to return to
LR2 without silently deleting commands the editor does not understand.

## V0.2 scope

V0.2 adds a virtual LR2 filesystem to the compatibility-first V0.1 format:

- export the currently loaded, fully expanded LR2 document as one ZIP package;
- classify editor Objects into semantic categories in `skin.json`;
- retain all LR2 commands, comments, conditions, timers and editor metadata in
  `lr2/main.lr2skin`;
- map resolvable `LR2files/...` declarations to `vfs/LR2files/...` while the
  skin is edited or previewed;
- bundle each resolved logical LR2 root, including wildcard choices, fonts and
  archive-backed resources, under `lr2/vfs/LR2files/...`;
- keep fixed non-LR2-rooted images under `lr2/assets/`; and
- materialize a new install-ready `LR2files/...` tree only when the user invokes
  `File > Export LR2 folder` from an imported V0.2 workspace.

The semantic layer remains descriptive. The compatibility script is the
compilation authority. Consumers must not assume that changing `skin.json`
changes LR2 output until a later compiler explicitly promotes that semantic
area and provides round-trip tests.

## Runtime path model

The editor does not rewrite source CSV just to make Preview work. At resource
open boundaries it resolves paths in this order:

1. an existing real `LR2files/...` tree;
2. the matching standalone skin folder found from the declaring include or
   main skin, for example `LR2files/Theme/IIDX/...` to an opened `IIDX` folder;
3. normal paths relative to the declaring include, then the main skin; and
4. the legacy sibling-play fallback.

Wildcard patterns stay wildcard patterns so the existing LR2 selection flow
chooses the concrete file. `#INCLUDE`, `#IMAGE`, `#CUSTOMFILE`,
`#CUSTOMFOLDER`, `#LR2FONT` and `#HELPFILE` use the same resolver. The raw
CSV, row ownership, selection and save model remain unchanged.

An imported V0.2 workspace already contains `vfs/LR2files/...`; those paths
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

Required V0.2 fields include:

```json
{
  "format": "olrskin",
  "version": 2,
  "profile": "lr2-vfs-v0.2",
  "semantic_authority": "descriptive",
  "lr2_entry": "lr2/main.lr2skin",
  "skin_entry": "skin.json",
  "path_map_entry": "compatibility/path-map.json"
}
```

The manifest also records object, asset, virtual-root, virtual-file, skipped
path and unresolved-resource counts. Export reports unresolved data instead of
pretending it was bundled.

## `skin.json`

`skin.json` is UTF-8 and contains:

- `metadata`: title, maker and LR2 scene type;
- `canvas`: width, height and whether the value was explicit or inferred;
- `sections`: semantic Objects grouped as `gear`, `notes`, `judge`, `combo`,
  `gauge`, `bga`, `effects`, `texts`, `ui` and `misc`;
- each Object's stable editor id when available, group, source/destination
  commands, source row ids, first destination rectangle, timer, loop and ops;
  and
- `compatibility`: the LR2 entry, source map and path map.

Rows are discovery aids, not long-term identity. `$SE_OBJECT_ID` remains the
stable identifier when it exists.

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
new folder. This creates `main.lr2skin`, `.olr-export-main.txt`, `vfs/` and any
flat `assets/`. Preview then follows the normal workspace load path.

`File > Export LR2 folder` is enabled only for an imported V0.2 workspace. It:

1. requires a new, non-existing output directory;
2. copies `vfs/LR2files/**` to `<output>/LR2files/**` without following
   symlinks;
3. restores virtual CSV fields from `vfs/LR2files/...` to Windows-style
   `LR2files\...`; and
4. writes the current edited compatibility script to the recorded main-skin
   location, replacing only the copied file inside that new output tree.

It never modifies an installed LR2 tree or the original source skin. The
exported folder can be inspected first and then copied into LR2 by the user.

## Compatibility and lossless boundary

The reader accepts both V0.1 and V0.2 packages. V0.1 imports remain loadable but
do not expose LR2 folder export because they have no path map or export marker.

V0.2 substantially improves portability, but does not claim perfect lossless
coverage for every skin:

- includes are flattened into the authoritative compatibility script; their
  original files are also present when they belonged to a captured root;
- unresolved `LR2files/...` roots remain external and are counted;
- paths longer than the safe package-entry limit are skipped and counted;
- Windows ANSI/`MAX_PATH` constraints still apply to the legacy editor and ZIP
  filename encoding; and
- semantic edits are not compiled back into LR2 yet.

## Versioning rules

- V0.2 readers must continue to accept V0.1 extraction.
- A V0.2 package requires both `compatibility/path-map.json` and
  `lr2/.olr-export-main.txt`.
- A change in which layer is authoritative requires a new format version.
- Unknown JSON fields must be preserved by tools that rewrite a package, or the
  tool must state that it only performs extraction.

## Verification

The `olr-package` self-test creates a synthetic virtual theme, writes and
inspects a V0.2 package, extracts it, verifies virtual resource bytes,
materializes a new LR2 tree, checks restored script paths, rejects unsafe roots
and CRC tampering, and tests standalone LR2-root resolution. Manual verification
must additionally open a real standalone M.H/IIDX-style skin, exercise Preview
wildcards, notes, explosions, judge/combo, gauge and fonts, import the package,
repeat Preview, and inspect the LR2 export in an actual LR2 installation.
