# SkinEditor UI architecture and debugging

Project-wide feature status, file-format rules and scenario coverage are in
[`PROJECT_STATE.md`](PROJECT_STATE.md). Build and manual regression procedures
are in [`BUILD_AND_TEST.md`](BUILD_AND_TEST.md).

This document describes the presentation layer introduced for the modern
SkinEditor UI. Its first rule is that a visual refactor must not own or copy
editor state. `WORKSPACE` and the existing skin/object models remain the source
of truth.

## Layers

```text
main.cpp
  ImGui/DX9 lifecycle, fonts, application menu
       |
       v
seUI.h / seUI.cpp
  palette, spacing, reusable stateless components
       |
       v
uiCatalog.h
  window names, intent, owner function and default dock contract
       |
       v
WORKSPACE::draw()
  menus, quick-action toolbar, docking, tool visibility
       |
       +--> Preview / Image Manager / DST View
       +--> Object Browser / Object Inspector / History
       +--> Customize / Option List / other existing tools
       |
       v
skin model, CSV rows, History and selection synchronization
```

`seUI` is deliberately stateless. A component may return a click or edited
value, but it must not load skins, mutate CSV, select objects, or push History.
Those actions belong to `WORKSPACE` or the existing domain helpers.

## Design system

`SEUI::ApplyModernTheme()` is called once after ImGui context creation. It owns:

- application colors and contrast;
- spacing, padding, corner radius and borders;
- tab, table, docking and selection appearance;
- DPI scaling of fixed UI metrics.

Fonts are assembled in `main.cpp`: Segoe UI is the base face and Meiryo/Malgun
glyphs are merged for Japanese and Korean metadata. Font/backend/device setup
stays outside `seUI` because it is part of application lifetime management.

Reusable components currently include:

- `BeginToolbar()` / `EndToolbar()` for a consistent action surface;
- `BeginStatusBar()` / `EndStatusBar()` for document state and workspace metadata;
- `ActionButton()` for enabled/disabled commands and tooltips;
- `StatusPill()` for short state indicators;
- `SectionHeader()` for forms and property groups;
- `EmptyState()` for a useful no-document screen;
- `HelpMarker()` for local explanations without permanent visual noise.

## Adding a tool window

1. Add the window visibility flag and draw method to `WORKSPACE`.
2. Add one `SEUIWindowSpec` entry to `uiCatalog.h` and use
   `FormatSEUIWindowTitle()` for the runtime title.
3. Call the draw method in the subwindow dispatch section near the bottom of
   `WORKSPACE::draw()`.
4. Add its visibility pointer to `WindowToggle windowToggles[]`. The visible
   menu label and group must come from the catalog instead of a second string.
5. Register the catalog-derived title in the matching `DockBuilderDockWindow`
   tab group even when the window is hidden by default.
6. Keep selection and mutations in `WORKSPACE`; use `seUI` only for rendering.
7. Run `scripts\ui-map.ps1 -Check` and add a short state-flow/debugging note if
   the tool introduces shared
   selection, History, preview texture, or file ownership.

The visible label may change, but the `##` suffix is the stable ImGui identity.
Every per-workspace window must include `num`; otherwise two workspaces can
share focus, scroll and docking state accidentally.

## Default docking layout

The default workspace is intentionally asymmetric and follows this structure:

```text
Object Browser | Object Inspector | Preview / Image Manager / DST View / Text Editor | Option List
               |                  |--------------------------------------------------|------------
               |                  | Asset Browser / File Manager / History           | Customize / Timer Control
```

Object Browser and Object Inspector are separate full-height columns. The wide
center keeps the canvas above assets, and the narrow right column keeps option
data above customization and timer controls. Legacy and developer windows are
also assigned to one of these six tab groups, so `Layout > Show all windows`
stays organized instead of creating floating panels. `Layout > Balanced
workspace` restores default visibility; `Rebuild current docking` preserves
visibility and only repairs placement.

Browser and Inspector use the full workspace height. ImageManager and dstView
share Preview's tab node in the center. Asset Browser occupies the lower center
node so Preview remains visible while an asset is dragged upward. The right
column is split into OpList above and a Customize/Timer Control tab node below.
Timer manipulation is kept out of the Preview canvas. Keep this hierarchy when
adjusting split ratios. `Layout > Rebuild current docking` rebuilds this exact
layout through `DockBuilder`.

## Adding a common action

