# 빌드, 실행 및 회귀 테스트

## 개발 환경

- Windows
- Visual Studio 2022 C++ toolchain (`v143`)
- Windows 10 SDK
- Project: `SkinEditor_DX9\SkinEditor_DX9.vcxproj`
- 기본 검증 구성: `Release | Win32(x86)`
- Direct3D 9 / DxLib
- `Microsoft.DXSDK.D3DX` NuGet package `9.29.952.8`

프로젝트에는 Win32와 x64 구성이 모두 있지만 현재 배포와 반복 검증 기준은 x86다.
새 PC에서는 Visual Studio Installer에서 **Desktop development with C++**와
Windows 10/11 SDK를 설치한다.

## 자동 빌드와 테스트

저장소 루트에서 다음 두 명령을 순서대로 실행한다.

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\ui-map.ps1 -Check
powershell -ExecutionPolicy Bypass -File .\scripts\docs-check.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\test.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\ai-context.ps1 -Check
```

`build.ps1`은 Visual Studio 2022 C++ Build Tools를 `vswhere`로 찾는다. 구형
DirectX SDK 설치에 의존하지 않고 Microsoft D3DX 패키지를 고정 버전과 SHA-512로
검증하여 준비한 다음 `Release | Win32`를 빌드한다. 기존
개발·배포 실행 경로와 동일한 `SkinEditor_DX9\Release`에 실행 파일과 런타임을
출력한다. 중간 생성물은 `.build\obj`, 텍스트/binlog는 `.build\logs`에 둔다.

`test.ps1`은 다음 계약 테스트를 각각 별도 프로세스로 실행한다.

- `schema-contract`: 실행 파일에 포함된 command/object 스키마와 symbolic field
- `ui-contract`: 창 카탈로그의 고유 key/title, owner, dock, workspace별 ImGui ID
- `skin-browser`: 외부 폴더의 대소문자 확장자, 하위 폴더 탐색, 비스킨 파일 제외,
  잘못된 위치 거부
- `preview-simulator`: PLAY 키 모드별 메모리 chart의 시간순 lane 배치, 동시치기,
  LN/mine, 2P lane, measure event/sentinel 및 Rhythm 140 시작·리셋 계약
- `resolution-estimator`: `#INFORMATION`/`#RESOLUTION` 우선순위, include가 펼쳐진
  DST 경계 기반 720p/1080p 추정, 화면 밖 전환 frame 내성, 640x480 fallback
- `olr-package`: stored ZIP 생성/검사/추출, manifest와 semantic object/asset count,
  LR2·asset byte 보존, path traversal 거부 및 CRC 손상 탐지
- `asset-metadata`: Asset 메타데이터 저장, 재파싱, 삭제 및 graphic ID 배정
- `pixel-paint`: Direct3D texture 편집, 이미지 원자 저장, 생성 및 병합

결과는 `.build\test-results\skineditor-self-tests.xml` JUnit 파일로 남는다. 테스트
하나라도 실패하면 스크립트와 CI job이 실패한다.

`ai-context.ps1`은 UI 지도 계약을 다시 검사하고 주요 문서/스크립트의 SHA-256,
현재 UI summary, 사용 가능한 마지막 JUnit 결과를 다음 파일로 묶는다.

- `.build\ai-context\context-manifest.json`: 자동화가 비교하는 기계용 상태
- `.build\ai-context\context-pack.md`: 다른 AI가 먼저 읽는 짧은 인계 자료

특정 화면만 넘기려면 `-UiKey`를 사용한다.

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\ai-context.ps1 `
  -UiKey object-inspector -Check
```

이 경우 `.build\ai-context\object-inspector`에 해당 화면의 owner, 목적과 컨트롤
source 위치만 포함한 focus pack이 생성된다.

`docs-check.ps1`은 루트, `docs`, `.github`의 Markdown 코드 fence와 상대 링크 대상을
검사하고 `.build\docs-check\docs-check.json`을 남긴다.

