# AI 스킨 제작 유틸 v0.1

외부 AI가 JSON 제작 지시를 작성하고 명령줄에서 스킨을 생성·검사·렌더링하는 도구다.
이 프로그램 자체는 언어 모델이나 API를 호출하지 않는다. `AI_experimental`에서만
개발하며, `AI_2`의 텍스트 자동완성/검증 변경을 기반으로 한다.

첫 버전은 편집기의 기존 preset 생성기를 사용해 스킨과 atlas를 만든다. AI는 기존
Object ID의 배치·크기·색을 명시적으로 지정하고 결과를 다시 읽어 다음 지시를 만들 수
있다. 생성물은 일반 LR2 CSV/PNG이며 OLRskin 0.9 계약은 변경하지 않는다.

## 실행

Windows, Python 3.10+, Direct3D 9가 필요하다. 별도 pip 패키지는 없다.
소스 checkout에서 먼저 `scripts/build.ps1`을 실행한 뒤:

```powershell
python tools/skin_agent.py --editor SkinEditor_DX9/Release/SkinEditor_DX9.exe schema --command '#DST_NOTE'
python tools/skin_agent.py --editor SkinEditor_DX9/Release/SkinEditor_DX9.exe build examples/agent/blue-play7.json --out .build/my-first-agent-skin
python tools/skin_agent.py --editor SkinEditor_DX9/Release/SkinEditor_DX9.exe inspect .build/my-first-agent-skin/LR2files/Theme/Generated/skin.lr2skin --object preset_note_0
python tools/skin_agent.py --editor SkinEditor_DX9/Release/SkinEditor_DX9.exe render .build/my-first-agent-skin/LR2files/Theme/Generated/skin.lr2skin --out .build/second-preview.png
```

이 작업에서 제공한 실행 묶음은 `.build/agent-utility-win32-20260912`다. 그 폴더에서는
`skin-agent.cmd`가 Python 도구와 실행 파일 경로를 함께 지정한다.

```powershell
.\skin-agent.cmd schema --command '#DST_NOTE'
.\skin-agent.cmd build .\examples\blue-play7.json --out .\my-first-agent-skin
```

`--editor`는 반드시 이 브랜치의 새 실행 파일을 가리켜야 한다. `schema`와 `inspect`는
GUI 없이 동작한다. `build`와 `render`는 숨겨진 네이티브 graphics 환경을 사용한다.
각 요청의 실행 제한은 60초이며, 실패는 exit 1과 `{"ok":false,"error":"..."}`로 반환한다.
잘못된 명령줄 인자는 argparse의 exit 2를 사용한다.

## AI 작업 순서

1. `schema`에서 네이티브 명령/열 정의를 읽는다. `--values`는 기존 심볼 값 표도 반환한다.
2. Object 수정 없이 기본 레시피로 `build --no-preview`를 실행하고 `inspect`로 ID와 값을 읽는다.
3. 필요한 `objects[].set`을 작성하고 **다른 새 출력 폴더**로 build한다.
4. `objects.json`, `build-report.json`, `preview.png`를 읽고 의도한 배치/색인지 확인한다.
5. 필요하면 레시피를 수정해 새 폴더에 반복한다. 설치와 기존 사용자 스킨 교체는 별도 작업이다.

## JSON 제작 지시

```json
{
  "version": 1,
  "scene": "play7",
  "width": 1280,
  "height": 720,
  "title": "AI Blue Play 7K",
  "maker": "SkinEditor experimental",
  "objects": [
    {"id": "preset_background", "set": {"r": 170, "g": 205, "b": 255}},
    {"id": "preset_note_0", "set": {"r": 255, "g": 205, "b": 205}}
  ]
}
```

| 필드 | 계약 |
|---|---|
| version | 제작 지시 버전 1. OLRskin 포맷 버전과 별개다. |
| scene | play7, play5, double14, double10, pms9, select, decide, result, battle7, battle5, battle9, course-result |
| width / height | 각각 320..1920, 240..1080의 정수. 기본 1280×720. |
| title / maker | 1..120자, CP932로 손실 없이 저장 가능해야 하며 쉼표/개행/NUL은 거부한다. |
| objects | 최대 512개. `id`와 `set`만 허용하고 ID 중복/미등록 ID를 거부한다. |
| set | x/y: -8192..8192, w/h/size: 1..8192, a/r/g/b: 0..255, angle: -3600..3600. 모두 정수. |

`set`은 해당 Object의 **모든 DST keyframe**에서 지정한 값을 대입한다. 배치 애니메이션을
상대 이동하는 명령이 아니다. 지정하지 않은 값과 명령/행 순서는 유지한다. 필드 위치는
네이티브 schema를 사용하며, 등록되지 않은 필드는 임의로 추정하지 않고 거부한다.
완료 뒤에는 네이티브 Object 모델을 다시 읽어 지정한 값과 Object 수를 확인한다.

## 출력과 파일 보호

- `LR2files/Theme/Generated/skin.lr2skin`, `preset.png`: 실제 LR2 스킨과 기본 atlas.
- `recipe.json`: 기본값을 포함한 제작 지시.
- `objects.json`: 네이티브 parser의 Object/명령/값 목록.
- `preview.png`: 기존 편집기 Preview renderer의 정적 이미지. `--no-preview`이면 생략한다.
- `build-report.json`: 성공 여부, 파일 위치, Object 수와 parser 검사 여부.

기존 출력 폴더와 PNG는 거부하며 덮어쓰기 옵션은 없다. 제작은 별도 staging 폴더에서
수행하고 검증 후 새 폴더로 옮긴다. 잘못된 지시나 native 실패 시 도구가 만든 staging만
정리한다. 실제 LR2 설치 경로나 원본 스킨에는 쓰지 않는다. Preview의 BGA/노트/숫자는
편집기 샘플 상태이며 실제 곡 재생 화면과 구분한다.

## 구현 및 검증

`agentUtility.cpp`는 `GetCommandHelp`, 기존 `BuildInitialPreset`, WORKSPACE parser와
Preview를 연결하는 명시적 CLI 진입점이다. `skin_agent.py`는 레시피 검증, 새 출력 폴더의
CSV 수정과 결과 검사를 담당한다. GUI 상태나 History의 별도 사본을 만들지 않는다.

```powershell
powershell -ExecutionPolicy Bypass -File scripts/test-agent.ps1
python -B -m unittest discover -s tests -p test_skin_agent.py -v
```

첫 명령은 실제 native 통합을 포함한 7개 검사다. 둘째는 `LR2_AGENT_EDITOR` 환경 변수가
없으면 native 통합 1개를 생략하고 단위 검사 6개만 실행한다. 기존 편집기 자체 검사
17개는 `scripts/test.ps1`로 별도 실행한다. 테스트는 사용자 키보드/마우스를 조작하지 않는다.

이번 샘플 `blue-play7`은 32개 Object 생성, native 수정 값 readback, 1280×720 PNG 생성을
확인했다. 기존 출력 거부와 실패 시 임시 폴더 정리도 통합 검사했다. 기존 LR2/DxLib
컴파일 경고는 남아 있다. 실제 LR2 실행/플레이와 모든 장면의 시각 검증은 미완료다.

이미지 생성 AI 연결, 외부 PNG를 새 Object로 추가하는 기능, 임의 기존 스킨의 in-place
편집, 자동 설치/배포와 상시 실행 에이전트는 현재 버전에 포함하지 않는다.
