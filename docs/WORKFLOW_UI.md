# Workflow UI

## 이미지부터 만들기

1. 스킨을 열거나 New 프리셋을 만든다.
2. Asset Browser > **Add... > Import image...**를 누른다. 기존 gr/와일드카드 확인과
   선택적 투명 여백 자동 인식을 그대로 사용한다.
3. 만들어진 Asset을 Preview로 드래그하고 Object 종류를 정한다.
4. Inspector에서 배치·조건·애니메이션을 편집한다. Source image는 해당 crop을,
   Appearance는 지원되는 숫자/판정/노트 등의 의미별 교체 도구를 연다.
5. 전문 도구의 **Back to Preview**로 배치 화면에 돌아온다.

Simple Mode의 Preview 이동은 상단 **Back to Preview** 하나로 통일했다.
창만 켜던 하단의 중복 **Open Preview** 버튼은 제거했다.

같은 도킹 영역에서 돌아올 때는 키보드 포커스도 Preview로 이동해 이전 탭으로
되돌아가지 않는다. 다른 영역의 Object Browser 등에서 요청한 탭 표시는 그 창의
입력/드래그 포커스를 가져오지 않는다.

## 배치부터 만들기

1. Asset Browser > **Add... > New layout Object...**에서 빈 배치를 만든다.
2. Preview의 Layout boxes에서 위치·크기를 조정한다.
3. **Add... > Images from layout...**에서 대상들을 골라 공유 또는 개별 이미지를 만든다.
4. 생성 PNG에 그림을 그리고 저장하면 외부 이미지 변경이 자동 재로드된다.

지원 유형/atlas/가이드 규칙은 기존과 같으며 이미지 생성과 저장은 별개다.
기능은 Windows 메뉴에서 계속 열 수 있다. Layout > Balanced workspace는 기본
작업 창을, Asset workspace/Show all windows는 전문 도구들을 다시 노출한다.
사용자가 조절한 dock 크기나 위치를 매 프레임 강제하지 않는다.

## FHD / 좁은 패널에서 작업하기

- Asset Browser 상단은 **Add... / Options... / 검색 / Use in Object / 개수**로 압축했다.
  공간이 부족할 때만 다음 줄로 이어지며, 기존 빈 공간 우클릭 생성 메뉴도 유지한다.
- 카드는 기본적으로 썸네일과 이름 한 줄만 표시한다. **Options > Detailed cards**로
  기존 gr·사용 개수 표시를 다시 켤 수 있고, 전체 정보는 카드 툴팁에도 그대로 나온다.
  썸네일 크기, SRC 애니메이션, 미사용 필터, 적용 시 애니메이션 복사도 Options에 있다.
  미사용 필터가 켜져 있으면 버튼에 **Options (unused)**로 표시한다.
- Inspector의 SRC / Layout / Timeline / Conditions / Advanced LR2는 좁을 때 여러 줄로
  표시한다. 탭 이동 화살표 없이 모든 항목에 접근하고, 창 너비나 프레임 수가 바뀌어도
  현재 항목을 유지한다. 입력 폭도 패널에 맞추며 좁은 Layout은 한 열로 배치한다.
- 기본 도킹 위치·크기와 Preview 공간은 변경하지 않는다. Timeline / Advanced의
  여러 프레임 비교표는 기존처럼 표 안에서 가로 스크롤한다.

## 선택한 Object에서 편집기로

Asset Browser 우클릭의 **Select first using Object** 또는 **Used by Objects**에서
사용처를 고르면 Object Browser/Inspector뿐 아니라 Preview도 열고 해당 DST의
점멸사각형을 즉시 갱신한다. 기존 파일 숨김/단독 보기 설정이나 조건은 바꾸지 않는다.

- Source image: 기존 선택/Asset 매핑으로 Image Manager를 연다. 다중 SRC는 첫 대응 crop.
- Edit text font: TEXT/BAR_TITLE의 schema `font` 열에 해당하는 현재 Preview 폰트.
  inactive/missing 슬롯은 오류를 안내한다. 다른 폰트에 미저장 초안이 있으면 확인한다.
- Appearance: 선택한 source row의 Simple Mode 분류와 대상까지 선택한다.
- Edit CSV: 해당 Object가 속한 파일의 Text Editor. 다른 미적용 초안은 덮어쓰지 않는다.
- Create artwork: 그림이 없는 배치용 Object를 위한 기존 Images from layout 대화상자.

이 버튼들은 탐색만 하며 Object 선택, 문서, Undo를 별도로 만들지 않는다.

## 선택했는데 Preview에서 보이지 않을 때

Object Browser의 오브젝트 행 또는 Inspector 상단 **(?)**에 커서를 올린다.
툴팁은 현재 로드된 IF/include 제외 상태, CSV 숨김/단독 보기, DST OP 조건과
타이머 번호, 애니메이션 시작 전/종료 후, 투명도·크기·화면 영역 등을 안내한다.
해당 CSV 위치와 확인할 창(Option List/Customize, Timer Control, File Manager)도 표시한다.
조회만 하므로 옵션·타이머·파일 숨김을 자동 변경하지 않는다.

여러 DST 상태 중 일부만 차단되면 다른 상태는 그려질 수 있다. Layout boxes에서는
IF/OP/타이머를 무시하는 정적 박스라는 점을 안내하며 CSV 숨김은 그대로 검사한다.
Preview 갱신 중이거나 NOWCOMBO처럼 Scene 코드가 제어하는 경우에는 불확실성을 표시한다.
검사를 통과하더라도 다른 그림에 가려지거나 텍스처가 투명할 수 있으므로, 실제 픽셀이
보인다고 단정하는 기능은 아니다.

