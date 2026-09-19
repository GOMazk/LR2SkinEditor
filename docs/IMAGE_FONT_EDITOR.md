# Image Font Editor

Open **Windows > Assets > Image Font Editor**. This edits LR2 text image fonts
(`.lr2font` / `#LR2FONT`), not NUMBER/NOWCOMBO/JUDGE atlases in Simple Mode.

## Workflow

1. Choose **Fonts in current Preview** to open an active font slot. The actual
   load records the path after IF, CUSTOMFILE and archive-aware wildcard resolution.
   Entries show the slot and parent/file name (such as `0 : barfnt/font.lr2font`);
   hover for the complete path. Missing/dummy fonts are not presented as `ERROR`
   entries. Alternatively
   use **Open font / DXA...** for an unpacked font or a legacy DXA archive.
   When an archive contains several `.lr2font` files, choose one from the dialog.
2. Search/select a character by LR2 code or actual character. Clicking a rectangle
   in the atlas also selects it. The right pane shows its crop, texture page and
   sample text. Atlas zoom supports Ctrl+wheel, up to 1600%.
3. Edit **Height (#S)**, **Spacing (#M)**, **Page ID/path (#T)** or the character's
   **code/page/x/y/width/height (#R)**. Character input converts CP932 to LR2's
   custom index: these numbers are NOT Unicode code points. Negative spacing is
   supported. New character adds a mapping; Remove character asks for confirmation.
4. **Apply edits** changes only the draft; **Revert fields** cancels unapplied
   form input. Apply/revert before selecting another character/page.
   Changing Page ID adds/updates that page and leaves the old one intact.
5. **Save font** (Ctrl+S with this pane focused) writes the external `.lr2font`
   (or replaces that definition inside its DXA)
   and reloads matching runtime fonts in this Workspace. It does not save/change
   skin CSVs, Object selection, script revision or script History.
   Font Undo/Redo is independent and remains available after saving.
6. **Reload** rereads the font and refreshes matching runtime fonts, with
   Save/discard/cancel for pending edits.
   **Reload images** refreshes image previews after external pixel editing.
   Hiding the tool or opening another skin keeps its draft in this Workspace;
   save before closing the Workspace/application. Other Workspaces using the same
   font need their own reload; stale drafts cannot overwrite an externally changed file.

## Safety and limits

- CP932, unedited lines, unknown directives, comments, trailing columns,
  CRLF/LF and a missing final newline are retained.
- Duplicate declarations follow LR2's last-one-wins rule. Removing/renaming a
  character comments out shadowed rows too, preventing the old mapping's return.
- Save reuses the script transaction's temp/backup/verify/rollback implementation.
  A changed source, unreadable file or existing recovery backup blocks saving.
  This is not a multi-file crash-recovery journal.
- Legacy DXA format versions 1-4, with the default key, can be inspected and
  edited in place. No unpacked folder is created and no skin CSV path is changed.
  Save keeps the archive version, names, timestamps and unrelated compressed
  payloads. The changed font is stored uncompressed; its old slot is reused if
  possible, otherwise it is appended before the index. Archives may grow after
  edits; this is not a general-purpose archive repacker.
  Custom passwords, format versions 5+ and oversized archives are not editable.
  Limits: 128 MiB archive, 4 MiB index, 64 MiB decoded image page. Unsupported or
  malformed input fails without modifying disk. Do not confuse DXA file-format
  version 3 with the DxaEncode tool's version 1.02.
- This window edits mapping/metrics, not bitmap pixels, TTF generation or skin
  `#LR2FONT` declarations. Image paths are relative to the font directory.
  Limits: 8 MiB per font, 1000 page IDs and LR2 character codes below 0x3BCE.
- Preview loads at most 16 pages (each up to 4 megapixels), lazily. Changing the
  selected page or Reload images clears the bounded cache. Missing images and
  empty/out-of-bounds crops are indicated; missing sample characters fall back to `?`.

## Implementation and verification

`imageFontDocument.*` owns lossless parsing, validated draft mutations, CP932
mapping and independent Undo/Redo. WORKSPACE's `SEImageFontEditor` owns forms,
selection and preview textures. `winWorkspaceImageFont.cpp` draws the tool and
refreshes runtime glyph caches after saving. `scriptFileSave.h` exposes the existing
binary-safe transaction for external assets without duplicating rollback logic.

The custom **DxLib 3.24f with legacy DXA reader** in `lib/DxLib` is built by
`lib/DxLibCustom.vcxproj` (core + draw + Ogg adapters, v143 /MD). The project references
this library for Release Win32/x64 and explicitly excludes the old precompiled
core/draw libraries; remaining codec/CRT binaries are in `lib/DxLib_unk`.
The old Ogg adapter has incompatible internal graphics type names and is rebuilt
from the supplied source with the pinned public headers in `lib/DxLibCodecHeaders`.
`scripts/build.ps1` builds this dependency automatically. Generated files stay in
`.build/dxlib` and `.build/dxlib-obj`, not the vendor source tree.
Native Preview continues using LR2's original `ReadImageFont` / `LoadGraph` calls;
no alternate skin parser or runtime font loader is installed.

`imageFontArchive.*` is the editor-only conservative writer/index validator.
Its bounded decompressor validates the immutable save snapshot and resource bytes
before native decompression; actual font/image reads use the custom library's
`FileRead_*` API. Opening compares both readers
before allowing edits. Saving releases idle DxLib archive handles before atomic
replacement, avoiding stale indexes and Windows sharing violations.
Format references: bundled `lib/DxLib/DxArchive_.{h,cpp}` and Takumi Yamada's
[original DXArchive source](https://github.com/henteko/2012TeResAI/blob/master/2012TeResAI/2012teresAI/DX%20lib/DxLib_VC/Tool/DXArchive/Source/DxArchive/DXArchive.h).
The writer is a separate bounds-checked implementation, not the upstream extractor.
OLRskin 0.9 packaging and unchanged archive byte-preservation contracts are unchanged.

`--self-test-image-font` tests byte preservation, duplicates, Japanese/halfwidth
codes, validation, undo/redo, save/reopen, external conflicts, locked-file failure,
retained-backup protection, workspace isolation, catalog identity and ImGui frames.
Actual LR2 reader checks verify same-directory reload and removal of stale glyphs.
It is included in `scripts/test.ps1`; it does not edit real fonts or skin files.

`--self-test-image-font-dxa` tests owned synthetic nested archives, versions 1-4,
compressed/uncompressed entries, overlapping LZ references, unchanged image/member
bytes, in-place/appended saves, native DxLib reads and actual LR2 font reload,
conflicts, recovery backups, sharing locks and a changed working directory after
opening. A read-only real-file diagnostic is:

```powershell
SkinEditor_DX9/Release/SkinEditor_DX9.exe --check-image-font-dxa "path/to/font.dxa"
```

It reads every font and declared image page without writing the source archive.

Manual QA: open multi-page SELECT fonts, compare crops/sample text, edit a glyph
or spacing, save and compare Preview, then reopen in LR2. Check narrow docks, zoom,
shortcuts, unsaved switch/reload, missing images, shared fonts and DXA save/reload.
GPU rendering, native dialogs and actual LR2 output are outside headless checks.

### Local validation — 2026-09-19

- Release Win32 and x64 builds succeeded with the custom DxLib source dependency.
  The complete builds still report legacy application/vendor compiler warnings;
  the final incremental builds reported no errors or warnings.
- `scripts/test.ps1` passed all 20 suites on each architecture, including DXA
  versions 1-4, save/reload, preserved members and the changed-directory case.
- `ui-map.ps1 -Check`, `docs-check.ps1` and `ai-context.ps1 -Check` passed.
- Read-only checks of tricoro `system.dxa`, `course_decide_title.dxa` and
  `title.dxa` read 7992/7992/8468 glyph definitions and 5/12/13 image pages.
  `bar.dxa` declares `font_09.png` but its original archive has pages 00-08 only;
  the diagnostic correctly reports the missing member rather than inventing one.
- `--agent-render` successfully rendered tricoro DECIDE using the custom DxLib;
  the resulting PNG was visually checked for image-font output. Source skins
  and archives were not modified. Interactive editor use and actual LR2 playback
  remain manual checks.

### Preview font-list correction

`ReadSkinSE` formerly passed every `#LR2FONT` path through the Win32-only
`GetRandomFileNoError`. DXA members therefore became the literal `ERROR` before
the custom DxLib could read them. Direct archive-opening tests did not cover
this registration step; rendered fallback text was not proof of the DXA binding.
`SEFindPreviewImageFontPath` now uses the existing skin/include root resolver,
then DxLib `FileRead_findFirst/Next/Close` for both disk and DXA font files.
Reserved-path rejection and IF/CUSTOMFILE/CONTINUE handling stay in their existing
owners. The general LR2 file helpers and CSV/OLRskin serialization are unchanged.
Only one resolved filename is chosen and shared by Preview loading and the combo.

Synthetic tests cover the old `ERROR` failure, exact/wildcard DXA members,
include-relative paths, missing members and unpacked exact/wildcard fonts.
The read-only integration command below loads a real skin, then opens every
registered font through the same action used by the combo:

```powershell
SkinEditor_DX9/Release/SkinEditor_DX9.exe --check-preview-image-fonts "path/to/skin.lr2skin"
```

LR2 STANDARD SELECT (5 fonts) and 7-key PLAY (3 fonts) passed this check. It does
not save the fonts or scripts. Interactive mouse selection remains manual QA.
