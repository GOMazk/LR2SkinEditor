# SkinEditor 현재 개발 상태

기준일: 2026-08-27
기준 브랜치: `AI_2`
주 대상: `Release | Win32(x86)`

이 문서는 지금까지 진행한 작업의 의도와 현재 구현 상태를 다음 작업자가 코드와
함께 확인할 수 있도록 기록한다. 완료 표시는 현재 코드에 구현되어 있다는 뜻이며,
모든 외부 스킨에 대한 수동 회귀 검증이 끝났다는 뜻은 아니다.

## 1. 제품 목표

SkinEditor는 LR2 스킨 스크립트를 단순 CSV 표가 아니라 편집 가능한 Object로
다루는 것을 목표로 한다. 한 CSV 행은 여전히 저장의 원본이며 Object Model은
그 행을 해석해 Browser, Inspector, Preview, Image Manager와 DST View가 공유하는
파생 모델이다.

### 시나리오 A: 기존 PLAY 스킨 편집

사용자가 5/7/9/10/14 keys 및 battle PLAY 스킨을 연 뒤:

1. 원하는 해상도를 정의한다.
2. FAST/SLOW를 포함한 일반 Object를 만들거나 배치한다.
3. Preview에서 위치와 조건 상태를 확인한다.
4. 저장 또는 Save As 한다.

현재 코드상 가능한 항목:

- 스킨을 열 때 `#INFORMATION` 해상도가 없으면 `#RESOLUTION`, 전체 include가
  펼쳐진 뒤의 TenRiff LR2 자동 판정, 640x480 fallback 순서로 Preview 해상도 결정
- Open 이후 toolbar의 현재 해상도 버튼에서 해상도를 바꾸고 즉시 원본 반영
- Object/Command 생성, 이름 지정, SRC/DST 속성 편집
- FAST/SLOW를 특별 예외가 아닌 NUMBER Object로 취급
- `$num/$type/$timer/$op/$st` 이름을 ID 또는 이름으로 검색하는 ComboBox
- Asset crop을 Preview에 놓은 뒤 IMAGE/NUMBER/SLIDER/BUTTON 중 Object 종류 선택
- Preview에서 선택 Object 이동 및 키보드 미세 이동
- DST 애니메이션 프레임 증감
- Ctrl+Z 기반 CSV/구조 편집 복구
- 현재 include 구조를 유지하는 Save/Ctrl+S와 `SAVED/MODIFIED` 상태 표시
- Windows 파일 선택기를 쓰는 Save As와 성공 후 새 메인 스크립트로 작업 경로 전환
- `File > Open another location` 또는 Skin Browser에서 지정한 외부 폴더의
  `.lr2skin`/`.lr2ss`를 하위 폴더까지 찾아 기존 로더로 열기. `LR2files`
  기준 경로는 소스 CSV를 바꾸지 않고 실제 LR2 트리 또는 standalone 스킨
  폴더로 가상 해석하여 include, image, custom file/folder, font, helpfile을
  Preview 로드 경계에서 사용
- `File > Export OLR package`에서 현재 Object를 의미별로 분류하고 LR2 호환
  스크립트, 고정 이미지와 해결된 LR2 가상 root를 한 `.olrskin`
  파일로 묶기
- `File > Import OLR package`에서 패키지를 검증한 뒤 사용자가 고른 위치의 새
  폴더에만 풀고, 추출한 `main.lr2skin`을 일반 Workspace로 열기
- Import한 V0.2 workspace의 `File > Export LR2 folder`에서만 `vfs/LR2files`를
  새 출력 폴더의 `LR2files`로 풀고 현재 편집 스크립트의 경로를 복원

주의할 항목:

- TenRiff 규칙으로 추정한 해상도는 workspace/Preview에만 적용하며 원본 CSV에는 자동
  기록하지 않는다. toolbar와 status bar의 `inferred` 표시로 명시 해상도와
  구분하고, 사용자가 해상도 modal의 Apply를 선택했을 때만 `#INFORMATION`에 쓴다.
  최종 visible `#DST_NOTE`의 최대 x와 좌상단에 고정된 큰 `#DST_IMAGE` backdrop을
  함께 판정하여 640x480/1280x720/1920x1080 중 하나로 정규화한다. 화면 밖에 둔
  전환 panel은 backdrop으로 보지 않는다. 이 판정은 `skinResolution.cpp`에 격리하며
  공용 LR2 parser의 명령 처리에는 결합하지 않는다.
- 해상도 변경의 즉시 저장은 의도된 동작이다. Workspace 해상도 modal은 이 점과
  기존 Object 좌표를 자동 scale하지 않는다는 점을 명시한다. 재로드 중 편집 내용을
  잃지 않도록 일반 script 변경은 먼저 Save해야 하고 미저장 Pixel paint도 정리해야 한다.
- Save As는 스크립트 경로를 전환하지만 이미지, 폰트 등 리소스 파일을 복제하지
  않는다. 정상 Save As 흐름이 있으므로 별도 Clone 기능은 현재 필수 기능이 아니다.
- 실제 스킨별 좌표·경로·조건 조합은 수동 회귀 테스트가 필요하다.

Skin Browser는 현재 탐색 위치와 검색 결과 수를 표시한다. 외부 위치에서는 Refresh와
Default locations 전환을 제공하며, 파일을 복사하거나 LR2 설정에 등록하지 않고 원래
경로에서 직접 연다. 재귀 탐색은 junction/symlink를 따라가지 않는다. 기존 LR2/CSTR
로더의 제약 때문에 Windows 현재 ANSI 코드 페이지로 표현할 수 없거나 `MAX_PATH`를
넘는 경로는 잘라서 열지 않고 오류 또는 건너뜀 상태로 표시한다.
외부 스킨의 `LR2files\Theme\<name>` 선언은 각 행의 owner include와 main 경로
조상에서 `<name>` 폴더를 찾는다. wildcard는 펼친 파일명으로 바꾸지 않으며
기존 LR2 random/custom 선택이 그대로 실행된다.
목록의 해상도 메타데이터도 공용 LR2 parser에 덧붙이지 않고 `skinResolution.cpp`가
해당 top-level 파일을 읽어 보정한다. 실제 Open 시에는 전체 include가 펼쳐진 뒤 다시
판정하므로 Browser의 빠른 메타데이터 판정과 Workspace의 최종 판정을 구분한다.

