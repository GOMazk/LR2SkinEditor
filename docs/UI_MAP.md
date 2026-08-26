# AI/UI 지도

UI 지도는 ImGui 코드를 모두 읽지 않고도 창의 의도, 소유 함수, 기본 도킹
위치와 컨트롤 소스 위치를 파악할 수 있게 만든 개발 보조 도구다.

## 생성

저장소 루트에서:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\ui-map.ps1 -Check
```

결과는 `.build\ui-map` 아래에 생성된다.

- `ui-map.json`: AI와 자동화가 읽는 전체 창/컨트롤 계약
- `ui-map.md`: 창별 의도와 소스 위치를 빠르게 읽는 문서
- `index.html`: 검색, dock 필터, 기본 레이아웃과 선택 창 JSON 복사를 제공하는 로컬 UI

`-Check`는 카탈로그의 중복 key/title, 없는 owner 함수, 카탈로그를 사용하지 않는
런타임 창 제목을 실패로 처리한다. CI도 같은 검사를 수행한다.

## 소스 규칙

`SkinEditor_DX9\uiCatalog.h`가 workspace tool window와 dialog/shell의 단일
기준이다. `SEUIWindowSpec`은 도킹 가능한 창을, `SEUISurfaceSpec`은 Help 같은
대화상자와 application/workspace 흐름을 설명한다. 각 항목은 다음을 포함한다.

- 안정적인 기계용 key
- 사용자에게 보이는 title
- 창의 역할과 owner draw function
- Windows 메뉴 group
- 기본 dock region과 visibility

창 제목을 직접 `snprintf()`로 복제하지 말고
`FormatSEUIWindowTitle()`을 사용한다. 그렇게 해야 Windows 메뉴, 도킹 타겟,
실제 ImGui window가 같은 표시 이름과 workspace별 ID를 공유한다.

## AI에게 전달할 때

전체 수정이면 `scripts\ai-context.ps1 -Check`로 생성한 context pack과
`ui-map.json`을 함께 제공한다. 한 화면만 수정할 때는 다음처럼 focus pack을 만들거나
`index.html`에서 화면을 선택한 뒤 **Copy selected JSON**을 사용한다.

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\ai-context.ps1 `
  -UiKey object-inspector -Check
```