Keep the complete command available in its existing menu. Add a toolbar button
only as a shortcut:

```cpp
if (SEUI::ActionButton("Action", "What it does", canRun))
    ExistingWorkspaceMethod();
```

Do not duplicate the operation inside the button handler. Menu, shortcut and
toolbar must call the same domain method so save paths, History and selection
updates cannot diverge.

## State flows that must remain intact

### Object selection

```text
Preview / right-click / DST View / Text Editor Object row
  -> workspace ObjectSelection (`$SE_OBJECT_ID` 중심 key)
  -> current model index와 selection request를 계산
  -> Object Browser submits and scrolls to the target row
  -> Object Inspector reads the same selected model index
  -> active Object의 SRC crop이 shared Image Manager selection으로 동기화
  -> scroll request is consumed
  -> Preview highlight reads the same selected object
```

Object Browser and Object Inspector are independent dockable windows, but only
their visibility is independent. They must not acquire separate selection
variables. The legacy `wObjectEditor` flag is a compatibility request that
opens both panes; `wObjectBrowser` and `wObjectInspector` then control their
visibility.

A left click on an Object command row in Text Editor resolves that source row
through `SEFindObjectForRow()` and enters the same workspace selection flow.
The Browser and Inspector open, filters/search are cleared and the Browser
scrolls to the Object. Control-flow, comment and other rows that do not belong
to an Object leave the current selection unchanged.

After New creates and loads a PLAY preset, it resolves `preset_note_0` from the
rebuilt Object Model and enters this same selection flow. The preset generator
does not retain a model index across `LoadSkin()`; the persistent editor ID is
what makes the required first Note visible in Browser and Inspector.

`SEObjectEditorModel`, search/filter popup state, active-branch cache and tree
open state are all owned by the corresponding `WORKSPACE`. Do not introduce a
global Object model or function-local `static` state for a per-workspace pane.
Model indices may change after any CSV rebuild; store `$SE_OBJECT_ID` plus the
legacy group/anchor-row fallback in `SEObjectSelectionState`, then resolve the
current indices through `RestoreObjectSelection()`.

`SetObjectSelection()`과 `RestoreObjectSelection()`은 active Object의 첫 SRC
command를 `GetCommandHelp()` schema로 해석해 동일 `arr_IMG` crop을 선택한다.
Image Manager를 강제로 열지는 않지만, 이미 열려 있거나 나중에 열면 해당 crop으로
스크롤하고 atlas 위의 주황색 점멸 사각형을 표시한다. 같은 좌표가 여러 Branch에
있으면 Object의 `ifgroup`과 일치하는 crop을 우선한다. 점멸 주기는 LR2 scene
timer가 아니라 ImGui UI clock을 사용하므로 Preview timer가 정지해도 표시된다.
Atlas hover도 마지막 ImGui item 상태를 재사용하지 않는다. Canvas screen 좌표와
mouse 좌표를 직접 비교하고, `w/h == -1` crop은 texture 끝까지의 실제 크기로
해석해 가장 작은 겹침 crop에 파란 점멸 사각형과 tooltip을 표시한다. Crop의
`ifgroup` 숫자만 비교하지 않고 `ResolveIMGTextureIndex()`가 현재 canvas texture를
가리키는지 확인하므로, 다른 Branch에 속해도 실제 같은 texture를 쓰는 crop은
hover할 수 있다. Image Manager 왼쪽 목록의 미선택 crop row hover도 같은 임시
파란 표시를 사용한다.
Object Browser에서 선택되지 않은 Object row에 mouse를 올리면 selection을
바꾸지 않고 `imageManagerHoveredAssetIndex/frame` presentation state만 갱신한다.
Image Manager가 Object Browser보다 먼저 그려지는 현재 dock 순서에서는 직전
frame의 hover까지 허용하며, 같은 texture의 SRC crop을 파란 점멸 사각형으로
표시한다.

The Object Browser `Active objects only` filter uses the same predicate as its
green active rows: the object has at least one DST command, all documented DST
`$op` fields are enabled, and the owning IF/ELSEIF/ELSE branch is the currently
active sibling branch. A selection arriving from Preview clears this filter
along with Type/Group/Search so the requested object can always be submitted
and scrolled into view.

### Semantic destination editing