`.github\workflows\ci.yml`은 push, pull request와 수동 실행에서 UI 지도 검증,
빌드, 자체 테스트와 AI context 생성을 Windows Server 2022에서 실행한다. 로그,
JUnit, UI 지도, 문서 검사와 AI 인계 자료는 실패 여부와 관계없이 CI
artifact로 보존한다. 러너는 현재 프로젝트의 Visual Studio 2022/`v143`
기준을 지키기 위해 `windows-2022`로 고정하고, checkout과 artifact action은 현행
major version을 사용한다.

## 저장소 배치

기본 예시:

```text
D:\Github\SkinEditor\
  +-- SkinEditor_DX9\
  |     +-- SkinEditor_DX9.vcxproj
  |     +-- Release\
  |           +-- SkinEditor_DX9.exe
  |           +-- LR2files\
  +-- LR2\
  +-- lib\
        +-- DxLib\
```

다른 경로에서도 빌드할 수 있지만 실행 시 상대 경로를 사용하는 코드와 테스트
스킨 배치 때문에 `Release` 폴더를 working directory로 실행하는 것이 안전하다.

`lib\DxLib`이 비어 있으면 보관된 `DxLib.zip`을 해당 위치에 풀어 include/lib가
프로젝트 설정과 맞는지 확인한다. 사용자 스킨 테스트에는
`SkinEditor_DX9\Release\LR2files`가 필요하다.

## Release x86 빌드

Visual Studio 2022 Developer PowerShell:

```powershell
cd D:\Github\SkinEditor
msbuild SkinEditor_DX9\SkinEditor_DX9.vcxproj /t:Build /p:Configuration=Release /p:Platform=Win32 /m
```

MSBuild 절대 경로 예시:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe' `
  SkinEditor_DX9\SkinEditor_DX9.vcxproj /t:Build `
  /p:Configuration=Release /p:Platform=Win32 /m
```

Codex/일부 PowerShell 환경에서 `Path`와 `PATH`가 동시에 상속되어 MSBuild가
`MSB6001`을 내면 현재 프로세스의 경로 변수를 한 번 정규화한다.

```powershell
$skinEditorBuildPath = $env:Path
Remove-Item Env:Path
$env:Path = $skinEditorBuildPath
```

성공 조건:

- 빌드 오류 0
- `SkinEditor_DX9\Release\SkinEditor_DX9.exe` 갱신
- 새 warning이 추가되지 않음

현재 프로젝트에는 기존 warning이 남아 있을 수 있으므로 총 warning 수만 보고
성공으로 판단하지 말고 변경 전후 차이를 확인한다.

Asset 메타데이터 저장/재파싱 자동 점검:

```powershell
$test = Start-Process `
  .\SkinEditor_DX9\Release\SkinEditor_DX9.exe `
  -ArgumentList '--self-test-asset-metadata' -Wait -PassThru
$test.ExitCode # 0이면 성공
```

이 테스트는 시스템 임시 폴더에서 사용/미사용 `$SRC_IMAGE`를 실제 저장하고 다시
파싱해 중복 제거, nocomment 보존, 논리 gr와 IF Branch 유지, crop 좌표 입력의
영속성, 수동 Asset 삭제 후 메타데이터 제거를 확인한다. 또한 IF/ELSE 형제 branch의
최대 graphic slot 뒤에 생성 이미지의 `#IMAGE + $SRC_IMAGE`가 root end marker
직전에 등록되는지 검사한 뒤 임시 파일을 삭제한다.

도트 그리기 texture와 파일 저장 경로:

```powershell
$test = Start-Process `
  .\SkinEditor_DX9\Release\SkinEditor_DX9.exe `
  -ArgumentList '--self-test-pixel-paint' -Wait -PassThru
$test.ExitCode # 0이면 성공
```

4x4 Direct3D texture에 RGBA pixel을 그리고 PNG로 atomic 저장한 뒤 다시 읽어 pixel
값과 `.skineditor-pixel.bak` 생성을 검사한다. 이어서 새 solid PNG 생성과 Asset
region alpha 병합/확장 canvas를 저장·재로드해 크기와 대표 pixel도 확인한다.

## 실행과 배포 파일

실행:

```powershell
cd D:\Github\SkinEditor\SkinEditor_DX9\Release
.\SkinEditor_DX9.exe
```

배포 시 핵심 사항:

- 파일명은 항상 `SkinEditor_DX9.exe`를 유지한다. 임시 suffix가 붙은 실행 파일을
  새 기준으로 만들지 않는다.
- `skinHelper.txt`, `skinObjGroup.txt`는 실행 파일에 RCDATA로 포함되므로 필수가
  아니다. 외부 파일이 있으면 개발용 override로 우선 읽는다.
- 현재 Preview는 오디오를 비활성화하므로 `fmod.dll` 없이 실행 가능한 구성을
  유지한다.
- 실제 스킨과 이미지 테스트가 필요하면 `LR2files`를 함께 둔다.
- PDB는 디버깅용이며 일반 실행에는 필수가 아니다.

## 최소 smoke test

1. `fmod.dll`, 외부 `skinHelper.txt`, 외부 `skinObjGroup.txt`가 없는 별도 폴더에서
   exe가 시작되는지 확인한다.
2. New/Open toolbar와 grouped Windows menu를 열고 Data 아래 Text Editor 항목이
   있는지 확인한다. `File > Open another location`에서 임의의 스킨 폴더를 고르면
   하위 폴더의 `.lr2skin`/`.lr2ss`가 목록에 나타나고, Refresh와 Default locations가
   정상 동작해야 한다.
   스킨 목록에는 별도의 `LOAD(TEXT)` 버튼이 없어야 한다.
3. 640x480 스킨 하나와 1280x720 이상 HD 스킨 하나를 각각 연다.
   HD 스킨은 `#INFORMATION` 크기가 없고 include 안의 DST 좌표만 있는 사례도
   포함한다. toolbar tooltip과 status bar가 `inferred`를 표시하고 Preview가 추정
   크기로 생성되며, 열기만 했을 때 원본 파일이 바뀌지 않아야 한다.
4. Preview, Image Manager, Asset Browser, DST View, Object Browser, Object Inspector가 보이는지
   확인한다. DST View를 처음 열었을 때 오른쪽 미리보기 열이 남은 폭을 즉시
   채우며, 폭을 확보하기 위해 splitter를 먼저 드래그할 필요가 없어야 한다.
   skin을 열기 전에는 Timer Control이 나타나지 않고, load가 끝난 뒤에는 기본
   layout에 나타나야 한다.
   Preview 하단에는 timer 조작 UI가 없어야 하며
   `Windows > Workspace > Timer Control`에서
   독립 창을 열어 scene restart를 수행하고, OpList 형식의 timer 항목을 체크/해제해
   해당 timer를 시작/리셋할 수 있어야 한다.
   runtime 상태인 timer는 기본 배경이고, 사용자가 직접 시작하거나 reset한 timer는
   각각 빨간 배경의 체크/빈 칸이어야 한다. Restart scene 후 빨간 배경이 사라지는지도
   확인한다.
   PLAY scene을 restart하면 선택한 무음 Preview chart가 LR2의 원본
   `ProcI_Play`/`DrawNotes` 경로로 실행되어야 한다. normal/LN/mine과 함께 key beam,
   note explosion, judge/combo가 해당 스킨 정의대로 반응하는지 확인한다. `Full`에서
   Timer Control의 140 Rhythm이 활성화되고 BPM 150에 따라 증가하다가 1.6초 간격의
   measure event에서 0부터 다시 시작하는지 확인한다. `Simple`은 sample 파일의 BPM과
   measure event를 따라야 한다. 140 tooltip은 ms가 아니라 `1000 = 1 beat`인 beat
   phase로 표시되어야 한다.
   기본 패턴은 `Simple`이며 현재 key mode에 맞는 기존 `sample_*.bme/pms`를
   재생해야 한다. `Full`을 누르면 장면이 즉시 재시작되고 기존 180개 밀집 패턴으로
   바뀌며, 다시 `Simple`로 돌아갈 수 있어야 한다. sample 파일이 없을 때는
   크래시하지 않고 16개 내장 패턴으로 동작해야 하며, 이 선택은 스킨 CSV에
   기록되지 않아야 한다.
   HI-SPEED 표시값은 200으로 시작하고 note가 judge line에 겹치지 않고 내려와야 한다.