## 저장과 종료

상단 **Pending: all (n)**(전체 미저장)의 숫자는 **모든 Workspace**의 저장/적용
대상 항목 수다. 숨겨진 Workspace와 스킨 없이 남은 외부 파일 초안도 포함하며,
클릭해서 여는 전체 검토창과 집계 범위가 같다. 고유 파일 수가 아닌 검토 항목 수다.

- Skin CSV (+ includes): 기존 Save. 이미지/폰트/미적용 초안은 저장하지 않는다.
- Text Editor / Custom Files draft: Review → 해당 도구에서 Apply → Skin CSV 저장.
- Painted image: Save file → 기존 Pixel Paint 백업/원자 저장.
- Image font: Save file → 기존 필드 검증/초안 적용/폰트 또는 DXA 정의 저장.

각 파일의 저장 성공은 독립적이다. 전체 파일의 단일 트랜잭션이나 강제 종료 복구를
제공하지 않는다. 오류 상세는 검토창에 남고 실패한 대상은 목록에서 없어지지 않는다.
다른 Workspace의 동일 파일 편집은 폰트의 기존 외부 변경 검증 등을 그대로 따른다.

앱 닫기/Alt+F4도 이 목록을 표시한다. 모두 저장하면 종료하며 Review/Keep working/Esc는
종료를 취소하고 작업으로 돌아간다. Exit without saving...은 추가 확인 후 남은
미저장 메모리 변경만 버린다. 이미 저장한 파일이나 외부에서 그린 PNG를 삭제하지 않는다.

## 검증

`--self-test-workflow`는 숨겨진 Workspace, 각 dirty 원본, 미적용 초안 저장 거부,
없는 paint texture의 저장 실패, 실제 파일 lock으로 인한 폰트 저장 실패/재시도,
TEXT의 font slot 이동과 기존 초안 보호, headless ImGui 검토창/Esc 취소를 검사한다.
기존 `image-font`, `image-font-dxa`, `save-recovery`, `layout-first`, `pixel-paint`도 유지한다.

수동 확인이 필요한 항목:

- 창을 좁힌 뒤 Asset Browser 두 시작점과 Inspector 바로가기, Preview 복귀.
- 여러 Workspace를 열고 하나를 숨긴 뒤 CSV·그림·폰트·텍스트 초안을 남겨 Alt+F4.
- Object Name 입력 도중 닫기, 검토 후 저장/취소/저장 실패/버리기 추가 확인.
- 같은 번호의 다른 IF/와일드카드, TEXT와 NUMBER 폰트의 서로 다른 도구 이동.
- Pixel Paint로 이미지 두 개를 수정하고 하나만 저장했을 때 다른 그림이 유지되는지.
- 네이티브 UI 조작과 실제 LR2 로드·재생은 자동 테스트와 별도로 확인한다.

### 구현 검증 기록 (2026-09-19)

- AI_1, 시작 commit `08a186e`. 이 변경은 아직 commit/push하지 않았다.
- `scripts/ui-map.ps1 -Check`: 19 windows / 18 flows, 통과.
- `scripts/docs-check.ps1`: 통과.
- `scripts/build.ps1` 및 `-Platform x64`: 모두 성공. 최종 Win32 증분 빌드는 경고 0;
  x64 전체 빌드는 기존 LR2/DxLib 등의 경고 932, 오류 0. workflow 소스의 새 경고는 없다.
- `scripts/test.ps1`: Win32 21/21, x64 실행 파일/별도 ResultsDirectory 지정도 21/21.
  기본 sandbox에서는 D3D 기반 4개 검사가 초기화에 실패하여, Windows 그래픽 세션에서
  전체를 다시 실행했다. 그래픽 세션 결과가 위 통과 수치다.
- `scripts/ai-context.ps1 -Check` 및 `git diff --check`: 통과.
- 네이티브 마우스 조작/Alt+F4와 실제 LR2 수동 검증은 미실행이다.

### Asset 사용처 선택 점멸 수정 (2026-09-19)

- `SelectImageAssetUsageObject`가 두 사용처 메뉴의 공용 선택 명령이다. 기존에는
  선택 후 bounds를 무효화하기만 했고 일반 Object는 Scene 재로드 전까지 복구되지 않았다.
- `object-reorder` 회귀 검사에 즉시 bounds 계산, 창 열기/탭 표시 요청, 이동/크기 조절
  상태 해제, 잘못된 index 거부, CSV revision/History 불변 검증을 추가했다.
- Release Win32 빌드 성공(기존 소스 경고 44, 오류 0), 전체 self-test 21/21 통과.
  UI 지도/문서/AI context 검사 통과. 이번 수정의 x64 재빌드와 실제 마우스 검증은 미실행.
- AI_1 미커밋 상태. 기존 UI 작업과 사용자 실행 로그는 보존했다.

### Back to Preview 탭 복귀 수정 (2026-09-19)

- 공용 `seUI.cpp`의 RevealWindowTab에서 같은 dock node의 이전 NavWindow를
  대상 창으로 옮긴다. 기존 탭/창은 닫지 않고 문서·선택·초안도 유지한다.
- `workflowTests.cpp`의 실제 ImGui dockspace/마우스 입력 검사에서 수정 전 exit 28로
  재현했다. 수정 후 세 전문 도구에서 복귀한 상태가 8프레임 유지되고, 다른 영역의
  child 창 포커스/ActiveId가 보존되는 것을 검증했다.
- Release Win32 증분 빌드 경고/오류 0, self-test 21/21, UI 지도·문서·AI context 검사 통과.
  네이티브 화면 수동 조작과 x64 재빌드는 이번 수정에서 미실행. AI_1 미커밋/미푸시.