### 시나리오 A-2: OLR 중간 포맷으로 공유

`.olrskin` V0.2는 LR2 CSV를 버리는 새 저장 형식이 아니라, AI와 사람이 구조를
찾기 쉬운 Semantic index와 원본 동작을 보존하는 Compatibility layer를 함께 담는
ZIP 컨테이너다. 자세한 계약은 [OLR 포맷 문서](OLRSKIN_FORMAT.md)를 따른다.

```text
loaded WORKSPACE
  +-- Object Model -> skin.json (gear/notes/judge/combo/gauge/...)
  +-- expanded CSV -> lr2/main.lr2skin
  +-- resolved LR2 roots -> lr2/vfs/LR2files/*
  +-- other fixed #IMAGE -> lr2/assets/*
  +-- row ownership -> compatibility/source-map.json
  `-- virtual/export map -> compatibility/path-map.json
```

V0.2의 `skin.json`은 설명용이다. Export 후 JSON만 직접 수정해도 LR2 스크립트가
자동 생성되지는 않으며, Import는 `lr2/main.lr2skin`을 기준으로 연다. 이 경계는
아직 해석하지 못한 LR2 명령, 조건, timer, op, editor metadata를 잃지 않기 위한
의도된 단계다. Semantic-to-LR2 컴파일은 각 영역의 왕복 테스트를 갖춘 뒤 한 영역씩
승격한다.

패키지에는 로컬 절대 owner 경로를 기록하지 않는다. main 폴더 내부 include는
상대 경로, 외부 include는 `<external>/<filename>` label만 source map에 남긴다.
Import는 archive path traversal, 암호화/지원하지 않는 압축, CRC 오류를 거부하고
기존 폴더를 덮어쓰지 않는다. 해결된 logical root 안의 wildcard/custom file,
font와 archive는 폴더 전체로 보존한다. 해결할 수 없는 LR2 root, 외부 경로,
과도하게 긴 파일은 외부/누락 개수를 결과와 manifest에 명시한다.
V0.1 패키지는 계속 Import할 수 있지만 path map이 없으므로 LR2 folder Export를
활성화하지 않는다.

### 시나리오 B: 새 스킨 생성

New에서 Scene과 해상도를 선택하여 필수 Object가 포함된 시작 프리셋을 만든다.

현재 프리셋 대상:

- PLAY: 7KEYS, 5KEYS, 14KEYS, 10KEYS, 9KEYS
- BATTLE: 7KEYSBATTLE, 5KEYSBATTLE, 9KEYSBATTLE
- SELECT
- DECIDE
- RESULT

New는 `LR2files\Theme` 아래에 새 `.lr2skin`과 `preset.bmp`를 만든 후 그 스킨을
즉시 연다. `..`, 드라이브명 같은 범위 이탈 경로는 거부하고 기존 파일은
덮어쓰지 않는다.

PLAY 프리셋은 배경, BGA, 키 모드별 Note/Mine/LN, Judge Line, Groove Gauge,
FAST/SLOW를 만든다. SELECT는 선택 바와 제목, DECIDE는 패널/강조/플래시,
RESULT는 패널/점수 숫자/게이지 및 스코어 차트를 포함한다.

향후 프리셋 확장 시 `BuildInitialPreset()`에 Scene별 생성기를 추가하되,
`$SE_OBJECT_NAME`과 고유한 `$SE_OBJECT_ID`를 함께 생성해야 한다.

## 2. 스키마와 Object Model

### 명령 스키마

`skinHelper.txt`는 각 명령과 열의 의미를 정의한다.

```text
#SRC_IMAGE,index,gr,x,y,w,h,...
                 ^ 각 필드의 편집기용 schema
```

UI는 `arr_CommandHelp`를 직접 탐색하지 않고 `GetCommandHelp(command, column)`을
통해 열 설명을 얻는 것을 원칙으로 한다. `$type`, `$op`, `$st`, `$num`,
`$timer`로 시작하는 필드는 숫자 입력 대신 이름이 표시되는 ComboBox로 그린다.
이 규칙은 Object Inspector, TextEdit와 New Object 창에 동일하게 적용한다.

`$type`은 하나의 공통 번호 공간이 아니라 명령별 번호 공간이다.
`GetCommandValueKind()`가 command와 schema를 함께 판정하고 다음 함수를 선택한다.

| Command/schema | 표시 함수 |
|---|---|
| `#INFORMATION,$type` | `SKINTYPESTR` |
| `#SRC_NUMBER,$num` | `numberName()` |
| `#SRC_SLIDER,$type` | `sliderName()` |
| `#SRC_BUTTON,$type` | `buttonName()` |
| `#SRC_BARGRAPH,$type` | `bargraphName()` |
| `#SRC_TEXT,$st` / `#SRC_BAR_TITLE,$st` | `textName()` |

새 symbolic 필드를 추가할 때 UI 세 곳에 분기를 복사하지 말고 공통 resolver와
ComboBox renderer를 확장한다. 공통 ComboBox popup 상단에는 ID/이름 검색 입력이
있으며 `Fast`, `Slow`, `900`처럼 번호나 영문 symbolic 이름으로 목록을 좁힌다.

### Object 스키마

`skinObjGroup.txt`는 어떤 SRC/DST 명령 조합이 한 Object 종류인지 정의한다.
`SEObjectEditorModel::Rebuild()`가 CSV 행을 스키마와 결합해
`SEObjectInstance` 목록을 만든다.

주요 규칙:

- 명령 도움말에서 1번 열이 명시적으로 `index`인 그룹은 `(ifgroup, index)`로
  행을 묶는다.
- NOTE 계열처럼 도움말이 불완전해도 여러 SRC/DST 명령 종류가 동일 숫자 인덱스를
  공유하면 indexed group으로 취급한다.
- index가 없는 그룹은 각 SRC부터 다음 SRC 전까지의 DST를 한 Object로 묶는다.
- `#IF/#ELSEIF/#ELSE/#ENDIF`는 Object 행에 들어가지 않고 branch 문맥만 제공한다.
- 큰 HD 스킨을 열 때 그룹마다 전체 CSV를 재검색하지 않도록 명령 인덱스를 먼저
  만들고 그룹별 후보 행을 구성한다.

### 편집기 메타데이터

다음 행은 LR2 명령이 아니라 SkinEditor가 Object를 안정적으로 식별하기 위한
메타데이터다.

```text
$SE_OBJECT_NAME,FAST
$SE_OBJECT_ID,obj_001_fast
#SRC_NUMBER,...
#DST_NUMBER,...
```

- `$SE_OBJECT_ID`: rebuild와 행 삽입/삭제 후에도 같은 Object를 다시 찾는 식별자
- `$SE_OBJECT_NAME`: 사용자가 지정한 표시 이름
- `$SE_GROUP_BEGIN`, `$SE_GROUP_MEMBER`, `$SE_GROUP_END`: 사용자 Object 그룹

Object 삭제 시 바로 앞에 붙은 NAME/ID 메타데이터도 함께 삭제한다. 자동 표시
이름은 CSV에 쓰지 않는다.

Browser 표시 이름의 우선순위:

1. 명시적인 `$SE_OBJECT_NAME`
2. SRC의 command-specific symbolic 값: NUMBER, SLIDER, BUTTON, BARGRAPH, TEXT
3. 위 이름이 없는 일반 Object는 활성 조건을 설명하는 서로 다른 `$op` 이름
4. `$op` 이름도 없으면 서로 다른 non-zero `$timer` 이름

각 SRC symbolic 이름은 ComboBox와 동일한 `numberName()`, `sliderName()`,
`buttonName()`, `bargraphName()`, `textName()` resolver 결과를 사용한다.
`MainTimer` 0은 대부분의 Object에 존재해 구분력이 없으므로 timer 자동 이름에서
제외한다. op/timer fallback은 표시 전용이며 파일과 History를 변경하지 않는다.

## 3. 조건 분기

Object Browser의 조건 구조는 다음을 유지한다.

```text
ConditionBlock
  +-- #IF 293
  |     +-- Object ...
  +-- #ELSEIF 294
  |     +-- Object ...
  +-- #ELSE
        +-- Object ...
```

- `#IF`, `#ELSEIF`, `#ELSE`는 한 ConditionBlock 아래의 형제 Branch다.
- `#IF`와 `#ELSEIF`는 원본 CSV의 조건 파라미터를 헤더에 표시한다.
- 인자가 없는 `#ELSE`는 그대로 표시한다.
- 중첩 IF는 현재 Branch의 자식 ConditionBlock으로 표시한다.
- 파싱 시 sibling branch는 같은 논리 IMAGE 번호 범위를 재사용한다. 중첩 IF가
  사이에 있어도 outer `#ELSEIF/#ELSE`를 단순 인접 인덱스로 가정하지 않는다.
- Branch 순서는 원본 행 순서를 기준으로 `stable_sort`한다.
- 빈 Branch에서도 우클릭으로 Object/Command를 만들 수 있다.
- `#ELSE` Branch 툴팁은 같은 ConditionBlock의 앞선 `#IF/#ELSEIF` 조건을
  모두 표시한다. 따라서 `Active objects only`가 비활성 형제를 숨겨도 ELSE의
  조건 문맥을 확인할 수 있다.

`Active objects only`는 DST가 존재하고, 문서화된 모든 DST `$op`가 성립하며,
현재 IF chain에서 실제 선택된 Branch에 속한 Object만 남긴다. 같은 판정이 Browser
행의 연한 초록(활성)/연한 빨강(비활성) 배경에도 사용된다.

DST View는 조건 Branch 활성 여부와 무관하게 선택한 SRC/DST의 텍스처를 확인하는
검사용 도구다. Preview와 Object Browser의 “현재 활성” 의미와 혼동하지 않는다.

## 4. Object Browser와 Object Inspector

기존 Object Editor의 좌우 영역은 독립 도킹 창으로 분리되었다.

- **Object Browser**: Type/사용자 그룹/Search/활성 Object 필터, 조건 트리,
  다중 선택, 생성·삭제 Context menu
- **Object Inspector**: Name, Tagged image, SRC, DST와 애니메이션 프레임 속성

두 창은 보이기 상태만 독립적이며 선택 상태를 복제하지 않는다. 구형 코드가
`wObjectEditor = true`를 요청하면 Browser와 Inspector를 함께 연다.
Browser의 Type/Group/Search/Active 필터 영역은 네 control이 모두 들어가는
계산 높이를 사용하고 내부 scrollbar를 만들지 않는다.

Browser 선택:

- 클릭: 단일 선택
- Ctrl+클릭: 선택 추가/제거
- Shift+클릭: anchor부터 범위 선택
- Preview/DST View에서 들어온 선택: 필터를 해제하고 해당 행으로 자동 스크롤
- Delete: 활성 Object의 종류와 이름을 확인하는 modal을 거친 뒤 삭제한다. 입력
  field나 다른 modal이 활성화된 동안에는 Delete shortcut을 받지 않는다.
- 우클릭 `Remove Object`도 같은 확인 modal과 CSV/History 삭제 경로를 사용한다.

Browser 순서 변경:

- Object 행을 드래그해 다른 Object의 위/아래에 놓으면 원본 CSV 순서를 바꾼다.
- SRC/DST가 떨어져 있는 indexed Object도 한 묶음으로 이동하며, 바로 앞의
  `$SE_OBJECT_NAME`과 `$SE_OBJECT_ID`도 함께 이동한다.
- include 저장 소유권과 조건 의미가 암묵적으로 바뀌지 않도록 같은 원본 파일,
  같은 IF/ELSEIF/ELSE Branch 안에서만 드롭할 수 있다.
- 파란 삽입선은 유효한 대상, 빨간 삽입선은 허용되지 않는 대상을 뜻한다.
- 재정렬은 문서 순서 snapshot 한 건으로 History에 기록되어 Ctrl+Z 한 번으로
  복구된다. 적용과 복구는 다음 frame 시작에서 이루어져 Preview/ImageManager가
  이전 행/texture 인덱스를 참조하지 않게 한다.

### New Object / New Command

우클릭 메뉴에서 둘 다 유지한다.

- New Object는 `skinObjGroup.txt`에 포함된 Object 명령만 표시한다.
- New Command / Setting은 header, condition, setting 명령까지 표시한다.
- Object 생성 창에는 Name 입력이 있으며 값이 있으면 `$SE_OBJECT_NAME`을 쓴다.
- NUMBER/SLIDER/BUTTON/BARGRAPH/TEXT의 Name은 사용자가 직접 편집하기 전까지
  각각 `$num/$type/$st`의 symbolic 이름을 자동으로 따른다. 직접 입력한 이름은
  이후 symbolic 값을 바꿔도 덮어쓰지 않는다.
- 모든 `$` schema 필드는 ComboBox다.
- SRC Object 생성 시 대응 DST 명령이 스키마에 있으면 기본 DST 하나를 함께 만든다.
- `#IF` 생성은 선택 Object를 새 조건으로 감쌀 수 있고, 빈 위치에서는
  `#IF/#ENDIF` 뼈대를 만든다.

### DST 프레임

- `+ DST`: 마지막 DST 행을 복제하여 애니메이션 프레임을 추가한다.
- `- DST`: 마지막 DST를 삭제하지만 최소 1개는 반드시 유지한다.
- DST가 0개인 Object에서는 `+ DST`가 비활성이다. 먼저 유효한 DST 명령을
  생성해야 한다.
- 탭의 내부 ID를 고정해 개수가 바뀌어도 `DST (n)` 탭이 계속 열린다.
- rebuild 후 `$SE_OBJECT_ID`를 우선 사용해 같은 Object 선택을 복원한다.
- 여러 DST는 열 방향 표로 표시하며 첫 DST와 속성명은 고정한다.
- loop/timer/op 필드는 첫 DST가 대표하며 뒤 animation frame에서는 반복 표시하지
  않는다.

## 5. Preview, Image Manager, DST View

### PLAY 장면 시뮬레이터

PLAY 계열 Preview는 **Timer Control**의 `Restart scene`으로 시작한다. 기본
`Simple`은 현재 키 모드에 맞는 LR2의 기존 sample BMS/PMS를 동기 파싱하고,
`Full`은 시간순 `LaneStruct`/`NoteStruct` chart를 메모리에서 만든다. 두 경로 모두
LR2 원본 `ProcI_Play`와 `DrawNotes`가 소비한다. 편집기 코드가 노트 좌표나
judge/combo 상태를 직접 만들지 않으므로 판정 시 `ApplyJudgeNote`가 설정하는
lane별 50/100/120 타이머를 key beam, note explosion, judge/combo를 포함한 실제
스킨 Object가 LR2와 같은 방식으로 소비한다. 오디오만 의도적으로 비활성 상태다.
시뮬레이터 실행 상태는 각 WORKSPACE가 소유하므로 여러 스킨 탭 사이에서 공유되지
않고, 실행 중 Object 편집으로 Preview가 재구성되면 장면도 안전하게 재시작한다.
선택한 chart가 끝나면 동일한 scene init을 거쳐 반복한다.

Timer Control의 `Simple / Full`은 CSV에 저장하지 않는 Workspace Preview 설정이다.
기본 `Simple`은 LR2의 기존 `sample_5/7/10/14.bme`, `sample_9.pms`를 사용해
예전의 성긴 Preview를 그대로 재생한다. 해당 파일이 없는 독립 배포 환경에서는
16개짜리 내장 패턴으로 안전하게 대체한다. `Full`은 기존 180개 메모리 패턴으로
LN, mine, 동시치기와 judge 효과를 집중 검사한다. 모드를 누르면 현재 장면이 즉시
재시작되며 두 모드 모두 LR2 원본 재생 경로와 Rhythm 140 event를 사용한다.

### 확대/축소

Preview, Image Manager, DST View 모두 포인터가 canvas 위에 있을 때
Ctrl+MouseWheel로 확대/축소한다.

- Preview: 25%~1600%
- Image Manager/DST View: 5%~1600%
- Image Manager/DST View의 초기값 0은 “0%”가 아니라 첫 draw에서 Fit 계산을
  요청하는 sentinel이다.
- Image Manager/DST View에는 Fit과 100% 버튼이 있다.

### Preview 선택과 이동

- Object를 선택하면 첫 DST와 마지막 DST 경계를 점멸 사각형으로 표시한다.
- 시작 경계는 cyan, 최종 경계는 red다.
- 선택한 Object 위에서 좌클릭 drag하면 해당 Object의 모든 선택 대상 DST 좌표를
  이동한다.
- 방향키 이동도 동일 CSV 편집/History 경로를 사용한다.
- Ctrl+Z 후 Preview object와 점멸 사각형이 어긋나지 않도록 모델과 Preview의
  파생 데이터를 함께 invalidate/rebuild한다.

Preview 캔버스에는 Object 확인·선택·배치 기능만 둔다. scene restart와 timer 조작은
독립된 **Timer Control** 도킹 창에 둔다. 이 창은 scene runtime
재시작과 OpList 형식의 timer 0~199 체크 목록을 제공한다. 체크하면 해당 timer를
시작하고 체크를 해제하면 리셋하며, 실행 시간은 항목 tooltip에서 확인할 수 있다.
runtime 상태인 timer는 기본 배경으로 표시한다. 사용자가 직접 시작하거나 reset한
timer는 체크박스 배경을 빨간색으로 표시하므로 수동 ON과 수동 OFF를 모두 구분할 수
있다. scene restart 시 수동 timer 표시도 함께 초기화한다.
Timer Control은 workspace가 skin을 성공적으로 연 뒤에만 생성되며,
`Windows > Workspace > Timer Control`에서 다시 열 수 있다.

OpList도 scene이 계산한 option은 기본색으로 표시하고, 사용자가 값을 바꾼 option은
runtime override로 유지한다. 사용자 override가 있는 option은 체크 여부와 관계없이
체크박스 배경이 빨간색이며 scene restart 시 자동 계산 상태로 돌아간다.

PLAY Preview는 별도 `ProcGameThread`를 만들지 않는다. UI thread의 scene tick에서
`ProcGame`을 정확히 한 번 실행한 뒤 PLAY Object를 그린다. `Full` 메모리 chart와
`Simple`의 16-note fallback은 `bmsobj_note`뿐 아니라 LR2 event queue인 `bmsobj`에도
measure event와 종료 sentinel을 넣는다. `Full`에서는 Rhythm timer 140이 BPM 150에
따라 증가하고 1.6초마다 measure event에서 0으로 돌아가며, `Simple` sample은 파일에
기록된 BPM과 measure event를 그대로 따른다. `flag_gameinput`은 legacy Draw loop가 drawing
buffer마다 `ProcGame`을 중복 호출하지 않도록 꺼진 상태를 유지한다.
PLAY Preview의 기본 HI-SPEED는 200이며 LR2 기본 허용 범위 10~900 안에서 동작한다.

Preview 우클릭은 해당 좌표와 겹치는 Object 중 현재 op와 IF Branch가 성립하는
Object만 목록에 표시한다. 각 항목은 Object 모델의 대응 SRC 명령과 index를 찾아
thumbnail을 만든다. 항목 hover 시 얇은 노란 점멸 사각형으로 위치를 표시하고,
클릭 시 Browser/Inspector/DST View 선택을 동기화한다.

### 이미지 번호와 텍스처 선택

LR2의 논리 `gr` 번호는 조건 Branch마다 다른 `#IMAGE` 파일에 대응할 수 있다.
따라서 `gr == 배열 인덱스`라고 가정하면 Image Manager와 thumbnail이 다른 숫자
atlas를 보여주는 문제가 생긴다.

현재 규칙:

- `SRCGR.grID`가 논리 gr 번호다.
- 같은 grID를 공유하는 모든 `#IMAGE` 선언은 별도 `SRCGR` 후보로 보존한다.
- Image Manager는 선택한 IMG의 `ifGroup`과 같은 후보를 우선 선택하고, 상단
  ComboBox에서 같은 grID의 다른 텍스처 파일을 볼 수 있다.
- Preview/thumbnail은 SRC 선언의 Branch와 custom file 선택이 맞는 텍스처를
  우선한다.
- DST View는 검사 목적상 Branch와 무관하게 같은 grID 후보 중 실제 로드 가능한
  텍스처를 찾는다.
- `#SRC_IMAGE`의 110/111 같은 LR2 특수 소스는 일반 atlas crop으로 표시하지 않는다.

Tagged image는 Object의 SRC 좌표와 일치하는 IMG preset을 보여준다. 선택 목록의
thumbnail도 위와 같은 실제 grID/Branch 매핑을 사용해야 하며, 일치 항목이 없을
때만 `Custom coordinates`가 된다.

### Asset Browser

Asset Browser는 `arr_IMG`의 crop을 반응형 썸네일 grid로 표시한다. 검색은 asset
번호, 논리 gr, IF 번호와 SRC command 이름을 대상으로 하며 thumbnail 크기를
48~192px로 조절할 수 있다. 화면에 보이는 row만 `ImGuiListClipper`로 제출하므로
대형 스킨에서도 모든 card widget을 매 frame 만들지 않는다.

Asset Browser와 Image Manager는 `src_selected`, `grID_selected`, `gr_selected`를
공유한다. 텍스처는 `ResolveIMGTextureIndex()`에서 crop의 IF branch와 현재 custom
file 이름을 우선해 고르며, `gr == arr_SRCGR index`라고 가정하지 않는다. 클릭은
Image Manager에 one-shot scroll request를 보내고, 더블클릭은 Image Manager 탭을
활성화한다. 별도 asset 복사본을 만들지 않으므로 ImageManager의 crop 추가/삭제와
재파싱 결과가 다음 frame에 그대로 반영된다.

ImageManager의 `New`로 만든 crop도 같은 `arr_IMG`에 추가되어 Asset Browser에
즉시 나타난다. 생성 시 선택된 `SRCGR` 배열 위치가 아니라 그 선언의 논리 `grID`와
IF Branch를 저장하고, 검색을 해제한 뒤 새 card를 선택·스크롤한다. 수동 crop에는
원본 `#SRC`의 `div_x/div_y/cycle` 선언이 없으므로 정지 Asset으로 취급한다.

수동 crop은 선택한 `#IMAGE` 선언 바로 뒤에 다음 편집기 메타데이터로 보존한다.

```text
$SRC_IMAGE,0,gr,x,y,w,h,1,1,0,0,0,0,0
```

열 배치는 `#SRC_IMAGE`와 같고 command head만 `$`로 바뀐다. LR2에는 주석이지만
SkinEditor는 같은 원본 파일과 IF Branch의 Asset으로 다시 파싱하며, comments 제거
저장에서도 유지한다. 동일 crop을 실제 `#SRC_*` Object가 사용하면 저장 출력에서는
`$SRC_IMAGE`를 생략하고 실제 SRC가 삭제되면 다시 미사용 Asset으로 출력한다.
ImageManager에서 수동 crop을 삭제할 때는 대응하는 `$SRC_IMAGE` 행도 같이 삭제해,
저장·재로드 뒤 삭제한 Asset이 다시 나타나지 않게 한다.

`Animate SRC`가 켜져 있으면 `cycle > 0`이고 `div_x * div_y > 1`인 Asset은
LR2의 전체 cycle 시간에 맞춰 분할 frame을 행 우선 순서로 재생한다. 각 `IMG`는
자신을 만든 SRC 선언 행(`sourceDeclare`)을 보존하므로, 같은 gr/좌표 crop이
여러 번 선언돼도 다른 SRC의 분할값이나 숫자 atlas를 잘못 사용하지 않는다.
애니메이션은 card의 UV만 바꾸며 선택과 drag payload는 원래 전체 crop을 유지한다.

Asset card를 Preview로 drag하면 Preview 확대율/스크롤을 반영한 스킨 좌표에
반투명 crop ghost가 표시된다. SRC에 `div_x/div_y`가 있으면 전체 sheet가 아니라
현재 animation frame 한 칸의 UV와 크기를 ghost에 사용한다. Drop 시 CSV를 즉시
추가하지 않고 기존 New Object 창을 연다. 이 modal의 Object type은
IMAGE/NUMBER/SLIDER/BUTTON 네 가지이며, 종류를 바꿔도 Asset의 `gr/x/y/w/h`와
`div_x/div_y/cycle/timer`, Drop 위치가 유지된다. 대응하는 SRC/DST command를
만들고 현재 선택 Object의 IF branch를 사용한다. New Object의 SRC에는 원본
분할/animation 값을 유지하고 DST 크기는 한 frame 크기로 초기화한다.
Asset이 실제 SRC 선언에서 만들어졌고 새 Object type도 같은 command라면 공통
crop뿐 아니라 command-specific 필드도 schema 이름으로 복사한다. 따라서 기존
NUMBER Asset은 `num/align/keta`, SLIDER/BUTTON Asset은 각 type 관련 값을 유지한다.
수동 `$SRC_IMAGE` crop처럼 원본 SRC가 없는 Asset은 안전한 기본값을 사용한다.
실제 삽입은 사용자가 OK를 누를 때만 기존
New Object 저장/History 경로에서 수행한다. Asset Drop으로 연 New Object는 기존
도킹 위치를 재사용하지 않고 화면 중앙의 스크롤 가능한 modal로 표시한다. 우클릭
메뉴에서 여는 일반 New Object 창은 기존처럼 독립적으로 dock할 수 있다. 기본
layout에서 이를 사용할 수 있도록
Asset Browser는 Preview 아래에 두고, Customize는 오른쪽 OpList 아래에 배치한다.

로드된 CSV는 각 원본 파일마다 메모리상의 `$FILE ... start/end` marker로 감싸진다.
새 Object는 선택한 Object/Branch와 같은 filename 소유권으로 그 파일의 end marker
앞에 삽입하며, 선택 맥락이 없으면 root skin의 end marker 앞에 삽입한다. marker
밖에 append하면 Preview runtime mask에서 제외되므로 `skinfileLines.count`를 무조건
삽입 위치로 사용하면 안 된다. 생성 후에는 새 `$SE_OBJECT_ID`로 재구성된 모델을
찾아 Object Browser의 필터를 해제하고 Browser/Inspector 선택을 동기화한다.

SRC 명령과 짝을 이루는 DST를 자동 생성할 때 SRC에 `w/h/div_x/div_y`가 있으면
DST 크기는 한 animation cell의 크기인 `w / div_x`, `h / div_y`로 초기화한다.
분할값이 0 이하이거나 LR2의 특수 크기 값처럼 SRC 크기가 양수가 아니면 기존
fallback 값을 유지한다.

ImageManager의 `Pixel paint`는 선택한 `#IMAGE` 원본 texture에 1픽셀 단위로 점을
찍는 간단한 도트 편집 모드다. 좌클릭/drag는 선택 색 그리기, 우클릭/drag는 투명
지우개, 가운데 클릭은 원본 pixel 색 추출이다. Asset Browser의
`Pixel paint in Image Manager` context menu로 선택 Asset의 atlas를 바로 열 수 있다.

그리기는 먼저 D3D texture에만 반영되며 `Save image`에서 원본 이미지 파일을 atomic
replace한다. 최초 저장 전 같은 경로에 `.skineditor-pixel.bak` 원본 백업을 만들고,
`Revert`는 저장하지 않은 texture 변경을 디스크 상태로 되돌린다. 같은 파일을
공유하는 다른 `SRCGR` texture에도 편집 pixel을 동기화하며 저장 후 Preview runtime을
다시 불러온다. 현재 직접 편집은 lock 가능한 32-bit D3D texture에 한정한다.

ImageManager의 `New image`는 1~16384px 범위의 새 RGBA 이미지를 만들고,
`Merge image`는 현재 전체 texture 위에 선택한 Asset crop을 자동 배치해 alpha
composite한다. 기준 texture의 alpha가 0인 영역을 위에서부터 탐색해 crop 전체가
들어가는 첫 빈자리를 사용한다. 맞는 공간이 없으면 오른쪽 추가와 아래쪽 추가 중
최종 canvas 면적이 작은 방향을 골라 자동 확장한다. 사용자가 x/y나 확장 여부를
직접 입력하지 않는다. 두 기능 모두 기본 출력은 PNG이고 기존 파일을 덮어쓰지 않는다.

`Register in this skin CSV`가 켜져 있으면 생성 파일을 root skin의 마지막 graphic
slot에 다음 두 행으로 함께 등록한다.

```text
#IMAGE,.\relative\generated.png
$SRC_IMAGE,0,new_gr,0,0,width,height,1,1,0,0,0,0,0
```

IF/ELSEIF/ELSE는 형제 branch마다 같은 gr slot을 공유하므로 현재 선언 옆에 새
`#IMAGE`를 삽입하면 뒤의 기존 gr가 밀린다. 이를 피하기 위해 전체 조건 트리의 가장
긴 branch가 끝난 뒤의 trailing gr를 계산하고 root `$FILE ... end` 직전에 추가한다.
재파싱 후 full-size Asset이 생성되며 ImageManager와 Asset Browser가 그 Asset을
자동 선택한다. 출력 파일이 root skin 폴더 아래 있으면 CSV에는 portable relative
path를 저장하고, 바깥에 있으면 absolute path를 유지한다.

`arr_SRC`, `arr_DST`, `arr_IMG`, `arr_SRCGR`, `arr_ifunit`, `arr_seobj`는 CSV에서
파생되는 편집기 cache다. 모든 CSV 변경은 `NotifyDocumentChanged()`를 거쳐
구조/값/Object metadata 변경으로 분류된다. 구조·값 변경은 Editor cache와 Preview
runtime을 함께 invalidate하고, `$SE_...` metadata만 바뀌면 Object 모델만 다시
만든다. 값 연속 입력은 80ms debounce 후 재파싱하고 구조 변경은 다음 frame 시작에
즉시 처리한다. 텍스처 해제는 ImGui draw command 제출 전 frame 시작에서만 수행한다.

`ParseSkin()`은 다음 고정 순서의 단계 함수다.

1. `ParseSkinConditions()` — IF/ELSEIF/ELSE sibling 및 nested 관계
2. `ParseSkinLegacyObjectsAndAssets()` — legacy Object와 tagged crop
3. `ParseSkinGraphics()` — 논리 gr와 Branch별 `#IMAGE` 후보
4. `ParseSkinSourcesAndDestinations()` — SRC/DST 및 animation 연결
5. `LoadSkinGraphicMetadata()` — 이미지 크기만 lazy texture와 별도로 조사

Object 모델과 Browser UI cache는 전역이 아니라 각 `WORKSPACE`가 소유한다.
선택의 기준은 model index가 아니라 `$SE_OBJECT_ID`이며, ID가 없는 기존 Object만
group/첫 행을 fallback key로 사용한다. Preview, Browser, Inspector, DST View가
필요로 하는 정수 index는 이 key에서 매번 복원되는 파생값이다. Inspector에서 Name을
편집하면 파생 모델만 고치지 않고 `$SE_OBJECT_NAME` 행을 편집/삽입한다. ID가 없는
Object에 metadata를 처음 추가할 때는 `$SE_OBJECT_ID`도 함께 생성한다.

## 6. CSV, 인코딩, 저장과 History

### Shift-JIS/CP932

LR2 스킨 원문은 CP932를 유지한다. ImGui에는 UTF-8로 변환해 표시하고 사용자가
편집한 UTF-8 문자열은 CP932로 변환해 CSV 모델에 저장한다.

- 변환 함수: `Cp932ToUtf8()`, `Utf8ToCp932()`
- 적용 영역: 메타데이터, Customize, TextEdit, New Object, Object Inspector 등
- 폰트: Segoe UI 기본 글꼴에 Meiryo 일본어와 Malgun Gothic 한국어 glyph merge

TextEdit도 `GetCommandHelp()` schema를 사용해 `$type/$op/$st/$num/$timer`를
ComboBox로 표시한다.

Text 전용 로드 모드는 사용하지 않는다. 모든 스킨은 동일한 Workspace로 로드하며,
TextEdit은 `Windows > Text Editor`에서 여는 일반 도킹 창이다.

### 저장

- CSV 행과 포함 파일 소유권은 `SKINFILELINEREAD.filename`으로 추적한다.
- toolbar/File의 Save와 Ctrl+S는 현재 main path에 저장하며 include 구조와 memo를
  유지한다. 성공하면 현재 `documentRevision`을 saved revision으로 확정한다.
- 편집 revision이 saved revision과 다르면 하단 status bar는 `MODIFIED`, 같으면
  `SAVED`다. 저장 실패는 일정 시간 `SAVE FAILED`로 표시하며 dirty 상태는 유지한다.
- Pixel paint의 texture 변경은 script revision과 별도로 `IMAGE EDIT`로 표시한다.
- Save As의 BROWSE는 Windows 파일 선택기와 overwrite 확인을 사용한다.
- Save As merged mode는 확장된 행을 새 메인 파일에 합친다.
- split mode는 include 연결과 각 include 파일의 기존 경로를 유지한다.
- Save As 성공 시 `mainpath`와 관련 행의 owner를 새 경로로 전환하고 이후 편집도
  새 파일을 대상으로 한다.
- 이미지·폰트 등 외부 리소스 파일은 Save As가 복사하지 않는다.
- 실패 시 원래 파일과 작업 경로를 보존한다.

### History / Undo

`InsertLine`, `DeleteLine`, `EditLine/EditValue`가 History의 단일 진입점이다.
Ctrl+Z와 toolbar Undo는 `WORKSPACE::UndoLastEdit()`를 호출한다. 구조 변경 후에는
Object Model을 rebuild하고 Preview reload를 예약한다.

새 기능이 CSV를 직접 변경하면서 History를 우회하면 안 된다. 여러 내부 변경을
한 번의 사용자 작업으로 묶을 때만 `applyingHistory`를 제한적으로 사용한다.

## 7. 로딩 안정성과 배포

대형 tricoro HD 스킨 및 연속 스킨 로딩 중 발생했던 종료 문제에 대응해:

- 새 스킨을 열기 전에 이전 DST animation, texture, SRC/DST/IMG/Object/IF/History
  배열을 정리한다.
- SRC보다 먼저 등장하는 비정상/조건 확장 DST가 빈 Object 배열을 참조하지 않게
  guard한다.
- Object Model rebuild는 명령별 인덱스를 사용해 대형 파일의 반복 전수 검색을
  줄인다.