5. Preview, Image Manager, DST View 캔버스 위에서 Ctrl+MouseWheel을 사용해
   마우스 아래의 이미지 지점이 움직이지 않은 채 확대/축소되는지 확인한다.
   Fit과 100%도 확인하고, 100% 초과 확대에서 선형 보간으로 흐려지지 않고
   원본 픽셀 경계가 선명하게 표시되는지 확인한다.
6. Object 속성 하나를 변경하고 Ctrl+Z로 복구한다.
7. 변경 후 하단 status bar가 `MODIFIED`, Ctrl+S 성공 후 `SAVED`인지 확인한다.
8. Save As의 BROWSE가 파일 선택기를 열고, 성공 후 새 경로가 workspace의 현재
   경로가 되는지 확인한다.
9. toolbar의 해상도 버튼에서 크기를 바꾸면 즉시 저장·재로드되며 Object 좌표가
   자동 확대되지 않는지 확인한다. 미저장 script/pixel edit 중에는 Apply가
   차단되는지도 확인한다.
10. `Layout > Show all windows`에서 16개 창이 여섯 dock tab group 안에 정돈되고,
    `Balanced workspace`가 기본 표시 상태와 4열 배치를 복원하는지 확인한다.
11. 실제 스킨에서 `File > Export OLR package`를 실행하고 일반 ZIP viewer로
    `manifest.json`, `skin.json`, `compatibility/source-map.json`,
    `lr2/main.lr2skin`, `lr2/assets/*`를 확인한다. source map에 드라이브명이나 로컬
    절대 경로가 없어야 한다. 같은 패키지를 `File > Import OLR package`로 빈 parent
    폴더에 가져오면 새 `<name>-lr2` 폴더가 생기고 추출한 스킨이 열려야 한다.
    정상/LN/mine, 폭발, judge/combo, gauge가 원본과 같은 LR2 Preview flow로
    표시되는지 확인한다. wildcard/font/video/sound가 있는 스킨은 Export 결과의
    외부 의존성 경고도 확인한다.

## 회귀 테스트

### A. 로딩 안정성

- tricoro `20th tricoro for HD - 7KEYS`를 단독으로 연다.
- tricoro 로딩 성공 후 bluewhite를 이어서 연다.
- 다시 tricoro를 열어 세 번째 load에서도 이전 배열/texture가 남지 않는지 본다.
- 실패 시 `Release\SkinEditor_load_crash.log`의 마지막 완료 stage를 기록한다.
- malformed include, 존재하지 않는 내부 Theme 폴더명, 대형 CSV를 확인한다.

### B. 조건 분기

다음 형태를 포함한 테스트 스킨을 사용한다.

```text
#IF,293
  ...
  #IF,900
    ...
  #ENDIF
#ELSEIF,294
  ...
#ELSE
  ...
#ENDIF
```

확인:

- IF/ELSEIF/ELSE가 한 ConditionBlock의 sibling인지
- IF와 ELSEIF 뒤에 각각 `293`, `294`가 표시되는지
- ELSE는 불필요한 파라미터 없이 표시되는지
- `Active objects only`에서 ELSE만 남았을 때 ELSE 툴팁에 그 체인의 원래
  IF/ELSEIF 조건이 모두 표시되는지
- nested IF가 IF branch의 child인지
- op를 바꿀 때 active filter와 녹색/빨간색이 동시에 갱신되는지
- 빈 branch 우클릭으로 Object를 생성할 수 있는지
- Type/Group/Search/Active 필터 영역에 내부 세로 scrollbar가 생기지 않는지

### C. Object 생성과 속성

- New Object와 New Command / Setting이 모두 보이는지
- Name을 입력하면 `$SE_OBJECT_NAME`이 생성되는지
- `$type/$op/$st/$num/$timer`가 ComboBox인지
- ComboBox에서 `Fast`, `Slow` 또는 숫자를 검색해 symbolic 값을 좁힐 수 있는지
- NUMBER는 `numberName`, SLIDER는 `sliderName`, BUTTON은 `buttonName`, TEXT는
  `textName`의 이름이 ComboBox에 나오는지
- 명시 이름이 없는 NUMBER/SLIDER/BUTTON/TEXT Object가 같은 symbolic 이름을
  Browser 이름으로 사용하는지
