# OLR Skin package format

`.olrskin` is an AI-readable intermediate package for LR2 skins. It separates
human-facing semantic structure from the compatibility data required to return
to LR2 without silently deleting commands the editor does not understand.

## V0.1 scope

V0.1 is a compatibility-first vertical slice:

- export the currently loaded, fully expanded LR2 document as one ZIP package;
- classify editor Objects into semantic categories in `skin.json`;
- retain all LR2 commands, comments, conditions, timers and editor metadata in
  a merged `lr2/main.lr2skin` compatibility script;
- bundle concrete `#IMAGE` files that the editor resolved successfully and
  rewrite those declarations to package-relative paths; and
- extract `lr2/` safely to a new folder so the result can be loaded again.

The semantic layer is descriptive in V0.1. The compatibility script remains
the compilation authority. A later format version may make supported semantic
fields authoritative only after the compiler can prove that their source rows
and LR2 invariants are preserved. Consumers must not assume that changing
`skin.json` changes the compiled LR2 output in V0.1.

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
|   `-- source-map.json
`-- lr2/
    |-- main.lr2skin
    `-- assets/
        |-- image_0000.png
        `-- image_0001.png
```

All entry names use forward slashes and must be relative. Import rejects an
absolute path, drive prefix, empty path segment, `.` or `..` segment, backslash,
encrypted entry, unsupported compression method, CRC mismatch and ZIP64-sized
entry. Extraction never overwrites an existing target directory.

## `manifest.json`

Required fields:

```json
{
  "format": "olrskin",
  "version": 1,
  "profile": "lr2-compat-v0.1",
  "semantic_authority": "descriptive",
  "lr2_entry": "lr2/main.lr2skin",
  "skin_entry": "skin.json"
}
```

The manifest also records counts and limitations. V0.1 reports unresolved or
dynamic image declarations instead of pretending they were bundled.

## `skin.json`

`skin.json` is UTF-8 and contains:

- `metadata`: title, maker and LR2 scene type;
- `canvas`: width, height and whether the value was explicit or inferred;
- `sections`: semantic Objects grouped as `gear`, `notes`, `judge`, `combo`,
  `gauge`, `bga`, `effects`, `texts`, `ui` and `misc`;
- each Object's stable editor id when available, group, source/destination
  commands, source row ids, first destination rectangle, timer, loop and ops;
  and
- `compatibility`: the LR2 entry and source-map entry that preserve unsupported
  data.

Rows are discovery aids, not long-term identity. `$SE_OBJECT_ID` remains the
stable identifier when it exists.

## `compatibility/source-map.json`

The source map lists the expanded row number, privacy-safe owner label and packaged
LR2 row number. It allows a later compiler or AI tool to explain where a merged
row came from without requiring `$FILE` pseudo-lines in the LR2 output.

Owner labels are relative to the main skin directory. An include outside that
directory is recorded only as `<external>/<filename>`. Absolute local paths are
never packaged, and import never writes to or reopens source-map owner labels.

## Lossless boundary

Script commands are preserved, but V0.1 deliberately does not claim a fully
portable lossless package for every LR2 skin:

- wildcard `#IMAGE` and `#CUSTOMFILE` choices can depend on the user's LR2
  installation;
- fonts, video, sound and arbitrary paths are not bundled yet;
- includes are merged into one script, while their original ownership is kept
  only in the source map; and
- semantic edits are not compiled back into LR2 yet.

These limitations must remain visible in the Export result and manifest. A
future version can promote one semantic area at a time only with import,
compile and round-trip regression tests.

## Versioning rules

- Readers reject an unsupported major `version`.
- New optional fields may be added without changing version 1.
- A change in which layer is authoritative requires a new format version.
- Unknown JSON fields must be preserved by tools that rewrite a package, or
  the tool must state that it only performs extraction.

## Verification

The `olr-package` self-test creates a synthetic package, inspects its entries,
extracts it, verifies JSON and LR2 bytes, checks CRC/path traversal rejection,
and removes the temporary directory. Manual verification should additionally
export a real HD skin, inspect the package with a normal ZIP viewer, import it
to a new folder and load the extracted `main.lr2skin`.
