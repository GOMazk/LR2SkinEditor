# Simple Mode 이미지 / TTF 폰트 교체

숫자, NOWCOMBO, NOWJUDGE의 이미지를 교체한다. OLRskin 0.9의 version, JSON 필드,
package entry 종류, authority 및 compiler는 변경하지 않는다. 결과는 일반 PNG와
기존 LR2 `#IMAGE`, `$SRC_IMAGE`, `#SRC_*` 필드이며 글꼴 파일/생성 설정은 저장하지 않는다.

## 사용

1. 스킨을 열고 Simple Mode의 Number fonts 또는 Judgement fonts에서 대상을 고른다.
2. Image replacement는 기존 Asset 또는 외부 이미지를 교체한다. TTF font는 글꼴
   파일을 선택하고 셀 크기, 여백, 색상, 테두리, 그림자와 판정 문구를 설정한다.
3. TTF의 Generate preview로 결과를 생성하고 적용 대상 목록을 확인한다.
4. Apply generated font로 적용한다. Undo/Redo는 문서 변경 전체를 한 번에 복원한다.
5. 기존 Save / Save OLRskin / Export LR2 경로를 사용한다.

TTF에는 숫자 0~9, 11칸 방식의 빈칸, 24칸 방식의 양수/음수 숫자·빈칸·부호를
생성한다. 반복 animation grid에는 같은 정적 글자를 반복하며 원래 cycle과 timer를
유지한다. TTF의 원본 애니메이션 생성 기능은 아니다. NOWJUDGE는 선택한 판정의 문구를
편집하며 다른 판정은 별도로 선택한다. 작은 숫자도 같은 배율·기준선을 사용해
글자마다 폭을 강제로 늘리지 않는다.

기본 셀 크기는 대상 이미지에서 얻으며 DST의 위치, 크기, 정렬, 자릿수, timer,
조건은 변경하지 않는다. 이미지 교체도 대상의 grid/cycle을 유지한다. 서로 다른
grid는 별도로 교체해야 한다. 생성 실패는 기존 이미지를 대체하지 않는다.

## 소유권과 구현 위치

- `SkinEditor_DX9/fontAtlas.h/.cpp`: 편집기 전용 입력과 CPU ARGB 래스터화. 최대
  256셀, 4096x4096, 16M pixel. 숫자 10/11/24 분류 우선순위는 LR2 renderer와 같다.
- `SkinEditor_DX9/winWorkspaceSimpleFonts.cpp`: 파일 선택, 미리보기/스타일 UI,
  RGBA 이미지 crop 읽기. WORKSPACE가 CPU 결과와 미리보기 texture를 소유한다.
- `SkinEditor_DX9/winWorkspace.cpp`: `drawSimpleMode`, `SimpleModeSlotMatchesScope`,
  `ApplySimpleModeFontBitmap`이 선택 범위와 CSV/History/파일 적용을 소유한다.
- `SkinEditor_DX9/fontAtlasTests.cpp`: 실제 PNG 생성/적용, 선언 순서, 다른 판정 보존,
  Undo/Redo, 이미지 가져오기와 변경하지 않은 OLRskin 0.9 packager 통합 검사.
- `SkinEditor_DX9/uiCatalog.h`: `simple-font-tools` flow와 소유 함수 연결.

글꼴은 Windows GDI+의 private font collection으로 읽고 설치하지 않는다.
[Microsoft API 문서](https://learn.microsoft.com/en-us/windows/win32/api/gdiplusheaders/nf-gdiplusheaders-privatefontcollection-addfontfile).
TTF를 기본으로 하며 GDI+가 읽지 못하는 OTF/TTC 변형은 오류로 표시한다. 글꼴 모음은
첫 family와 제공되는 style을 사용한다. 글꼴 family/가변 축 선택은 제공하지 않는다.

## 중요한 불변식

LR2는 이미지 선언을 순서대로 읽는다. 폰트용 PNG는 정보 header 뒤, 기존 이미지와
source보다 앞에 graphic 0으로 선언한다. 모든 기존 schema의 일반 graphic 참조
0~98을 1씩 이동하므로 선택하지 않은 오브젝트는 같은 원본 이미지를 계속 사용한다.
100 이상 특수 handle과 음수 handle은 그대로 둔다. 새 PNG를 참조하도록 선택한
source만 0으로 바꾼다. 모든 행 변경과 두 선언 삽입을 한 snapshot으로 묶는다.
기존 파일 순서, include 소유권, IF 문맥, DST draw order는 유지된다.

이미지 100개 이상, graphic 99 참조, `#IMAGE,CONTINUE`, 알 수 없는 SRC schema는
안전한 graphic 이동을 보장할 수 없어 적용을 거부한다. 사용자 파일이나 기존 PNG를
덮어쓰지 않는다. Undo 후 생성 PNG는 미사용 리소스로 남아 Redo가 같은 파일을 사용한다.

1P/2P 짝은 서로 다른 player suffix와 동일 명령 family, source index, owner 파일,
IF group으로 한정한다. 일반 NUMBER에는 추정 pair를 만들지 않는다. 이미지의 넓은
명령/category 범위는 대상 목록을 확인한 명시적 선택이며, TTF는 선택 대상/짝만 제공한다.
설정이나 projection generation이 바뀌면 이전 미리보기로 적용할 수 없다.

## 검증 및 한계

저장소 루트에서 `scripts/build.ps1`, `scripts/test.ps1`을 실행한다. `font-atlas`
self-test는 Windows Arial을 사용하며 GDI+와 Direct3D 9가 필요하다. 다른 검증은
`ui-map.ps1 -Check`, `docs-check.ps1`, `ai-context.ps1 -Check`다.

수동으로는 임의의 TTF, 일본어/한글 파일 경로, 작은 셀과 큰 테두리, 영문/한글 판정,
숫자 10/11/24칸, 1P/2P와 다른 IF/다른 include, 생성 후 연속 스킨 로드, 실제 LR2의
읽기와 표시를 확인한다. 자동 PNG/문서 검사는 네이티브 UI 조작과 실제 플레이 검증을
대신하지 않는다.
