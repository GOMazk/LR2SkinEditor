# 0001. AI-readable UI and handoff contract

- 상태: Accepted
- 결정일: 2026-08-26

## 배경

SkinEditor UI는 큰 `winWorkspace.cpp` 안의 즉시 모드 ImGui 호출과 여러 공유 상태로
구성되어 있다. 창 제목, 도킹 문자열, 메뉴 label이 흩어지면 사람과 AI 모두 같은
화면을 다른 이름으로 해석하고 workspace별 ImGui ID 충돌을 놓치기 쉽다. 긴 기능
문서만 전달하는 방식도 현재 수치, owner 함수와 실제 검증 결과를 빠르게 확인하기
어렵다.

## 결정

1. `SkinEditor_DX9/uiCatalog.h`를 창과 비도킹 surface의 이름, 의도, owner와 기본
   위치에 대한 단일 기준으로 사용한다.
2. `scripts/ui-map.ps1`이 카탈로그와 ImGui 호출을 결합해 JSON, Markdown, HTML을
   만들고 CI에서 계약을 검사한다.
3. 루트 `AGENTS.md`가 모든 AI의 시작 순서, 불변식, 검증과 게시 경계를 정의한다.
4. `scripts/ai-context.ps1`이 주요 문서의 hash, UI-map summary, 테스트 결과를
   기계용 manifest와 짧은 context pack으로 만든다.
5. 작업 결과는 `docs/HANDOFF_TEMPLATE.md` 형식으로 인계한다.

## 결과

- 창 이름과 owner를 한 번만 정의하고 UI map과 runtime이 같은 계약을 사용한다.
- 다른 AI는 전체 소스를 먼저 읽지 않고도 관련 owner와 컨트롤을 찾을 수 있다.
- 문서가 바뀌면 manifest hash가 바뀌어 인계 자료의 기준을 비교할 수 있다.
- 정적 scanner는 동적 label과 runtime 상태를 완전히 해석하지 못하므로, 관련 source
  확인과 네이티브 GUI smoke test는 여전히 필요하다.

## 배제한 대안

- `winWorkspace.cpp` 전체를 AI prompt에 매번 첨부: 컨텍스트가 크고 결정과 구현이
  섞여 변경 비교가 어렵다.
- 별도 GUI builder가 runtime state를 소유: 기존 `WORKSPACE`/CSV/Object model과
  상태가 이중화되어 selection과 History가 갈라질 위험이 있다.
