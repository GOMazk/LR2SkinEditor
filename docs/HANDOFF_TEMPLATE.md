# SkinEditor 변경 인계 템플릿

## 목표와 결과

- 요청:
- 사용자에게 보이는 결과:
- 완료 여부: 완료 / 부분 완료 / 차단

## 변경 범위

| 파일 또는 심볼 | 변경 내용 | 이유 |
|---|---|---|
| | | |

## 유지한 불변식

- [ ] CP932/Shift-JIS 저장과 문자열 표시
- [ ] include 파일 소유권
- [ ] IF/ELSEIF/ELSE branch 문맥
- [ ] Object selection과 `$SE_OBJECT_ID` 복원
- [ ] History/Undo와 파생 모델 rebuild
- [ ] workspace별 ImGui ID와 Begin/End 균형

해당하지 않는 항목은 이유와 함께 지운다.

## 검증 증거

| 검사 | 실행 명령 또는 시나리오 | 결과 |
|---|---|---|
| UI contract | `.\scripts\ui-map.ps1 -Check` | 미실행 |
| Docs contract | `.\scripts\docs-check.ps1` | 미실행 |
| Build | `.\scripts\build.ps1` | 미실행 |
| Self-tests | `.\scripts\test.ps1` | 미실행 |
| AI context | `.\scripts\ai-context.ps1 -Check` | 미실행 |
| Manual GUI | skin/scene/resolution/steps | 미실행 |

## 남은 위험과 다음 작업

- 자동화가 확인하지 못한 것:
- 알려진 기존 warning:
- 후속 작업:

## Git 상태

- Checkout 경로:
- Branch:
- 시작 SHA / 종료 SHA:
- Dirty 또는 관련 없는 변경:
- Commit/push/PR 상태:
