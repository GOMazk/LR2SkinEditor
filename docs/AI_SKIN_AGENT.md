# AI 스킨 제작 유틸 v0.2

Codex·Claude 같은 외부 코딩 에이전트가 JSON 레시피를 작성하고 로컬 명령줄에서
스킨을 생성·검사·렌더링하는 도구다. PNG를 등록해 아틀라스와 SRC 연결을 만들고,
상태별 미리보기와 원본 파일/행을 포함한 표시 진단을 에이전트가 다시 읽을 수 있다.
프로그램 자체는 언어 모델이나 API를 호출하지 않는다. `AI_experimental`에서만
개발하며 기존 레시피 `version: 1`, 일반 LR2 CSV/PNG와 OLRskin 0.9 계약을 유지한다.

## 준비와 실행

Windows, Python 3.10+, 이 브랜치에서 빌드한 Release Win32 실행 파일이 필요하다.
저장소 루트에서 다음을 실행한다. PNG 등록을 쓸 때만 Pillow가 필요하며, 전체 유틸
테스트도 이 의존성을 사용한다. 이미지가 없는 기존 레시피는 Python 표준 라이브러리만
사용한다.

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
python -m pip install -r tools/requirements-agent.txt
python -B tools/skin_agent.py --editor SkinEditor_DX9/Release/SkinEditor_DX9.exe schema --command '#DST_NOTE'
python -B tools/skin_agent.py --editor SkinEditor_DX9/Release/SkinEditor_DX9.exe build examples/agent/blue-play7.json --out .build/my-first-agent-skin
python -B tools/skin_agent.py --editor SkinEditor_DX9/Release/SkinEditor_DX9.exe inspect .build/my-first-agent-skin/LR2files/Theme/Generated/skin.lr2skin --object preset_note_0
```

`--editor`는 전역 옵션이므로 항상 하위 명령 앞에 둔다. 오래된 묶음 실행 파일에는
상태 렌더링/진단 기능이 없을 수 있다. 현재 소스로 다시 빌드하고, `schema`의
`capabilities`에 `state_render`, `visibility_diagnostics`가 있는지 확인한다.
`.build`의 로컬 실행 묶음은 Git에 포함되지 않으므로 checkout만 받았다면 먼저 빌드한다.

`schema`와 `inspect`는 graphics 없이 동작한다. `build`, `render`, `preview-suite`,
`diagnose`는 숨겨진 네이티브 Direct3D 9 환경을 사용한다. 네이티브 요청 하나의 제한은
60초이며, 상태 묶음은 각 상태를 순차 실행한다. 실패는 exit 1과
`{"ok":false,"error":"..."}`로 반환한다. 잘못된 명령줄 인자는 argparse의 exit 2다.

Direct3D 초기화가 간헐적으로 보고서 없이 실패했던 현상은 근본 원인이 아직 확정되지
않았다. 새 네이티브 시작 경로는 초기화 실패 시 `code: graphics_initialization_failed`를
담은 오류 보고서를 남긴다. 이는 실패를 식별하기 위한 개선이며 초기화 문제 해결을
뜻하지 않는다. 자동 재시도나 실패를 성공으로 처리하는 동작은 없다.

## Codex·Claude 작업 순서

1. `schema`로 네이티브 명령/열 정의를 읽는다. `--values`로 심볼 값 표도 받을 수 있다.
2. 기본 레시피를 `build --no-preview`로 새 폴더에 만들고 `inspect`로 Object ID를 찾는다.
3. `objects[].set`과 `assets`를 작성해 다른 새 폴더에 build한다.
4. `objects.json`, `preview.png`로 배치를 확인하고, PNG를 등록했다면 `assets.json`의 연결도 읽는다.
5. PLAY 스킨은 `preview-suite`의 갤러리와 상태별 JSON을 읽어 숫자 잘림, 게이지와 판정을 확인한다.
6. 보이지 않는 요소는 `diagnose --object ID`의 원본 `file`/`line`, `code`, `hint`를 읽는다.
7. 레시피를 고쳐 새 출력 폴더에 반복한다. 원본 스킨 편집·실제 LR2 설치는 별도 작업이다.

이 흐름에는 에이전트의 로컬 명령 실행과 PNG/JSON 읽기 기능만 필요하다. 전용 MCP
서버, 계정 연결, API 키 설정은 제공하지 않는다. 진단의 파일/행은 에이전트가 해당
소스를 찾아가는 근거이며, 이 CLI가 편집기 선택 위치를 자동 이동하지는 않는다.

## JSON 제작 지시와 PNG 등록

다음 내용을 `examples/agent/overworld-assets.json`에 저장하면 저장소에 있는
`blockbeat/overworld.png`를 배경과 작은 앞쪽 이미지에 함께 연결할 수 있다.
PNG 경로는 **레시피 파일의 폴더 기준**이다.

```json
{
  "version": 1,
  "scene": "play7",
  "width": 1280,
  "height": 720,
  "title": "Overworld asset example",
  "maker": "SkinEditor experimental",
  "objects": [
    {"id": "preset_background", "set": {"r": 255, "g": 255, "b": 255}}
  ],
  "assets": [
    {
      "id": "overworld",
      "path": "blockbeat/overworld.png",
      "bind": [{"object": "preset_background", "command": "#SRC_IMAGE"}],
      "image": {"x": 990, "y": 535, "w": 240, "h": 135, "layer": "front"}
    }
  ]
}
```

```powershell
python -B tools/skin_agent.py --editor SkinEditor_DX9/Release/SkinEditor_DX9.exe build examples/agent/overworld-assets.json --out .build/my-overworld-assets
```

| 필드 | 계약 |
|---|---|
| version | 제작 지시 버전 1. OLRskin 포맷 버전과 별개다. 기존 v0.1 레시피도 그대로 받는다. |
| scene | play7, play5, double14, double10, pms9, select, decide, result, battle7, battle5, battle9, course-result |
| width / height | 각각 320..1920, 240..1080의 정수. 기본 1280×720. |
| title / maker | 1..120자, CP932로 손실 없이 저장 가능해야 하며 쉼표/개행/NUL은 거부한다. |
| objects | 최대 512개. `id`와 `set`만 허용하며 기존 preset Object ID를 사용한다. |
| set | x/y: -8192..8192, w/h/size: 1..8192, a/r/g/b: 0..255, angle: -3600..3600. 모두 정수. |
| assets | 선택 필드. 최대 128개이며 각 항목에 `id`, `path`, 비어 있지 않은 `bind` 또는 `image`가 필요하다. 둘 다 지정해도 된다. |
| asset.id | ASCII 영문/숫자/밑줄/하이픈 1..64자. 중복을 거부한다. |
| asset.path | 정적 PNG 로컬 파일. 레시피 기준 상대 경로 또는 명시적 절대 경로. APNG는 거부한다. |
| asset.bind | `{ "object": "preset_note_0", "command": "#SRC_NOTE", "index": 0 }` 형태. `index`는 선택이며 해당 명령의 네이티브 index 필드다. |
| asset.image | x/y: -8192..8192, w/h: 1..8192, `layer`: `back` 또는 `front`(기본). 새 `asset_<id>` Object를 만든다. |
| asset.div_x / div_y / cycle | 기본 1 / 1 / 0. 분할 수 1..2048, 주기 0..600000ms. PNG 너비/높이가 분할 수로 정확히 나누어져야 한다. |

`set`은 Object의 **모든 DST keyframe**에 값을 대입한다. 배치 애니메이션을 상대
이동하지 않는다. `bind`는 정확히 한 SRC 행의 gr/x/y/w/h/div_x/div_y/cycle을 변경하고,
타이머·조건·DST 등 다른 필드는 유지한다. 같은 명령이 여러 행에 있으면 `index`로
하나를 지정하며 누락·중복·모호한 바인딩은 거부한다. 필드 위치는 네이티브 schema에서
읽는다. 생성 후 네이티브 parser로 Object 수와 적용 값을 다시 검사한다.

새 `image`는 `blend=1`로 PNG 알파를 사용한다. `back`은 기존 모든 Object보다 먼저,
`front`는 마지막에 그린다. 기본 배경이 `back` 이미지를 덮을 수 있으므로 배경 교체는
위 예제처럼 `preset_background`에 바인딩한다. 기존 Object를 바인딩할 때는 원래 DST의
색과 블렌드가 유지된다. 새 이미지 배치는 `image`에서 지정한다.

PNG 한 장과 아틀라스 페이지의 최대 크기는 2048×2048이다. 원본 RGBA 픽셀을 회전·확대·축소
없이 레시피 순서로 배치하고 이미지 사이에 2px를 띄운다. 공간이 부족하면 다음 페이지를
만든다. 압축 파일은 한 장 32MiB/전체 64MiB, 디코딩 픽셀 합계는 32×1024×1024로 제한한다.
생성되는 파일명은 ASCII이며 원본 PNG 경로는 CSV에 넣지 않아 한국어 입력 파일명도
사용할 수 있다. 단, 최종 스킨을 여는 네이티브 경로는 시스템 코드 페이지와 기존
MAX_PATH 제약을 따른다. 기존 `#IMAGE` 순서와 gr 번호는 보존한다.

## 상태별 렌더링과 일괄 미리보기

상태를 지정하지 않은 `render`는 기존 Preview 동작을 유지한다. `render --state`와
`preview-suite`, `diagnose --state`의 고정 상태는 PLAY 계열만 지원한다:
play7/play5/double14/double10/pms9/battle7/battle5/battle9. `diagnose`는 상태를 생략하면
그 밖의 장면도 로드한 네이티브 Preview 기준으로 검사한다.

단일 상태 예제를 `.build/preview-state.json`에 저장한다.

```json
{
  "name": "full-gauge-perfect",
  "time_ms": 1000,
  "gauge": 100,
  "score": 999999999,
  "exscore": 99999999,
  "combo": 9999999,
  "max_combo": 9999999,
  "bpm": 9999,
  "judge": "perfect",
  "notes": "long",
  "timers": {"100": 100}
}
```

```powershell
python -B tools/skin_agent.py --editor SkinEditor_DX9/Release/SkinEditor_DX9.exe render .build/my-first-agent-skin/LR2files/Theme/Generated/skin.lr2skin --state .build/preview-state.json --out .build/full-gauge-perfect.png
python -B tools/skin_agent.py --editor SkinEditor_DX9/Release/SkinEditor_DX9.exe preview-suite .build/my-first-agent-skin/LR2files/Theme/Generated/skin.lr2skin --out .build/my-first-preview-suite
python -B tools/skin_agent.py --editor SkinEditor_DX9/Release/SkinEditor_DX9.exe states
```

기본 묶음은 게이지 0/80/100%, 최대 숫자 자릿수, 판정 6종(poor/miss/bad/good/great/perfect),
일반/롱/지뢰 노트와 키 누름 상태로 총 14개다. `states`는 이 상태 배열을 JSON으로
출력한다. 직접 작성한 1..32개 상태 배열은 `preview-suite --states 파일.json`으로 준다.
상태 이름은 ASCII 영문/숫자/밑줄/하이픈 1..64자이며 대소문자를 무시해 중복을 거부한다.

| 상태 필드 | 값과 기본값 |
|---|---|
| time_ms | 0..3600000, 기본 1000ms |
| gauge | 0..100, 기본 80 |
| score / exscore | 각각 0..999999999 / 0..99999999, 기본 123456 / 1234 |
| combo / max_combo | 0..9999999, 기본 123 / 321 |
| bpm | 1..9999, 기본 150 |
| judge | none(기본), poor, miss, bad, good, great, perfect |
| notes | all(기본), normal, long, mine, none |
| timers | 최대 128개. 문자열 ID `"0"`..`"499"` → 경과 ms(-1은 비활성, 0..3600000은 활성). 기본 빈 객체. |

숫자는 정수만 받는다. 플레이어별 점수·게이지는 양쪽 플레이어에 동일하게 적용한다.
고정 상태는 시간과 샘플 노트/판정/타이머를 명시해 비교 가능한 프레임을 만드는 기능이다.
실제 BMS 재생, 게임의 모든 조건 조합이나 롱노트 시간 진행을 재현하는 기능은 아니다.

묶음 출력에는 `index.html` 갤러리, 정규화한 `states.json`, 전체 `report.json`, 상태별
PNG와 진단 JSON이 있다. 상태 하나가 실패해도 다른 상태 검사를 계속하고 실패를
갤러리/보고서에 남긴다. 하나라도 실행 실패하면 최종 exit 1이다. `ok:true`는 렌더 요청이
성공했다는 뜻이며 진단 이슈가 없거나 화면이 올바르다는 보증이 아니다.

## “왜 안 보이지?” 진단

```powershell
python -B tools/skin_agent.py --editor SkinEditor_DX9/Release/SkinEditor_DX9.exe diagnose .build/my-first-agent-skin/LR2files/Theme/Generated/skin.lr2skin
python -B tools/skin_agent.py --editor SkinEditor_DX9/Release/SkinEditor_DX9.exe diagnose .build/my-first-agent-skin/LR2files/Theme/Generated/skin.lr2skin --state .build/preview-state.json --object preset_background
```

`--object ID`로 좁히거나 ID가 없는 원본은 `--object-index`에 `inspect`의 0부터 시작하는
index를 준다. index는 현재 로드의 위치이므로 레시피의 장기 식별자로 저장하지 않는다.
진단은 원본 소유 파일의 `file`, 1부터 시작하는 `line`(확인 불가 시 null), `command`,
`code`, `severity`, `hint`, `scope`, `confirmed_hidden`, `branch_active`를 반환한다.
include가 있으면 합친 임시 파일의 행 번호가 아닌 해당 원본 파일 위치를 사용한다.