- New Object에서 `$num/$type/$st`를 바꾸면 수동 편집 전의 Name이 즉시 같은
  symbolic 이름으로 갱신되고, 직접 입력한 Name은 이후 값 변경에도 유지되는지
- symbolic 이름이 없는 Object가 op 이름, 그마저 없으면 non-zero timer 이름을
  쓰는지
- OpList의 자동 상태는 기본 배경이고, 강제 ON은 빨간 배경의 체크, 강제 OFF는 빨간
  배경의 빈 칸으로 유지되는지. Restart scene 후 자동 상태로 복귀하는지도 확인한다.
- timer 0만 가진 Object가 `MainTimer`로 도배되지 않는지
- Tagged image ComboBox의 thumbnail과 실제 SRC crop이 일치하는지
- preset에 일치할 때 `Custom coordinates`로 잘못 표시되지 않는지

### D. DST 프레임과 History

- DST 1개에서 `- DST`가 비활성인지
- `+ DST` 후 개수가 즉시 바뀌고 DST 탭이 유지되는지
- 추가 후 다른 Object Inspector로 바뀌지 않는지
- `- DST` 후 최소 하나가 남는지
- 추가/삭제 각각 Ctrl+Z가 되는지
- Undo 후 Preview object와 점멸 사각형 좌표가 일치하는지
- PLAY의 `NOWJUDGE_1P/2P`, `NOWCOMBO_1P/2P`를 선택했을 때도 점멸
  사각형이 표시되고 DST 프레임 위치를 따라가는지
- Object Browser에서 같은 파일·같은 Branch의 Object를 위/아래로 drag하면
  SRC/DST와 `$SE_OBJECT_NAME/$SE_OBJECT_ID`가 함께 이동하는지
- 다른 include 파일 또는 다른 IF/ELSEIF/ELSE Branch에 drop할 때 빨간 삽입선이
  표시되고 문서가 바뀌지 않는지
- 재정렬 직후 선택/Inspector/Preview가 같은 Object를 유지하고 Ctrl+Z 한 번으로
  원래 CSV 순서가 복구되는지

### E. 선택 동기화

- Object Browser 선택이 Inspector, DST View, Preview에 반영되는지
- Object 선택 후 Delete를 누르면 확인 modal이 뜨고 Cancel은 문서를 바꾸지 않는지
- 확인 modal의 Delete를 누르면 해당 Object의 SRC/DST와 `$SE_OBJECT_ID`,
  `$SE_OBJECT_NAME`, `$SE_GROUP_MEMBER` 참조가 함께 삭제되는지
- Object Inspector의 Name 등 text field를 편집할 때 Delete가 Object 삭제로
  동작하지 않는지
- DST View 선택이 Browser의 정확한 행으로 이동하고 자동 스크롤되는지
- Preview 우클릭 목록을 선택하면 Browser와 Inspector가 바뀌는지
- Browser filter가 걸려 있어도 Preview 요청이 대상을 보이게 만드는지
- Ctrl/Shift 다중 선택과 Preview drag가 선택 범위에만 적용되는지

### F. Preview 우클릭과 이미지 매핑

- 서로 겹친 Object만 우클릭 목록에 나타나는지
- 현재 op와 IF Branch가 성립하지 않는 Object는 제외되는지
- 각 목록 thumbnail이 해당 Object의 SRC command/index와 일치하는지
- hover한 항목의 얇은 노란 사각형이 실제 Object 위치인지
- 같은 gr 번호를 여러 `#IMAGE`가 공유할 때 Image Manager 상단에서 각 파일을
  전환할 수 있는지
- Image Manager의 동일 gr 파일 콤보와 선택 경로에서 CP932 한글·일본어 파일명이
  UTF-8 replacement 문자 없이 표시되는지
- Asset Browser가 `#SRC` crop을 썸네일 카드로 표시하고 검색/크기 조절이 되는지
- `Animate SRC` on/off가 동작하고, `div_x/div_y/cycle`이 있는 SRC 카드가
  cycle 시간 동안 모든 frame을 올바른 행/열 순서로 재생하는지
