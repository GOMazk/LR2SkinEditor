# Text Editor 입력 보조

`Windows > Data > Text Editor` 또는 File Manager에서 CSV를 더블클릭해 연다.
기존 다중 행 편집기에 LR2 명령 자동완성과 인자 안내를 추가했다.

## 사용법

| 입력/조작 | 동작 |
|---|---|
| `#SRC_`, `dst_n`, `note` | 명령 이름의 시작 또는 일부가 일치하는 후보를 표시한다. 대소문자를 구분하지 않는다. |
| timer 인자에 `SongCh` | `11  SongChange`처럼 번호와 이름을 보여주고 확정하면 `11`을 넣는다. |
| `↑` / `↓` | 입력 커서를 유지하면서 후보를 고른다. |
| `Tab` / `Enter` | 선택한 후보를 현재 명령 또는 인자에 넣는다. |
| 후보 클릭 | 선택한 후보를 넣고 편집기로 돌아온다. |
| `Esc` | 후보 목록만 닫으며 초안을 되돌리지 않는다. |
| `Ctrl+Space` / `Suggest` | 현재 위치의 후보 목록을 다시 연다. |
| `Ctrl+Z` | 자동완성 한 번을 텍스트 편집 한 단계로 취소한다. |

아래 도움말에는 현재 명령, CSV 인자 번호/이름과 전체 인자 순서를 표시한다.
현재 인자는 `[ ... ]`로 구분한다. 선택한 후보의 schema 미리보기는 목록 아래와
마우스 툴팁에서 확인할 수 있다. 후보는 최대 64개이며 입력을 더 하면 좁혀진다.

예를 들어 다음 줄의 마지막 timer 위치에서 `SongCh`를 입력하고 확정하면
`SongChange`에 대응하는 LR2 번호가 들어간다.

```text
#SRC_IMAGE,0,0,0,0,16,16,1,1,0,SongCh
#SRC_IMAGE,0,0,0,0,16,16,1,1,0,11
```

명령 뒤에 이미 쓴 인자, 다른 줄, UTF-8 주석과 공백은 유지한다. 목록이 없으면
Tab은 탭 문자, Enter는 줄바꿈으로 동작한다. 자동완성은 초안에만 적용된다.
기존과 같이 **Apply → Save**를 사용하며, Apply 후에는 기존 Preview가 갱신된다.

## 구현과 검증 범위

- `codeEditorAssist.h/.cpp`: 입력 문맥, schema 후보, popup/인자 도움말과 원자적 치환.
- `codeEditorAssistTests.cpp`: 명령/값 검색과 UTF-8/기존 인자 보존, 버퍼 경계,
  실제 ImGui 키보드·마우스 프레임과 Undo 검증.
- `seHelper.h/.cpp`: 로드된 schema의 명령 열거와 정확한 명령/열 조회.
- 기존 Workspace, Text Editor Apply와 CP932 변환/INCLUDE 보호/History는 계속
  같은 경로를 사용한다. 외부 편집기나 새 의존성은 추가하지 않았다.
- 작업과 로컬 커밋은 `AI_2`에서만 수행한다. `AI`·`AI_1`을 수정하지 않는다.

네이티브 GUI에서 IME 입력, DPI 변경, 좁은 도킹 창, 긴 파일을 스크롤한 뒤의
popup 위치와 실제 마우스 조작은 별도 수동 검증 항목이다. 자동 입력 테스트는
운영체제의 키보드/마우스를 조작하지 않는다.

## 현재 범위

추천은 LR2 command schema에 등록된 명령과 심볼 값에 한정한다. `abs` 같은
일반 프로그래밍 언어 함수, 경로/이미지 썸네일 완성, 전체 문법 색칠, 자동 파일 저장은
추가하지 않았다. 기존 Text Editor의 INCLUDE 순서 보존과 CP932 검증도 그대로 적용된다.

## 2026-09-12 검증본

- Release Win32/x64 빌드 통과. 각각 별도 검증 폴더에서 self-test **17/17 통과**.
- `ui-map.ps1 -Check`: 18 windows, 15 flows/shells, 792 controls, 오류 0.
- `docs-check.ps1`: 17개 문서, 상대 링크 22개, 오류 0.
- `ai-context.ps1 -Check`와 `git diff --check`도 수행한다.
- Win32: `.build/text-assist-win32-20260912/SkinEditor_DX9.exe`.
- x64: `.build/text-assist-x64-20260912/SkinEditor_DX9.exe`.
- 두 폴더의 exe/PDB/D3DX9 runtime은 복사 후 SHA-256 일치를 확인했다.
- 이전 검증본과 저장소가 추적하는 Release 바이너리는 유지했다. 이번 입력 보조를
  확인할 때는 위 새 폴더의 실행 파일을 사용한다.
- 기존 LR2/DxLib 컴파일 경고는 남아 있다. 네이티브 IME/DPI와 실제 GUI 수동
  확인은 위의 수동 검증 범위로 남긴다. GitHub push/릴리즈는 하지 않았다.