```text
Object Inspector Layout / Timeline / Conditions
  -> resolve selected SEObjectInstance rows
  -> resolve field columns through GetCommandHelp()
  -> EditValue / InsertLine / DeleteLine
  -> shared CSV + History
  -> Object model and Preview derived caches
  -> Save OLRskin projects the same rows into skin.json.objects
```

Layout, Timeline and Conditions currently select the first destination command
family in the Object that exposes the complete semantic contract. Timeline is
an ordered view over every matching row in that family and owns frame time,
transform and alpha; Conditions reads timer/loop/op1..3 from its frame 0 and
evaluates OP terms with the same `GetOptionFlag_dst()` used by Preview. The
Inspector does not yet expose a nested part or destination-family selector.
Other command families remain editable through `Advanced LR2`, even though the
V0.9 package projection serializes every supported source-bound destination
run. These tabs never retain their own Object, frame or condition copies; ImGui
values are rebuilt from Workspace rows on each draw.

Preview movement translates every selected destination frame. The white
bottom-right handle is intentionally single-selection only and changes only the
first destination's width/height (or text size), matching V0.5 static Layout
authority. All mutations use the existing History entry points.

OLR V0.9 export translates expanded SRC and DST rows through the package source
map, derives source-bound parts, and writes each consecutive supported
destination-command run separately. Import validates the exact source and
destination commands and schemas before compiling; V0.7 flat Objects continue
through their legacy authority. Layout and Timeline frame 0 must agree, and
unknown/custom OPs remain raw numeric terms rather than being guessed. V0.9
does not infer 1P/2P/DP variants or link alternate destination families or
IF/ELSE branches. Compile assigns a supported numeric field only when its parsed
LR2 value differs, preserving empty zero tokens and legacy text spellings during
an unchanged round trip.

### Image asset selection

```text
Asset Browser card
  -> shared IMG index (`src_selected`)
  -> `ResolveIMGTextureIndex` resolves logical gr + IF/custom-file texture
  -> Image Manager receives a one-shot scroll request

Asset Browser `Use in selected Object`
  -> resolve active Object through `SEObjectSelectionKey`
  -> collect schema-compatible SRC rows (`gr/x/y/w/h`)
  -> one row: apply directly / multiple rows: choose in modal
  -> one full-row `EditLine()` History entry
  -> derived model, Preview and shared image selection rebuild
```

Asset Browser is a thumbnail view over the existing `arr_IMG` crop model; it
does not own copied crop records or a second selection. A single click updates
the shared Image Manager selection, and a double click activates Image Manager
at the corresponding atlas position. Cards expose the stable
`SKINEDITOR_IMG_ASSET` drag/drop payload containing one `int` IMG index so the
Preview drop target can consume assets without depending on Asset Browser
rendering code.

`BuildImageAssetUsage()` derives reverse usage from `IMG::sourceDeclare` or
`editorDeclare` to the rows held by each `SEObjectInstance`; legacy rows use the
shared schema/crop resolver only as a fallback. Asset Browser cards and Image
Manager crop rows display the number of distinct Objects using each crop.
The result is cached until Object Model generation or `arr_IMG.count` changes,
so large skins do not rescan every Object separately in both windows each frame.
`Unused only` filters this derived view without changing CSV/model state. The
Asset context menu lists up to 32 users and routes a chosen model index through
`SetObjectSelection()`, so Browser focus, Inspector, Preview and Image Manager
remain synchronized. Texture candidate labels aggregate distinct Object users
by logical gr rather than claiming ownership of a branch-dependent file.

`ApplyImageAssetToObjectSource()` is the only path that replaces an existing
Object SRC from an Asset. It validates that the target row still belongs to the
resolved Object, writes `gr/x/y/w/h`, optionally writes
`div_x/div_y/cycle/timer`, then submits the complete CSV row through
`EditLine()`. Command-specific fields are never reinitialized. The Asset Browser
button/context menu and Object Inspector Tagged image ComboBox both call this
method, preventing five separate History entries for one crop change. A modal
keeps only `SEObjectSelectionKey` while open and resolves the current model
index again every frame; it does not retain an Object vector index as identity.

Each `IMG` records the declaring SRC row in `sourceDeclare`. `Animate SRC`
resolves that row back to `arr_SRC` and previews its `div_x`, `div_y` and
`cycle`; coordinate-only matching is only a compatibility fallback. This is
required because duplicate SRC rectangles can intentionally use different
animation grids. Animation changes UVs only—the card selection and drag/drop
payload remain the original full crop.

