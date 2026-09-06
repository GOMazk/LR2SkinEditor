# Simple Mode 선곡 화면 편집

SELECT 스킨을 불러오면 Simple Mode의 **선곡 화면 배치·효과**에서 화면 요소를
묶어 편집한다. 기존 이미지·글꼴 교체 화면은 상단 전환 버튼으로 계속 사용할 수 있다.

## 사용 흐름

1. 선곡 스킨을 열고 Simple Mode에서 대상 묶음을 선택한다.
2. `적용 대상 확인`을 펼쳐 포함된 요소를 확인한다. 필요하면 `선택 Object와 같은
   조건 분기만`을 켠다. 이 옵션은 Object Browser의 현재 선택을 기준으로 한다.
3. 자동 분류 대신 원하는 조합을 편집하려면 Object Browser/Preview에서 여러
   Object를 선택하고 `선택한 Object 묶음`을 고른다.
4. 배치에서 이동량과 크기를 정하거나 색조·투명도를 조정해 묶음에 적용한다.
5. 애니메이션은 효과, 시작 시점, 지연, 재생 시간을 정하고 `효과 적용하고
   Preview에서 재생`을 누른다. 적용은 열린 문서에 대한 편집이며 아직 디스크에
   저장하지 않는다. `현재 효과 다시 재생`은 문서 변경 없이 시작 타이머만 재실행한다.
6. 결과가 좋으면 기존 Save/Ctrl+S로 저장한다. Undo 한 번으로 묶음 편집 전체를
   복구할 수 있고 Redo도 기존 Workspace History를 사용한다.

## 묶음과 동작

| 묶음 | 발견 기준 |
|---|---|
| 곡 목록 전체 | schema에 배치 필드가 있는 `#DST_BAR_*` |
| 선택된 곡 강조 | `#DST_BAR_BODY_ON`, `#DST_BAR_FLASH` |
| 곡 정보·숫자 | `#DST_TEXT`, `#DST_NUMBER` |
| 난이도·그래프 | `#DST_BARGRAPH` |
| 버튼·슬라이더 | `#DST_BUTTON`, `#DST_SLIDER`, `#DST_ONMOUSE` |
| 배경·장식 | `#DST_IMAGE`, `#DST_BGA` |
| 선택한 Object 묶음 | 기존 공유 Object 선택에 포함된, 지원 DST를 가진 요소 |

자동 분류는 명령의 역할을 기준으로 한다. 그림의 의미를 추측해 분류하지 않는다.
특정 스킨에서 다른 용도로 사용한 그림·숫자는 Object 선택으로 적용 범위를 좁힌다.
구형 Object schema에 빠진 BAR_TITLE도 CSV에서 직접 발견하므로 자동 목록 묶음에
포함된다. 위치·크기 schema가 없는 미지원 DST는 포함하지 않는다.

이동량은 스킨 좌표의 픽셀 단위이며 크기는 기존 간격과 애니메이션 경로를 함께
확대·축소한다. 곡 목록 제목·램프·강조는 막대 기준의 상대 좌표다. 막대와 함께
이동할 때에는 막대에만 이동량을 더해 이중 이동을 막고, 상대 좌표도 크기 비율에
맞춰 조정한다. 상대 요소만 선택하면 해당 요소의 상대 위치를 직접 조정한다.

색조와 불투명도는 각 기존 프레임에 곱해 기존 페이드·명암 차이를 유지한다.
흰색과 비율 1.0은 무변경이다. 원본 PNG 픽셀은 수정하지 않는다.

## 효과와 시작 시점

- 서서히 나타나기 / 사라지기: 지정 시간 동안 alpha를 보간한다.
- 이동하며 나타나기: 시작 위치 차이와 alpha를 함께 보간한다.
- 반복 점멸: 원래 alpha → 지정 최소 비율 → 원래 alpha를 반복한다.
- 시작 조건: 기존 타이머 유지, 화면 진입(0), 곡 변경(11), 화면 종료(2),
  패널 1 열림(21)/닫힘(31). 추가 조건은 기존 timer 선택기를 사용한다.