- 로딩 단계는 실행 폴더의 `SkinEditor_load_crash.log`에 기록한다.
- 내부 경로에 옛 Theme 폴더명이 남은 스킨은 열린 main skin의 sibling `Play`
  경로에서 동등 리소스를 탐색할 수 있다.

`skinHelper.txt`와 `skinObjGroup.txt`는 개발 시 외부 파일을 우선 읽고, 없거나
비어 있으면 `SkinEditorResources.rc`에 포함된 RCDATA를 읽는다. 배포 시 두 TXT는
필수 파일이 아니다.

FMOD는 delay-load이며 Preview 진입 시 오디오 경로를 비활성화한다. 따라서 현재
편집/Preview 기능은 `fmod.dll` 없이 시작할 수 있도록 구성되어 있다. 오디오
재생 기능을 다시 활성화하는 경우 DLL 의존성을 별도로 재검증해야 한다.

## 8. 현대화 UI 구조

현대화의 원칙은 기존 기능과 상태를 유지하면서 표현 계층만 재사용 가능하게
분리하는 것이다.

- `seUI.h/.cpp`: palette, compact spacing, toolbar/status bar, action button,
  status pill, section header, empty state, help marker
- `uiCatalog.h`: 도킹 창과 dialog/shell의 stable key, title, 목적, owner, 메뉴 group,
  기본 위치
- `main.cpp`: ImGui/DX9 lifetime, font와 전역 theme 초기화
- `WORKSPACE::draw()`: grouped menu, layout preset, four-column docking, toolbar,
  status bar, tool visibility, 실제 command 실행
- Object/CSV/Preview state는 기존 WORKSPACE와 모델이 단일 source of truth

`scripts\ui-map.ps1`은 이 카탈로그와 ImGui 호출을 결합해 JSON/Markdown/HTML을
만들며 CI에서 title/owner/ID 계약을 검사한다. AI 간 인계는 루트 `AGENTS.md`와
`docs/AI_COLLABORATION.md`를 시작점으로 삼고, `scripts\ai-context.ps1`이 문서 hash,
UI summary와 마지막 JUnit 결과를 context pack으로 묶는다.

창을 추가하거나 selection flow를 변경할 때는
[UI 구조 문서](UI_ARCHITECTURE.md)를 함께 갱신한다.

## 9. 반드시 유지할 구현 규칙

- `seObjectEditor.cpp`는 `seHelper.h`를 include한다.
- `SKINFILELINEREAD` helper 인자에 잘못된 `const`를 붙이지 않는다. 기존 CSTR/CSV
  API가 non-const 접근을 요구한다.
- `arr_CommandHelp` 직접 접근으로 새 schema 판정을 만들지 말고
  `GetCommandHelp()`를 사용한다. 명령 목록을 그리는 기존 코드 외에는 schema
  lookup을 중앙 함수로 통일한다.
- 조건 제어 행을 Object rows에 섞지 않는다.
- UI component가 별도 selection/model/history 사본을 소유하지 않는다.
- ImGui `Begin/End`, `BeginChild/EndChild`, ID와 cursor bounds 규칙을 지킨다.
- 기존 파일을 덮어쓰거나 삭제하는 동작은 명시적인 사용자 선택 없이 추가하지
  않는다.

## 10. 결정 기록

반복 논의를 거쳐 현재 기준으로 확정한 동작을 요약한다.

| 주제 | 현재 결정 |
|---|---|
| 해상도 변경 | 변경 즉시 skin 파일에 저장하는 것이 의도된 동작 |
| 자동 해상도 판정 | TenRiff의 LR2 lane/backdrop heuristic을 해상도 전용 모듈에서 사용 |
| Clone | Save As가 정상 동작하고 새 경로로 전환하므로 별도 Clone은 보류 |
| Save As | 성공 후 반드시 새 main script가 현재 작업 경로가 됨 |
| FAST/SLOW | 특수 UI 예외가 아니라 일반 Object 생성/편집 흐름으로 처리 |
| Branch context menu | 빈 Branch에 Object를 넣기 위해 유지 |
| Workspace Close | workspace 배열 삭제/상단 close UI는 제거한 상태를 유지 |
| Schema 배포 | 개발 중 외부 TXT 우선, 배포 시 실행 파일 내 RCDATA fallback |
| FMOD | Preview 편집에서는 선택 의존성. DLL이 없어도 실행 가능하게 유지 |
| Object Editor 구조 | Browser와 Inspector를 별도 도킹 창으로 유지 |
| Object 이름 fallback | 명시 이름, SRC symbolic 이름, op, non-zero timer 순서. 자동 이름은 저장하지 않음 |
| AI/UI 계약 | `uiCatalog.h`와 자동 UI map을 기준으로 하고 handoff는 context pack과 검증 증거를 포함 |
| OLR V0.2 authority | `skin.json`은 설명용, `lr2/main.lr2skin`은 왕복 호환 기준; `vfs/LR2files`는 LR2 Export 전까지 유지 |

## 11. 다음 작업 전 확인

1. 루트 `AGENTS.md`와 [AI 협업 가이드](AI_COLLABORATION.md)를 읽는다.
2. `git status --short`와 `git diff --stat`로 미커밋 작업을 확인한다.
3. `scripts\ai-context.ps1 -Check`로 최신 context pack을 만든다.
4. `Release|Win32`가 빌드되는지 확인한다.
5. `SkinEditor_DX9\Release\LR2files`가 테스트할 스킨을 포함하는지 확인한다.
6. tricoro HD와 bluewhite를 순서대로 열어 연속 로딩을 확인한다.
7. [회귀 테스트](BUILD_AND_TEST.md#회귀-테스트)를 실행한다.

현재 기능의 상세 UI 상태 흐름과 ImGui 디버깅 순서는
[UI_ARCHITECTURE.md](UI_ARCHITECTURE.md)에 정리되어 있다.