Dragging `SKINEDITOR_IMG_ASSET` onto Preview does not edit the CSV immediately.
Preview converts the mouse position through its current zoom/scroll transform,
shows a translucent crop ghost for one `div_x/div_y` animation cell, then opens
the existing New Object form with the source division/cycle/timer preserved and
`#SRC_IMAGE`, crop coordinates, destination position and current Object branch
pre-filled. The normal OK path remains responsible for CSV insertion, model
rebuild, Preview invalidation and History.

Image Manager owns the simple `Pixel paint` mode. Canvas mouse coordinates are
converted through `ImageManagerZoom` to one source-texture pixel. Left drag
paints the selected RGBA color, right drag writes transparent pixels, and middle
click samples a color. A Bresenham segment fills gaps between mouse frames while
the brush remains exactly one pixel wide.

The `Folder` action beside `grReload` opens the current resolved `SRCGR::path`
in Windows Explorer. Existing files are opened with `/select`; an unavailable
wildcard candidate falls back to its nearest existing parent directory. The
CP932 path is converted to UTF-16 before calling the Windows shell.

Painting first updates every loaded `SRCGR` texture that shares the same source
path. `SaveTextureToImageFileAtomic()` encodes the original file type to a
temporary file, creates a one-time `.skineditor-pixel.bak`, then replaces the
source. Revert releases same-path editor textures and reloads disk state. Do not
route bitmap edits through CSV History: they modify an external image file, not
`skinfileLines`.

`New image` and `Merge image` are Image Manager modal workflows. New image
creates a solid RGBA canvas. Merge uses the currently displayed full `SRCGR` as
the base and an `arr_IMG` crop as the overlay. A one-pass vertical-run scan finds
the first fully transparent rectangle large enough for the crop. If none exists,
the packer compares right and bottom growth and chooses the smaller final canvas.
There are no manual placement coordinates or expansion toggle. The image file is
written atomically and never replaces an existing path.

CSV registration is intentionally separate from bitmap creation. On success,
`RegisterGeneratedImage()` appends `#IMAGE` and one full-size `$SRC_IMAGE`
editor Asset immediately before the root owner's `$FILE ... end`. It computes
the next logical gr using the same sibling-branch maximum rule as
`ParseSkinGraphics()`; inserting next to the current declaration is forbidden
because it would shift later gr IDs. A pending gr request survives the derived
cache rebuild, resolves to the new IMG index, and synchronizes Image Manager and
Asset Browser selection on the next frame.

`ReplaceImageDeclarationPath()` changes only the declaring `#IMAGE` row through
one `EditLine()` call. The confirmation modal compares old/new dimensions and
counts affected/out-of-bounds crops, but never rewrites crop coordinates. It
stores the declaration document row across the deferred cache rebuild rather
than retaining a `SRCGR` array index.

`grReload` queues the resolved CP932 path and releases/recreates matching editor
textures at the next frame boundary, before any window submits image draw
commands. It is blocked while that path has unsaved Pixel paint. `Usage` only
opens the derived Image status panel and does not mutate CSV state.

`RegisterImageAssetGrid()` partitions the selected IMG rectangle with integer
boundary ratios, skips duplicate branch/crop keys and inserts named
`$SRC_IMAGE` rows beside the owning `#IMAGE`. The grid modal owns only temporary
columns/rows/cell-selection presentation state. Every primitive insert/edit
keeps the normal CSV notification path; a trailing `HISTORYOP::group` marker
makes the whole batch one Ctrl+Z action. After rebuild, the first inserted
metadata row resolves back to the shared IMG selection without retaining a
transient asset index.

`BuildImageDiagnostics()` derives Image Manager status directly from
`arr_SRCGR`, `arr_IMG`, `ImageAssetUsage()` and Object SRC rows. It reports
missing/unloadable files, bounds and duplicate crops, unused editor Assets and
SRC rows without an Asset. Diagnostic navigation routes through
`SelectIMGAsset()` or `SetObjectSelection()`; the panel does not own another
selection or repair CSV automatically.

The insertion owner is explicit: a selected Object/branch keeps its source
filename, while an unscoped drop uses the root skin. Rows are clamped before
that owner's in-memory `$FILE ... end` marker so the Preview runtime mask and
Save path both recognize them. After rebuild, the generated `$SE_OBJECT_ID`
drives a one-shot Object Browser selection request; this clears filters/search,
scrolls to the new row and shows the same Object in Inspector.