- 같은 좌표의 SRC 선언이 여러 개여도 원본 선언 행의 animation grid를 사용하고
  다른 SRC의 숫자 atlas/분할값을 사용하지 않는지
- ImageManager에서 `New`로 crop을 만든 직후 Asset Browser 검색이 해제되고 새
  `manual crop` card가 선택·스크롤되는지
- 같은 논리 gr를 여러 IF Branch가 공유할 때 수동 crop이 선택한 texture 선언의
  `grID/IF Branch`를 유지하고 다른 Branch 이미지를 표시하지 않는지
- 수동 crop은 우연히 같은 좌표의 animated SRC가 있어도 정지 card로 남는지
- 미사용 수동 crop 저장 후 `$SRC_IMAGE,0,gr,x,y,w,h,...`가 원래 `#IMAGE`와
  같은 파일·IF Branch에 남고, 재시작 후 Asset Browser에 복원되는지
- 동일 crop으로 Object를 만든 뒤 저장하면 중복 `$SRC_IMAGE`가 출력되지 않는지
- ImageManager에서 수동 crop을 삭제하고 저장·재로드했을 때 다시 나타나지 않는지
- Asset Browser의 crop이 같은 gr 번호의 다른 IF branch 텍스처로 바뀌지 않는지
- 카드 클릭 후 Image Manager 선택과 atlas 위치가 같고, 더블클릭 시 해당 탭이
  활성화되는지
- Asset Browser가 열린 상태에서도 Preview가 동시에 보이며, card를 Preview로
  drag하는 동안 `div_x/div_y` 한 frame 크기와 UV의 반투명 ghost가 표시되는지
- Drop 시 New Object 창이 열리고 `#SRC_IMAGE`의 gr/crop, `#DST_IMAGE`의 x/y/w/h,
  현재 선택 Object의 IF branch가 미리 채워지는지
- Drop modal에서 IMAGE/NUMBER/SLIDER/BUTTON을 바꿔도 gr/crop/div/cycle/timer와
  DST Drop 위치가 유지되고 선택한 command 쌍으로 생성되는지
- 기존 `#SRC_NUMBER`에서 나온 Asset을 NUMBER로 선택하면 `num/align/keta`가
  그대로 복사되는지. SLIDER/BUTTON도 같은 command-specific 복사 규칙을 따르는지
- 분할 SRC Drop 시 New Object의 `div_x/div_y/cycle/timer`가 원본과 같고 DST w/h가
  전체 sheet가 아닌 한 frame 크기인지
- ImageManager `Pixel paint`에서 좌클릭이 선택 RGBA 색을 1픽셀로 그리고,
  우클릭은 투명 pixel, 가운데 클릭은 색 추출로 동작하는지
- 빠르게 drag해도 mouse frame 사이가 1픽셀 선으로 이어지며 확대 화면이 선명한지
- `Revert`가 미저장 도트를 없애고, `Save image`가 원본 형식을 유지해 저장하며
  `.skineditor-pixel.bak`을 한 번만 만드는지
- 저장 후 Asset Browser의 같은 파일 thumbnail과 Preview가 갱신되는지
- `New image`에서 투명/유색 PNG를 만들 수 있고 기존 파일 경로를 지정하면
  덮어쓰지 않고 오류를 표시하는지
- 생성 등록 후 root CSV의 `$FILE ... end` 직전에 `#IMAGE`와 full-size
  `$SRC_IMAGE`가 함께 들어가며 ImageManager/Asset Browser가 새 Asset을 선택하는지
- 기존 IF/ELSEIF/ELSE 안의 gr 번호가 생성 전후 동일하고, 새 gr가 가장 긴 형제
  branch 뒤의 마지막 번호로 배정되는지
- `Merge image`가 선택 Asset crop 전체가 들어가는 첫 alpha=0 빈 영역을 자동으로
  찾아 합성하고, 빈 영역이 없으면 오른쪽/아래 확장 중 작은 canvas를 선택하는지
- skin 폴더 안의 출력 파일은 CSV에 상대 경로, 바깥 파일은 절대 경로로 저장되는지
- Asset Drop으로 연 New Object가 이전 도킹 위치가 아닌 화면 중앙 modal로 뜨고,
  내용이 길 때 내부 스크롤과 Cancel/닫기가 동작하는지