이미지 누락·읽기 실패·잘린 SRC 영역, 비활성 IF/타이머/조건, 애니메이션 전후,
투명도와 0 크기, 화면 밖/부분 잘림 등을 검사한다. 이미지 메타데이터 검사는 비활성
선택 분기도 포함하므로 `scope: all_branches` 경고만으로 현재 요소가 안 보인다고
판정하지 않는다. 옵션과 타이머는 로드된 네이티브 상태를 사용한다. blend 0은 알파를
무시하므로 a=0만으로 숨김 판정을 하지 않는다.

Object의 `visibility`는 다음 의미다.

- `hidden`: 이 Preview 상태에서 수집된 모든 DST 타임라인의 숨김 근거가 있다.
- `unknown`: 특수 명령·공유 SRC/DST·회전·상대 좌표 등으로 일반 검사만으로 확정할 수 없다.
- `not_proven_hidden`: 검사에서 숨김을 입증하지 못했다. 실제 픽셀이 보인다는 보증은 아니다.

다른 요소에 가려짐, 텍스처 자체의 투명도, 소스 변형과 장면별 선택 로직은 PNG로
함께 확인해야 한다. 진단은 원본을 수정하거나 조건·타이머를 자동으로 고치지 않는다.

## 출력과 파일 보호

- `LR2files/Theme/Generated/skin.lr2skin`, `preset.png`: 기본 LR2 스킨과 아틀라스.
- `agent-atlas-000.png` 등: PNG 등록 시 Generated 폴더에 추가하는 아틀라스 페이지.
- `assets.json`: 등록한 이미지의 페이지/gr/사각형/분할 정보, 바인딩과 새 Object ID.
- `recipe.json`, `objects.json`, `build-report.json`: 정규화 레시피, 네이티브 명령/값, 생성 결과.
- `preview.png`: build의 Preview. `--no-preview`이면 생략한다.

기존 출력 폴더와 PNG는 거부하며 덮어쓰기 옵션은 없다. 생성/상태 묶음은 새 staging에서
진행해 새 폴더로 옮기고, 원본 스킨과 PNG는 읽기만 한다. PNG 등록은 새 단일 파일 preset에
적용하며 임의 기존 스킨의 include/IF 문서를 다시 쓰는 기능은 제공하지 않는다.

## 구현과 검증

`agentUtility.cpp`는 기존 preset 생성기와 WORKSPACE parser/Preview를 연결하고,
`agentDiagnostics.cpp`는 기존 네이티브 조건·시간·이미지 진단과 소유 행을 사용한다.
`skin_agent.py`는 레시피/출력 검증, `skin_agent_assets.py`는 PNG 등록,
`skin_agent_preview.py`는 고정 상태와 갤러리를 담당한다.

```powershell
powershell -ExecutionPolicy Bypass -File scripts/test-agent.ps1
python -B -m unittest discover -s tests -p 'test_skin_agent*.py' -v
```

`test-agent.ps1`은 레시피·인코딩·아틀라스 픽셀·바인딩·상태·진단 검사와 실제 네이티브
생성→검사→렌더, 출력 보존/실패 정리 검사를 실행한다. 둘째 명령은 `LR2_AGENT_EDITOR`
환경 변수가 없으면 네이티브 통합을 생략한다. 편집기 자체 검사는 `scripts/test.ps1`로
별도 실행한다. 테스트 수와 최종 빌드/실행 결과는 해당 변경의 검증 기록을 따른다.
자동 테스트는 사용자 키보드/마우스를 조작하지 않는다. 실제 LR2 실행/플레이와 외부
스킨 전체의 수동 회귀 검증은 별도로 필요하다.

2026-09-12 로컬 검증: Release Win32 빌드, 유틸 검사 26개(실제 네이티브 통합 포함),
편집기 자체 검사 17개를 통과했다. 실행 묶음으로 PNG 바인딩/추가 Object 33개를
생성하고 BLOCKBEAT의 14개 고정 상태를 렌더링했다. 동일 상태의 픽셀 재현성,
원본 파일 보존, include 원본 행, 한글 폴더 경로와 최대 자릿수 잘림 경고를 확인했다.
미리보기 성공과 실제 LR2 플레이 검증은 구분한다.

범위 밖: 이미지 생성 AI/API 연결, MCP 서버, 자연어 자동 수정, 기존 스킨의 in-place
편집, 자동 설치/배포와 상시 실행 에이전트.

## 테마 제작 예제

[BLOCKBEAT — OVERWORLD](../examples/agent/blockbeat/README.md)는 7키 + 스크래치용
마인크래프트풍 테마다. 예제 전용 제작 스크립트가 기본 스킨에 일러스트와 픽셀 UI/노트
아틀라스를 붙인다. 위 PNG 레시피 예제는 이 일러스트를 범용 등록 기능으로 연결하는
작은 예시이며 BLOCKBEAT의 전체 UI/노트 구성을 복제하지 않는다. 실제 LR2 플레이
검증은 별도로 필요하다.