### Simple Mode

`WORKSPACE::drawSimpleMode()` projects authoritative LR2 `#SRC_*` rows into five
authoring groups: number fonts (including NOWCOMBO), judgement fonts, gear parts,
notes and gauge sources. Object Model names and ids enrich labels and selection when present,
but they are not a discovery gate; legacy skins and newer note-family commands
remain visible even when an older `skinObjGroup.txt` does not know them. Each
card resolves back to one authoritative source row and displays compatible
`arr_IMG` crops already used by the same group. UI selection stores the Object
id when available, otherwise a source-row fallback, plus the source command and
ordinal. The semantic projection is a per-`WORKSPACE` derived cache because
building it scans all source rows, Object ownership and graphic contexts. An
unchanged Simple Mode frame reuses that projection; `NotifyDocumentChanged()`,
Object-model rebuild and skin reload invalidate it. Selection still resolves
its stable slot id against the current projection rather than retaining a row
pointer across rebuilds.

Applying art changes only the shared atlas fields (`gr`, crop rectangle,
division grid and cycle). NUMBER ids, note lane ids, timers, ops, DST placement,
conditions and owner files remain intact. Apply scope is explicit: one component,
same white/black/scratch note family or 1P/2P font pair, exact source command, or
the complete category. The note-family scope also requires the exact part command,
so normal notes do not silently replace mines or long-note parts.

`Import image...` copies the selected bitmap into `simple-assets/`, appends a
new trailing `#IMAGE` plus `$SRC_IMAGE`, and points the chosen semantic slot at
that graphic. By default it inherits the target `div_x/div_y/cycle`; dimensions
that do not divide evenly by the atlas grid are rejected before the copy. Asset
choices can be filtered to the same grid. Hue/Saturation/Brightness creates a
cropped PNG in `simple-assets` without modifying the source texture and applies
it through the same scope. Before/After previews and a local Undo button use the
normal Workspace History path. The document mutation is one snapshot-backed History action;
Undo restores all touched rows even when the import changed the line count.
The copied file intentionally remains as an unused local asset after Undo.
Preview and both asset windows continue to use their existing rebuild path.

For Object Browser labels, naming priority is explicit `$SE_OBJECT_NAME`, then
the command-specific symbolic SRC value (NUMBER, SLIDER, BUTTON, BARGRAPH or
TEXT), the first distinct `$op` names, and finally the first distinct non-zero
`$timer` names. `MainTimer` (`0`) is skipped because it appears on most objects
and does not provide a useful fallback identity. Automatic names are
presentation only; they never write metadata back to the skin file.

Do not store a second selected object in a UI component. When selection looks
correct in one panel but not another, log `selected_obj`, the object ID, source
row and pending scroll request at each arrow above.

### Editing and undo

```text
property edit or preview drag
  -> snapshot/history entry
  -> CSV/model mutation
  -> NotifyDocumentChanged(change kind)
  -> editor cache / Object model / Preview rebuild as required
  -> Ctrl+Z or toolbar Undo calls WORKSPACE::UndoLastEdit()
```

The toolbar does not maintain its own history. If the rectangle and object
position separate after Undo, inspect the rebuild/invalidation stage rather
than the button.

### PLAY preview simulation

```text
Timer Control > Restart scene
  -> LoadSceneSE rebuilds the current skin runtime objects
  -> LR2SESceneInit builds silent LaneStruct / NoteStruct chart data
  -> LR2SESceneProc calls LR2's original ProcI_Play
  -> DrawNotes moves notes and ApplyJudgeNote starts lane effect timers
  -> LR2 skin objects consume note, key-beam, explosion and judge/combo timers
  -> LR2SEDrawLoop captures them into the Preview texture
```

The simulator must remain independent of external LR2 sample BMS and keyconfig
files, because packaged/editor-only environments do not contain them. Playback
state belongs to `WORKSPACE`, never to a function-local static, so two open
skins cannot start or stop one another. A preview rebuild while running must
reinitialize the scene before the next draw-buffer pass.


An inactive Preview dock tab returns false from `ImGui::Begin`, but that is
only a presentation result and must not pause its Workspace scene.
`drawPreview()` calls `WORKSPACE::UpdatePreviewRuntime()` for a running scene
before taking the inactive-tab early return. Keep the runtime on the UI thread
and consume each Workspace draw buffer once per tick;
do not add a `ProcGameThread` for apparent parallelism because LR2/DxLib render
state is process-wide and the application frame already interleaves workspaces.