- New Object가 DST를 자동 생성할 때 SRC의 `w=320, h=160, div_x=4, div_y=2`라면
  DST의 `w=80, h=80`으로 생성되는지
- New Object 창을 닫으면 아무 CSV도 추가되지 않고, OK를 눌렀을 때만 Object가
  생성되는지
- 선택 Object 없이 Drop 후 OK를 눌러도 새 행이 루트 `$FILE ... end` 앞에 들어가
  저장 전 Preview에 즉시 보이는지
- Drop 후 생성된 Object가 Object Browser의 기존 Type/Group/Search/Active 필터에
  가려지지 않고 자동 선택·스크롤되며 Inspector에 같은 Object가 표시되는지
- 생성 직후 Preview에서 새 Object를 우클릭했을 때 후보 메뉴와 올바른 thumbnail이
  나타나고, hover 사각형 및 메뉴 선택이 새 Object를 가리키는지
- Object/DST 구조 추가·삭제 후와 Ctrl+Z 후에도 Preview 우클릭, DST View,
  Image Manager/Asset Browser가 현재 CSV 행을 기준으로 동기화되는지
- include 파일의 Object/Branch를 기준으로 생성하면 새 행의 filename 소유권이
  해당 include 파일로 유지되는지
- 그래프 배경 Object가 숫자 atlas를 잘못 사용하지 않는지
- DST View는 비활성 IF Branch의 텍스처도 표시할 수 있는지

### G. 해상도와 Scene

- 640x480 Preview
- 1280x720 이상 HD Preview
- `#INFORMATION`이 없고 `#RESOLUTION,1280,720`만 있는 스킨이 1280x720으로 열리는지
- 명시 해상도가 없고 DST가 include 파일에 분산된 HD 스킨이 전체 DST 경계로
  추정되며 toolbar/status에 `inferred`로 표시되는지
- 화면 밖 애니메이션/전환 DST 행 하나가 정상 720p 스킨을 4K로 과대 추정하지 않는지
- 추정 상태로 열기만 하면 원본이 바뀌지 않고, modal Apply 뒤에만 명시
  `#INFORMATION`으로 기록되는지
- PLAY 5/7/9/10/14 keys, 5/7/9 battle
- SELECT, DECIDE, RESULT
- 해상도 변경 즉시 원본 파일에 반영되는지
- 로드 후 toolbar 해상도 modal에서도 같은 즉시 저장 정책이 적용되고 정상적으로
  재로드되는지
- PLAY 무음 내장 chart note가 정상적으로 내려오는지

### H. New 프리셋

- 각 지원 Scene/key mode로 새 폴더를 만든다.
- 기존 파일이 있는 경로는 덮어쓰지 않는지 확인한다.
- 생성 직후 skin이 열리고 Preview가 표시되는지 확인한다.
- PLAY의 note/mine/LN/judge/gauge/FAST/SLOW를 확인한다.
- SELECT bar/title, DECIDE panel/flash, RESULT numbers/charts를 확인한다.
- 임의 해상도에서 화면 밖으로 심하게 벗어나는 필수 Object가 없는지 확인한다.

### I. 인코딩과 저장

- 일본어/한국어 제목과 Customize label이 깨지지 않는지
- TextEdit의 문자열 표시와 편집 후 저장이 CP932를 유지하는지
- Save As merged/split 양쪽을 확인한다.
- Save As 성공 후 추가 편집이 새 main script에 저장되는지
- 실패 또는 overwrite 취소 시 원본이 유지되는지

## 장애 기록 형식

재현 가능한 문제는 다음 내용을 남긴다.

```text
Build: AI branch commit/dirty status, Release Win32
Skin: scene, key mode, resolution, main lr2skin path
Steps: open -> option change -> selection -> action
Expected:
Actual:
Log: SkinEditor_load_crash.log 마지막 20줄
Screenshot:
```

이미지/선택 문제는 `model index`, `source row`, `dst row`, `ifgroup`, 논리 `grID`,
선택된 `SRCGR` 배열 인덱스를 함께 기록해야 배열 번호와 LR2 논리 번호를 혼동하지
않는다.
