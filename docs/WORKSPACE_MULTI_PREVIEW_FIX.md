# Multi-Workspace Scene Preview bug fix

이 문서는 `AI_1`처럼 기능 구성이 다른 브랜치에서 OLR/Simple Mode 변경을 함께
가져오지 않고, 여러 Workspace의 Scene Simulator가 동시에 진행되지 않는 문제만
이식하기 위한 안내서다.

## 사용자에게 보이던 문제

1. Workspace를 두 개 이상 연다.
2. 각 Workspace에서 PLAY 스킨을 열고 `Timer Control > Restart scene`을 누른다.
3. 다른 Workspace 또는 다른 dock tab을 활성화한다.
4. 비활성 Preview의 노트, 판정 효과와 timer 41이 멈춘다.

Scene 상태와 `game` 구조체는 이미 Workspace별이었지만, scene tick은
`WORKSPACE::drawPreview()` 안에서 Preview 창이 표시될 때만 실행됐다.
ImGui의 비활성 dock tab은 `ImGui::Begin()`에서 `false`를 반환한다. 기존 코드는
즉시 `End()`/`return`했기 때문에 `LR2SESceneProcSafe()`와
`LR2SEDrawLoopSafe()`까지 건너뛰었다.

## 독립 커밋

`AI_2`의 최소 수정 커밋은 다음과 같다.

```text
ecd0e37 fix: keep workspace scene previews advancing
```

먼저 대상 브랜치에서 다음을 시도한다.

```powershell
git cherry-pick ecd0e37
```

이 커밋은 다음 파일만 바꾼다.

- `SkinEditor_DX9/winWorkspace.h`
- `SkinEditor_DX9/winWorkspace.cpp`
- `SkinEditor_DX9/winWorkspace2.cpp`
- `docs/BUILD_AND_TEST.md`
- `docs/PROJECT_STATE.md`
- `docs/UI_ARCHITECTURE.md`

`AI_1`의 Preview 코드가 달라 충돌하면 cherry-pick을 억지로 완료하지 말고 아래
최소 변경을 현재 구조에 맞게 적용한다. `d0a58fe` 같은 앞선 OLR/편집 기능 기준
커밋 전체를 가져오는 것은 이 버그 수정에 필요하지 않다.

## 최소 구현 계약

### 1. ImGui 없는 runtime 함수로 분리

`WORKSPACE`에 다음과 같은 함수를 둔다.

```cpp
bool UpdatePreviewRuntime(unsigned long long previewNow);
```

기존 `drawPreview()`에서 다음 책임을 이 함수로 옮긴다.

- 지연된 Preview rebuild와 실행 중 scene 재초기화
- 16 ms frame interval 판정
- `LR2SESceneProcSafe()`를 통한 Workspace별 `ProcGame`/`ProcI_Play`
- `LR2SEDrawLoopSafe()`를 통한 draw-buffer 소비와 soft-image capture
- `previewLastRenderAt`, `previewTextureDirty` 갱신

이 함수는 ImGui 창 표시 여부나 현재 활성 tab을 읽지 않는다. 입력은 현재 tick이고,
상태 원본은 호출한 `WORKSPACE`의 `g`, Preview 설정과 render target이다.

### 2. 비활성 tab에서도 실행 중 scene 진행

`drawPreview()`는 `ImGui::Begin()` 결과를 presentation 상태로만 취급한다.

```cpp
const bool previewWindowVisible = ImGui::Begin(...);
const bool previewFrameUpdated = previewWindowVisible || previewSimulationPlaying
    ? UpdatePreviewRuntime(GetTickCount64()) : false;
if (!previewWindowVisible) {
    ImGui::End();
    return 0;
}
```

실행 중 simulator는 비활성 tab에서도 runtime update와 draw-buffer 소비를 계속한다.
scene이 실행 중이 아닌 비활성 Preview까지 매 frame 다시 그릴 필요는 없다.

### 3. 지켜야 하는 불변식

- `previewSimulationPlaying`, chart mode, timer와 `game`은 Workspace별 상태다.
- `ProcGame`은 각 Workspace tick에서 정확히 한 번만 실행한다.
- scene processing 뒤 같은 Workspace의 draw buffer를 그 frame에 소비한다.
  처리만 반복하고 draw를 생략하면 buffer가 누적된다.
- LR2/DxLib render state는 process-wide이므로 별도 `ProcGameThread`를 만들지 않는다.
  UI thread가 Workspace를 순서대로 interleave하는 것이 여기서 말하는 병렬 동작이다.
- CSV parser, LR2 명령 해석, skin source text는 이 수정 범위가 아니다.
- 한 Workspace의 restart, chart mode 변경 또는 Preview rebuild가 다른 Workspace의
  scene을 초기화하면 안 된다.

## 회귀 테스트

`RunWorkspaceRuntimeMultiWorkspaceSmokeTest()`는 단순히 두 Preview를 한 번 그리는
검사로는 부족하다. 두 scene을 각각 `LR2SESceneInitSafe()`로 시작한 뒤 최소 네 frame
동안 다음 순서를 반복한다.

```text
Workspace A UpdatePreviewRuntime
Workspace B UpdatePreviewRuntime
```

각 호출이 frame을 만들고, 두 `previewSimulationPlaying`이 유지되며, 양쪽 timer 41이
시작값보다 증가해야 통과시킨다. 실제 스킨이 있는 개발 환경에서는 다음 smoke를
추가로 실행한다.

```powershell
$env:SKINEDITOR_RELOAD_FIRST = 'D:\skins\first\play_7.lr2skin'
$env:SKINEDITOR_RELOAD_SECOND = 'D:\skins\second\play_single.lr2skin'
$test = Start-Process .\SkinEditor_DX9\Release\SkinEditor_DX9.exe `
  -ArgumentList '--skin-multi-workspace-smoke' -Wait -PassThru
$test.ExitCode
```

수동 검증에서는 두 Workspace의 Scene을 시작하고 한쪽 Preview를 비활성 dock tab으로
둔 채 5초 이상 기다린다. 다시 열었을 때 chart가 그동안 진행된 위치에 있고,
양쪽 Timer Control의 timer 41이 계속 증가해야 한다. note, key beam, explosion,
judge/combo도 각 스킨의 LR2 flow를 유지하는지 확인한다.

마지막으로 저장소 표준 검증을 실행한다.

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\ui-map.ps1 -Check
powershell -ExecutionPolicy Bypass -File .\scripts\docs-check.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\test.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\ai-context.ps1 -Check
```

## 롤백

이 변경에는 파일 포맷이나 데이터 migration이 없다. 문제가 생기면 대상 브랜치에서
해당 cherry-pick commit을 revert하고, `drawPreview()`의 기존 단일 Preview 경로로
돌아갈 수 있다. 사용자 스킨 파일은 수정하지 않는다.