The LR2 preview core is also per Workspace. `WORKSPACE::lr2CoreInitialized`
tracks whether that Workspace's `game` has initialized its song list, object
strings, gameplay buffers and critical sections. A process-wide static flag is
invalid because the second Workspace owns distinct, otherwise uninitialized
`game` storage. Reloading the same Workspace reuses its initialized core.

The editor may synthesize chart data, but it must not synthesize note screen
coordinates, judgement state, combo values or effect timers. Those remain
owned by LR2's PLAY pipeline so a skin preview cannot silently diverge from
runtime behavior.

Object Browser reordering is a special structural edit. An Object may consist
of non-contiguous indexed rows, so it records one `SkinDocumentSnapshot`
instead of a chain of line moves. The drag payload carries a model index only
for the current ImGui frame; the queued operation stores stable selection keys,
then resolves and applies at the next frame boundary. Reorder and snapshot
Undo must complete before any texture-backed window submits draw commands.
Cross-file and cross-Branch drops are rejected.

### Files

New, Open, Save, Save As and Export keep their domain methods. File menu,
toolbar and Ctrl+S all call `SaveCurrentSkin()`; the bottom status bar derives
`SAVED/MODIFIED/SAVE FAILED` from workspace revision state. Save As uses the
native file picker and must continue switching the active working path after
success. The loaded-workspace resolution modal is the deliberate exception to
ordinary undoable editing: after a clean-state guard it atomically changes the
root `#INFORMATION` resolution, neutralizes active `#RESOLUTION` rows as
`$OLR_IGNORED_RESOLUTION`, and reloads the workspace. This avoids the legacy LR2
skin-list parser's next-slot write while keeping physical row addresses stable.
Panel components must not duplicate any of these file operations.

`skinResolution.cpp` owns the loaded canvas resolution decision. `LoadSkin()`
passes it the fully expanded script after includes have been read, and applies
the result before LR2 graph handles are created. The precedence is valid
`#INFORMATION`, `#RESOLUTION`, robust `#DST_*` bounds, then 640x480. DST field
positions come from `skinHelper.txt` through `GetCommandHelp()` rather than a
second hard-coded schema. An inferred result is workspace state only; the UI
may display it but must not persist it until the user explicitly applies a
resolution through the existing modal.

Skin Browser uses the native folder picker for `Open another location`, then
`SEScanSkinFolder()` discovers `.lr2skin` and `.lr2ss` recursively before the
existing `ParseLR2SkinCustom()`/`LoadSkin()` path takes over. Refresh reuses the
current location; Default locations returns to `LR2files/Theme` and
`LR2files/Sound`. The scan must skip reparse-point directories and reject
unrepresentable or over-`MAX_PATH` paths because the legacy loader is ANSI and
fixed-path based.

`WORKSPACE::LoadSkin()` owns the reload boundary. It first destroys nested
editor-owned CSTR/CSV/Object/History values, recreates every derived array, then
releases LR2 graph and image-font handles before changing DxLib graph mode.
Parser passes may only populate the freshly reset containers. Resource paths
already resolved to an absolute Windows byte path are idempotent: they must not
be reconstructed through `std::filesystem`, because CP932 skin filenames can be
invalid in the host ANSI code page. D3DX image inspection/loading receives file
bytes opened by Win32 rather than interpreting the legacy filename itself.

The LR2 preview core is also per workspace. `WORKSPACE::lr2CoreInitialized`
tracks whether that workspace's `game` has initialized its song list, text
storage, gameplay buffers and critical sections. A function-local or
process-wide static flag is invalid here: it would skip initialization for the
second workspace while its `game` storage is still empty. Same-workspace reload
reuses the initialized core; a newly created workspace initializes its own.

OLR uses three explicit `WORKSPACE` file flows. `ExportOlrSkin()` is the
low-level projection boundary: it reads the same expanded CSV rows and
per-workspace Object Model used by the editor, then hands an
`SEOLRSkinDocument` to `olrSkin.cpp`; it does not create another editable model.
`SaveOlrSkin()` owns the `File > Save OLRskin` policy. A normal LR2 workspace is
packaged without changing its source files. An imported OLR workspace saves a
dirty script first, then atomically replaces the package so the later LR2
folder export observes the same edit. Unsaved Image Manager pixel edits block
the command because bundled assets are read from disk. Neither operation
changes `mainpath`.

