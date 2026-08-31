# SkinEditor repository instructions

이 파일은 이 저장소에서 작업하는 AI와 자동화 도구의 공통 계약이다. 사용자 지시가
더 구체적이면 사용자 지시를 우선하되, 아래 검증 기준과 데이터 불변식은 함께 지킨다.

## 작업 기준

- 기준 브랜치: `AI_2`
- 기준 빌드: `Release | Win32(x86)`
- 도구 체인: Visual Studio 2022 C++ (`v143`), Windows SDK, Direct3D 9
- 게시 대상: `https://github.com/GOMazk/SkinEditor`, branch `AI_2`
- 현재 폴더에 `.git`이 없을 수 있다. 이 경우 export 작업본으로 취급하고 push를
  시도하지 않는다. 게시 요청을 받으면 실제 checkout과 remote/branch를 먼저 검증한다.
- force-push, 기존 변경 폐기, 실제 LR2 사용자 데이터 수정은 명시적 요청 없이 하지 않는다.

## 시작 순서

1. `README.md`와 `docs/AI_COLLABORATION.md`를 읽는다.
2. 작업 종류에 따라 `docs/PROJECT_STATE.md`, `docs/UI_ARCHITECTURE.md`,
   `docs/BUILD_AND_TEST.md` 중 필요한 문서를 읽는다.
3. Git checkout이면 `git status --short`와 관련 diff를 확인해 사용자 변경을 보존한다.
4. UI 작업이면 `.\scripts\ui-map.ps1 -Check`로 현재 카탈로그와 소스 연결을 확인한다.
5. 변경 전에 소유 함수, 호출자, 상태 원본과 기존 테스트를 찾는다.

## 단일 기준 파일

| 영역 | 단일 기준 |
|---|---|
| UI 창/대화상자 이름, 의도, owner | `SkinEditor_DX9/uiCatalog.h` |
| 명령과 열 schema | `skinHelper.txt` 및 포함 RCDATA |
| Object 명령 그룹 | `skinObjGroup.txt` 및 포함 RCDATA |
| UI palette/component | `SkinEditor_DX9/seUI.h`, `SkinEditor_DX9/seUI.cpp` |
| 편집 상태와 작업 실행 | `WORKSPACE` 및 기존 CSV/Object model |
| 현재 기능 결정 | `docs/PROJECT_STATE.md` |
| 검증 기준 | `docs/BUILD_AND_TEST.md` |

같은 이름, schema 판정, selection, History 또는 저장 동작을 UI 컴포넌트에 복제하지
않는다. `arr_CommandHelp` 직접 접근 대신 `GetCommandHelp()`를 사용하고, workspace
창 제목은 `FormatSEUIWindowTitle()`을 사용한다. 동적 배열 index를 장기 식별자로
저장하지 말고 `$SE_OBJECT_ID`와 기존 복원 경로를 유지한다.

## 변경 규칙

- 작은 관련 변경으로 끝내고 무관한 refactor와 dependency 추가를 피한다.
- 공개 포맷 이름은 `OLRskin 0.9`로 고정한다. JSON의 정수 `version: 9`는
  `0.9`의 기존 직렬화 표현이다. 사용자의 명시적 허가 없이는 포맷 버전을 올리거나,
  package entry/field, authority, compiler ownership, import/export 의미 또는 OLRskin
  기능 계약을 추가·삭제·변경하지 않는다. 요청된 호환성 버그 수정도 기존 0.9 계약을
  복원하는 범위에서만 수행한다.
- CP932/Shift-JIS 저장, include 파일 소유권, IF/ELSEIF/ELSE 문맥을 보존한다.
- ImGui `Begin/End`, child/table/tab/popup, ID stack을 모든 분기에서 맞춘다.
- UI는 intent만 반환하고 CSV/model/selection/history의 별도 사본을 소유하지 않는다.
- 창이나 흐름을 추가하면 `uiCatalog.h`, UI 지도, 관련 문서를 같은 변경에서 갱신한다.
- 버그 수정에는 가능한 범위에서 회귀 self-test를 추가한다.

## 완료 검증

저장소 루트에서 순서대로 실행한다.

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\ui-map.ps1 -Check
powershell -ExecutionPolicy Bypass -File .\scripts\docs-check.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\test.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\ai-context.ps1 -Check
```

실행하지 않은 검증을 통과했다고 쓰지 않는다. 네이티브 GUI 또는 실제 LR2 스킨을
직접 확인하지 못했다면 자동 테스트와 수동 검증의 경계를 인계문에 명시한다.

## AI 인계 형식

모든 인계에는 다음을 짧게 남긴다.

- 목표와 사용자에게 보이는 결과
- 바꾼 파일과 중요한 설계 결정
- 실제 실행한 명령과 결과
- 남은 위험, 미실행 수동 테스트, 알려진 경고
- Git checkout/branch/dirty 상태와 게시 여부

상세 템플릿은 `docs/HANDOFF_TEMPLATE.md`, 전체 협업 절차는
`docs/AI_COLLABORATION.md`를 따른다.
