# AI 협업 가이드

이 문서는 처음 참여하는 AI가 기존 결정을 되풀이하지 않고, 다른 AI의 변경을
안전하게 이어받도록 하는 시작점이다. 기능 상세를 모두 복사하지 않고 단일 기준
문서와 자동 생성 컨텍스트를 연결한다.

## 5분 시작 절차

1. 루트 `AGENTS.md`에서 작업 기준과 금지 사항을 확인한다.
2. `docs/PROJECT_STATE.md`의 관련 기능 절과 결정 기록을 읽는다.
3. UI 작업이면 `docs/UI_ARCHITECTURE.md`와 생성된 `.build/ui-map/index.html`에서
   owner 함수와 연결된 컨트롤을 찾는다.
4. `.\scripts\ai-context.ps1 -Check`를 실행해 `.build/ai-context`에 최신 인계
   묶음을 만든다.
5. Git checkout이면 현재 branch와 dirty diff를 확인한다. `.git`이 없으면 게시용
   checkout이 아닌 export 작업본으로 취급한다.

## 문서의 우선순위

서로 다른 문장이 충돌하면 다음 순서로 판단한다.

1. 현재 사용자 요청
2. 루트 `AGENTS.md`
3. 코드와 자동 테스트가 증명하는 현재 동작
4. `docs/PROJECT_STATE.md`의 결정 기록
5. 기능별 문서와 생성된 UI 지도
6. 오래된 주석 또는 샘플 코드

문서와 코드가 다르면 코드를 무조건 정답으로 선언하지 않는다. 재현 또는 테스트로
현재 동작을 확인하고, 의도된 결정인지 판단한 뒤 코드와 문서를 같은 변경에서 맞춘다.

## AI가 읽을 자료

| 목적 | 먼저 읽을 것 | 기계용 자료 |
|---|---|---|
| 전체 구조 | `README.md`, `PROJECT_STATE.md` | `.build/ai-context/context-manifest.json` |
| UI 수정 | `UI_ARCHITECTURE.md`, `UI_MAP.md` | `.build/ui-map/ui-map.json` |
| 빌드/CI | `BUILD_AND_TEST.md` | JUnit, MSBuild binlog |
| AI_1 Workspace 버그 이식 | `WORKSPACE_MULTI_PREVIEW_FIX.md` | `--skin-multi-workspace-smoke` |
| 변경 인계 | `HANDOFF_TEMPLATE.md` | `.build/ai-context/context-pack.md` |
| 결정 이유 | `docs/decisions/` | manifest의 문서 SHA-256 |

`ui-map.json`은 창/대화상자 의도, owner, 기본 dock, 소스 컨트롤 위치를 제공한다.
동적 label은 `<dynamic:...>`으로 남을 수 있으므로 해당 source line을 직접 확인한다.
Object Browser와 Object Inspector처럼 같은 owner 함수를 공유하면 focus pack에도
owner 전체 컨트롤이 포함될 수 있다. pack의 `Shared owner warning`과 sibling key를
확인한 뒤 실제 `ImGui::Begin` 구간을 구분한다.

## 역할을 나눌 때

큰 작업은 다음 경계로 나누면 충돌이 적다.

- 조사자: 재현, 호출 경로, 불변식과 테스트 후보를 기록하고 소스는 수정하지 않는다.
- 구현자: 합의된 owner 안에서 최소 변경과 회귀 테스트를 만든다.
- 검증자: diff를 읽고 UI map, build, self-test, 필요한 수동 시나리오를 독립 확인한다.
- 문서 담당: 결정과 검증 결과를 단일 기준 문서에 반영하고 중복 설명을 제거한다.

여러 AI가 동시에 같은 파일을 수정하지 않도록 owner 범위를 먼저 나눈다.
`winWorkspace.cpp`, `PROJECT_STATE.md`, project XML처럼 충돌이 잦은 파일은 한 작업자가
통합하고, 다른 작업자는 발견 사항과 patch 후보만 인계하는 편이 안전하다.

## 작업 계약

작업을 넘길 때 다음 다섯 항목을 먼저 고정한다.

1. **목표**: 사용자가 관찰할 수 있는 완료 상태
2. **범위**: 수정 가능한 owner 함수와 파일
3. **불변식**: selection, History, encoding, branch/file ownership 등 유지할 동작
4. **증거**: 재현 로그, source line, UI-map key, 관련 test
5. **완료 조건**: 자동 검사와 필요한 수동 smoke test

추정과 확인된 사실을 구분한다. 재현하지 못한 현상이나 실행하지 않은 검증은
`미확인`으로 남긴다.

## 변경 후 인계

`docs/HANDOFF_TEMPLATE.md`를 복사해 결과를 작성한다. 최소한 다음이 있어야 한다.

- 변경 전 문제와 변경 후 동작
- 핵심 파일/심볼과 선택한 설계 이유
- 실행한 검증 명령, exit code 또는 테스트 수
- 자동화가 다루지 못한 네이티브 GUI/실제 스킨 검증
- 알려진 기존 warning과 새 regression 여부
- Git branch, dirty 상태, commit/push 여부

컨텍스트 팩은 설명을 대신하지 않는다. 다른 AI가 파일 무결성과 현재 UI 수치를
빠르게 확인하게 하는 첨부물이다.

## GitHub 인계

게시 대상은 `https://github.com/GOMazk/SkinEditor`의 `AI_2` branch다. push 요청을
받은 작업자는 다음을 확인한다.

1. 현재 디렉터리가 실제 Git checkout인지 확인한다.
2. `git remote -v`와 `git branch --show-current`로 대상이 정확한지 확인한다.
3. 관련 없는 사용자 변경과 비밀 파일이 포함되지 않았는지 확인한다.
4. 자동 검증 결과를 handoff 또는 PR 본문에 기록한다.
5. push 뒤 remote `AI_2` SHA를 다시 확인한다.

force-push나 다른 branch 게시로 대체하지 않는다. 인증이 필요하면 그 시점에만
사용자에게 요청한다.