`ImportOlrSkinInteractive()` validates the complete archive before creating a
directory. V0.8+ then validates every nested part source binding and destination
row, the Layout/frame-0 invariant, semantic or raw condition, and every V0.4
`skin.json.simple_mode` row. V0.7 flat Objects retain their existing authority
and validation path. Import atomically compiles the declared authorities into
the extracted compatibility script; a mismatch removes the new directory. It reparses the resulting
`#INFORMATION`, and enters the normal `LoadSkin()` path. It never follows owner
labels from `compatibility/source-map.json` and never overwrites an existing
folder. The result popup is presentation state only; the loaded document and
selection continue to be owned by `WORKSPACE`.

`ExportLr2SkinInteractive()` is the third user-facing flow. It accepts only an
imported V0.2+ virtual workspace, rejects dirty script or pixel state, and
materializes a new non-existing install-ready LR2 tree without writing into a
user's LR2 installation. For V0.9 it keeps the copied original include-based
main only while the hidden marker and byte-identical compatibility baseline are
valid, the original main exists and no fixed asset relocation is required.
Otherwise it writes the current compatibility script, so semantic or raw edits
cannot be lost. Before that fallback is written, `$OLR_FILE start/end` markers
become generated CSV `#INCLUDE` files beside the exported main. Their directives
use full `LR2files\...` paths because LR2 ignores the declaring CSV directory
when opening an include. This restores
LR2's fresh per-include IF stack instead of relying on its broken nested-IF
gating: a child `#IF` inside an inactive/right parent can no longer reactivate
and replace lane 0's `#DST_NOTE`. Orphan and unclosed child controls remain
contained by the generated file boundary just like the original include graph.
Both paths resolve an otherwise unknown OLR canvas to HD
1280x720, persist the selected canvas in `#INFORMATION` fields 6/7 and leave no
active `#RESOLUTION` command in the materialized main. The materializer accepts
only LR2-discoverable mains below `LR2files/Theme|Sound`, rewrites virtual path
aliases by CSV command field across the selected main/include graph, and roots
process-relative image/font/include/thumbnail/custom/help declarations to that
graph's logical LR2 location. Its result popup reports the rewrite count and any reason an
original-main attempt fell back to the compatibility script; `INSTALL.txt`
records how to merge the output beside `LR2.exe`.

After a successful import, the source `.olrskin` path is retained only as local
Workspace state to suggest the next Save OLRskin destination. The association
is cleared at the next document-load boundary and is never serialized into the
portable package or materialized LR2 tree.

## Debugging checklist

When a UI change causes a regression, check in this order:

1. **ImGui ID collision** — run with the ImGui debug tool and inspect duplicate
   labels lacking a `##workspace` or pushed object ID.
2. **Begin/End balance** — every `Begin`, `BeginChild`, table, tab, popup, style
   push and disabled block must be paired on every early-return path.
3. **Cursor extension assertions** — after `SetCursorPos` or
   `SetCursorScreenPos`, submit a real item (`Dummy`, text, button, child) that
   grows the parent bounds.
4. **Wrong owner** — verify the component only reports intent and does not keep
   a copy of model/selection/history state.
5. **Dock title mismatch** — the title passed to `DockBuilderDockWindow` must
   exactly match the title passed to `ImGui::Begin`.
6. **Stale derived data** — after CSV mutation or Undo, confirm preview objects,
   Image Manager sources and Object Editor rows were rebuilt together.
7. **DPI/font issue** — reproduce at 100% and the target monitor scale; inspect
   missing glyphs separately from layout sizes.

## Visual regression pass

Before merging a UI change, manually verify:

- no skin loaded: New/Open empty state and both buttons;
- PLAY, SELECT, DECIDE and RESULT workspaces at 640x480 and HD;
- Preview, Image Manager and DST View zoom/scroll behavior;
- Preview right-click selection and Object Editor auto-scroll;
- add/remove DST and continued selection of the same object;
- Ctrl+Z after property editing and preview dragging;
- New Object fields, tagged-image thumbnail and `$` option combo boxes;
- Text Edit mode and Shift-JIS text;
- Save As working-path switch and existing-file protection;
- `Layout > Show all windows`, then reopen every grouped Windows menu entry.

The Release x86 build remains the final compile gate. Existing compiler
warnings should be tracked separately; a UI-only change must introduce no new
errors or warnings.