기존 애니메이션이 있으면 `기존 애니메이션 교체 허용`을 켜야 효과를 적용할 수
있다. 마지막 프레임의 자세를 기준으로 새 타임라인을 만들고 첫 프레임의 조건과
알 수 없는 확장 필드를 보존한다. 선택한 시작 타이머, 시간, loop, 보간은 효과에
맞춰 바뀐다. 애니메이션을 레이어로 중첩하는 기능은 아니다.

LR2의 BAR_BODY_ON/OFF는 scene loop가 timer 0으로 직접 평가한다. 따라서 이
막대를 포함한 묶음에는 다른 시작 이벤트를 허용하지 않는다. 제목·flash를 따로
선택하면 곡 변경 등의 이벤트를 사용할 수 있다. Preview는 편집 결과를 기존 LR2
runtime에 다시 읽힌 뒤 타이머를 재실행한다. 전체 선곡 조작을 자동으로 수행하는
것이 아니라 설정한 이벤트의 재생을 확인하는 기능이다.

## 소유권과 진입점

- `simpleSelection.h`: 저장하지 않는 묶음/편집 요청 타입과 파생 타임라인.
- `simpleSelection.cpp`: Workspace별 발견 캐시, 현재 공유 선택 해석, 사전 검증,
  배치·색·효과의 원자적 적용, 단일 snapshot Undo, Preview 타이머 요청.
- `winWorkspaceSimpleSelection.cpp`: 한국어/영어 편집 UI. 임시 설정만 보유하고
  문서 변경은 `WORKSPACE::ApplySimpleSelectionEdit()`에 맡긴다.
- `winWorkspace.cpp`: SELECT 스킨에서 Simple Mode 화면 전환과 캐시 무효화.
- `winWorkspacePreview.cpp`: runtime 재로딩 완료 후 요청한 효과 타이머 재실행.
- `simpleSelectionTests.cpp`: 명령별 회귀 검증과 LR2 실제 DST reader/interpolator.
- `uiCatalog.h`: `simple-selection` 흐름과 Simple Mode owner 계약.

저장 원본은 기존 CSV/WORKSPACE다. 기존 Simple Mode source projection과 OLRskin
0.9의 `simple_mode` 분류, package entry/field, compiler ownership은 변경하지
않는다. 편집된 기존 DST는 기존 LR2/OLR 저장 경로를 사용한다. include 소유권,
IF/ELSE 경계, SRC, 오브젝트 ID, 비대상 행을 유지한다. 파생 행 번호와 model index는
무효화 전까지만 유효하며 UI의 장기 선택에는 사용하지 않는다.

## 검증과 수동 확인

표준 명령은 `BUILD_AND_TEST.md`를 따른다. 집중 검증은 실행 파일의
`--self-test-simple-selection`이며 `scripts/test.ps1`에도 등록되어 있다.
회귀 테스트는 그룹 발견, BAR 상대 좌표, 크기/색 편집, 조건별 선택, 기존 애니메이션
교체 gate, 이벤트/loop/미지 필드 보존, 단일 Undo/Redo, 범위 오류의 무변경,
Workspace 분리와 LR2의 실제 페이드 보간을 확인한다.
추가로 편집 후 남은 재생 요청 취소, 한국어/영어 및 320/700픽셀 창의 실제
ImGui UI 제출과 가로 넘침도 검사한다. 이 검사는 네이티브 글꼴 렌더링이나
사용자 마우스 조작에 대한 시각 검증을 대신하지 않는다.

수동 확인:

- 기존 SELECT 스킨에서 각 자동 묶음과 Object 다중 선택 범위를 확인한다.
- 목록 전체를 이동/확대해 제목과 램프가 이중 이동하지 않는지 확인한다.
- 곡 정보에 곡 변경 페이드/슬라이드, flash만 선택해 반복 점멸을 적용하고 재생한다.
- 효과 적용 후 Preview가 비활성 탭이어도 다시 열었을 때 올바른 결과가 보이는지 확인한다.
- 좁은 도킹 영역과 한국어/영어 설정에서 각 버튼과 입력란을 확인한다.
- Save/Ctrl+S 후 다시 열기, 기존 OLR 저장/재가져오기와 실제 LR2 재생을 확인한다.

오류가 나면 메시지의 명령과 행, 선택 묶음, IF 범위, 타이머를 함께 확인한다.
기존 애니메이션 교체 허용 여부와 목록 막대의 timer 0 제한도 확인한다.
