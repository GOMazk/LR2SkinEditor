# 빌드, 실행 및 회귀 테스트

## 개발 환경

- Windows
- Visual Studio 2022 C++ toolchain (`v143`)
- Windows 10 SDK
- Project: `SkinEditor_DX9\SkinEditor_DX9.vcxproj`
- 기본 검증 구성: `Release | Win32(x86)`
- Direct3D 9 / DxLib
- `Microsoft.DXSDK.D3DX` NuGet package `9.29.952.8`

프로젝트는 Release Win32와 Release x64를 모두 빌드할 수 있다. 현재 배포와 전체
반복 검증 기준은 계속 x86이며, x64는 별도 산출물로 빌드·링크를 검증한다.
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
검증하여 준비한 다음 기본값으로 `Release | Win32`를 빌드한다. 기존 개발·배포
실행 경로와 동일한 `SkinEditor_DX9\Release`에 x86 실행 파일과 런타임을 출력한다.
`-Platform x64`를 지정하면 x64 DxLib/FMOD/D3DX 라이브러리를 선택하고 x64 D3DX
런타임과 실행 파일을 `SkinEditor_DX9\Release-x64`에 분리한다. 중간 생성물은
`.build\obj`, 텍스트/binlog는 `.build\logs`에 둔다.

`test.ps1`은 다음 계약 테스트를 각각 별도 프로세스로 실행한다.

- `schema-contract`: 실행 파일에 포함된 command/object 스키마와 symbolic field
- `ui-contract`: 창 카탈로그의 고유 key/title, owner, dock, workspace별 ImGui ID
- `skin-browser`: 외부 폴더의 대소문자 확장자, 하위 폴더 탐색, 비스킨 파일 제외,
  잘못된 위치 거부, 100개를 넘는 등록 스킨 목록의 구조체 크기 기반 안전 확장
- `preview-simulator`: PLAY 키 모드별 메모리 chart의 시간순 lane 배치, 동시치기,
  LN/mine, 2P lane, Simple sample의 LR2 호환 scratch-side 선택,
  COURSERESULT에서 PLAY로 전환할 때 course 상태 제거, measure event/sentinel 및
  Rhythm 140 시작·리셋 계약
- `resolution-estimator`: `#INFORMATION`/`#RESOLUTION` 우선순위, TenRiff에서 이식한
  lane/backdrop 기반 SD/HD/FHD 판정, 화면 밖 전환 panel 제외, 640x480 Preview
  fallback, LR2 출력 시 `#INFORMATION` 기록과 활성 `#RESOLUTION` 무력화
- `olr-package`: 명시적으로 두 part와 여러 source/destination을 구성한 V0.9
  document의 stored ZIP 생성/검사/추출, manifest와 `skin.json`/archive의 구조·asset
  count 일치, nested
  part별 Layout/Timeline/Condition compile, null timer/loop와 누락 OP slot 보존,
  M.H형 1P/2P wrapper와 include 내부 고아 `#ELSE` 사이의 `$OLR_FILE` scope 분리,
  고아 control row 무력화와 열린 child `#IF` 자동 종료, 재저장 시 scope marker 비증식,
  빈 numeric zero token의 무변경 왕복, 원본 main/include graph materialization과
  편집된 compatibility script의 안전한 fallback,
  OLR HD 기본값 및 package/원본-main의 `#INFORMATION` 해상도 단일화,
  known OP/TIMER와 raw OP 왕복, V0.8/V0.7 authority 호환, LR2·asset byte 보존,
  path traversal 거부, CRC 손상 탐지, CP932 가상/절대 경로의 반복 해석 안전성.
  별도 compiler fixture는 한 part의 multi-source binding과 명시적으로 구성한 두
  part 주소를 검증한다. 이 self-test는 `WORKSPACE::ExportOlrSkin()`을 호출하지
  않으므로 실제 Workspace 행의 multi-SRC-before-DST 또는 SRC/DST/SRC/DST 경계
  도출은 아래 수동 항목 15의 검증 범위다.
- `simple-mode`: Object Editor 그룹이 없는 기존 LR2 행에서도 숫자/콤보 폰트,
  판정 폰트, 기어 라인, 일반·롱·마인·AUTO 노트를 직접 분류하는 투영 계약
- `reload-lifecycle`: 중첩 CSTR/CSV/Object/History를 포함한 편집 문서를 두 번
  초기화해 이전 스킨의 소유 메모리와 파생 배열이 남거나 이중 해제되지 않는지,
  OLR source package 연결이 다음 문서로 새지 않는지, 미로드 Save OLRskin이 명확히
  실패하는지 확인
- `dst-color`: ARGB 네 필드를 한 picker gesture/History 항목으로 갱신하고 0~255로
  제한하며, 다음 gesture와 Ctrl+Z 두 번이 각 단계의 전체 색상을 복원하는지 확인
- `initial-preset`: 공용 atlas의 읽을 수 있는 0~9 NUMBER glyph, PLAY/BATTLE 키 모드별 lane index와 각 lane의
  Normal/Mine/LN/DST_NOTE/Bomb, Measure Line, Judge Line, Gauge, FAST/SLOW,
  플레이어별 NOWJUDGE/NOWCOMBO, RESULT의 label/판정 숫자/chart 및 COURSERESULT의
  1~5스테이지 제목/레벨과 누적 결과 필수 Object 생성, `#INFORMATION` 해상도와
  활성 `#RESOLUTION` 비생성
- `asset-metadata`: Asset 메타데이터 저장, 재파싱, 삭제, graphic ID 배정과 선택
  Object SRC에 대한 원자적 Asset 적용/Undo, `#IMAGE` 경로 교체/Undo, 이미지 상태
  진단, named grid Asset 일괄 등록/단일 Undo
- `object-reorder`: 같은 파일 안의 IF/ELSEIF/ELSE 간 Object 이동과 확인 후 서로
  다른 include 파일 간 소유권 이전, metadata/선택 보존 및 단일 Undo
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
Release x86 빌드·자체 테스트, Release x64 빌드와 AI context 생성을 Windows Server
2022에서 실행한다. 로그,
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
  |     +-- Release-x64\
  |           +-- SkinEditor_DX9.exe
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

## Release x64 빌드

저장소 루트의 권장 명령은 다음과 같다.

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Platform x64
```

Visual Studio 2022 Developer PowerShell에서 직접 빌드할 수도 있다.

```powershell
cd D:\Github\SkinEditor
msbuild SkinEditor_DX9\SkinEditor_DX9.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m
```

성공하면 `SkinEditor_DX9\Release-x64\SkinEditor_DX9.exe`와 x64
`D3DX9_43.dll`이 생성된다. x86과 x64는 서로 다른 출력·중간 폴더를 사용하므로
연속으로 빌드해도 산출물을 덮어쓰지 않는다. 전체 GUI/LR2 호환 회귀 기준은 아직
x86이며, x64 실행 파일은 자동 self-test와 별도 smoke test로 확인한다.

Asset 메타데이터 저장/재파싱 자동 점검:

```powershell
$test = Start-Process `
  .\SkinEditor_DX9\Release\SkinEditor_DX9.exe `
  -ArgumentList '--self-test-asset-metadata' -Wait -PassThru
$test.ExitCode # 0이면 성공
```

이 테스트는 시스템 임시 폴더에서 사용/미사용 `$SRC_IMAGE`를 실제 저장하고 다시
파싱해 중복 제거, nocomment 보존, 논리 gr와 IF Branch 유지, crop 좌표 입력의
영속성, 수동 Asset 삭제 후 메타데이터 제거를 확인한다. 선택 Object의 SRC에 다른
Asset을 적용할 때 `gr/x/y/w/h`만 바뀌고 NUMBER의 `num/align/keta`가 보존되며,
History 한 건과 Ctrl+Z 한 번으로 원본 행이 복구되는지도 검사한다. 또한 IF/ELSE 형제 branch의
최대 graphic slot 뒤에 생성 이미지의 `#IMAGE + $SRC_IMAGE`가 root end marker
직전에 등록되는지 검사한다. 이어서 missing/unused 이미지 진단, `#IMAGE` 경로만
교체한 뒤 Undo 복원, 2x2 named `$SRC_IMAGE` 분할의 재파싱 및 일괄 Ctrl+Z를 확인한
뒤 임시 파일을 삭제한다.

DST ARGB 색상 편집과 History 자동 점검:

```powershell
$test = Start-Process `
  .\SkinEditor_DX9\Release\SkinEditor_DX9.exe `
  -ArgumentList '--self-test-dst-color' -Wait -PassThru
$test.ExitCode # 0이면 성공
```

한 picker gesture 안의 여러 ARGB 갱신이 History 한 건으로 합쳐지고, byte 범위 제한과
gesture별 Ctrl+Z 복원이 모두 동작하는지 검사한다.

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

New 시작 프리셋 PNG 자동 점검:

```powershell
$test = Start-Process `
  .\SkinEditor_DX9\Release\SkinEditor_DX9.exe `
  -ArgumentList '--self-test-initial-preset' -Wait -PassThru
$test.ExitCode # 0이면 성공
```

임시 작업 폴더에서 New와 같은 경로로 시작 프리셋을 생성해 스크립트가
`preset.png`만 참조하는지 확인한다. 생성된 모든 PLAY/BATTLE/SELECT/DECIDE/RESULT/
COURSERESULT 256x256 PNG를 다시 읽어 빈 아틀라스 영역의 alpha가 0이고 실제 source
영역의 alpha가 255인지 검사한다. SELECT bar 색상, RESULT 숫자 glyph와 gauge/chart
색상도 pixel 단위로 확인한 뒤 임시 파일을 삭제한다.

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
   실제 LR2 설치 밖의 M.H/IIDX 형태 폴더를 열어 `LR2files\Theme\IIDX`
   include와 wildcard image/custom file, LR2FONT가 원본 CSV 수정 없이 해결되고
   Preview에서 note/폭발/judge/combo/gauge가 표시되는지 확인한다.
   New로 스킨을 만들면 `preset.png`가 생성되고 빈 아틀라스 영역이 투명한지,
   생성 직후 해당 스킨이 정상적으로 열리는지도 확인한다.
   New Skin 창은 열릴 때 `620 x 400` 크기로 표시되고, 생성 실패 메시지가 세로
   스크롤 없이 하단에 보여야 한다.
   Image Manager의 같은 gr 콤보에는 LR2 와일드카드에 잡힌 폴더와 비이미지 파일도
   후보로 유지되어야 한다. 다만 처음 열거나 tagged image를 바꿀 때는 그 항목을
   자동 기본값으로 선택하지 않고, 실제로 로드 가능한 첫 이미지가 표시되어야 한다.
   `...\\*\\main.png`는 후보 `Default`를 그대로 표시하면서 실제로는
   `...\\Default\\main.png`를 로드해야 한다.
   `...\\*.png`는 다른 확장자의 파일을 후보로 포함하지 않고 `OFF` 후보를
   `...\\OFF.png`로 해석해야 하며, `OFF.png.png`를 만들면 안 된다.
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
   Text Editor에서 SRC 또는 DST Object 명령 행을 좌클릭하면 Object Browser/Inspector가
   열리고 Type/Group/Search/Active-only 필터에 가려지지 않은 채 해당 Object로 자동
   스크롤되어야 한다. IF 헤더나 주석을 누르면 기존 Object 선택이 유지되어야 한다.
   PLAY scene을 restart하면 선택한 무음 Preview chart가 LR2의 원본
   `ProcI_Play`/`DrawNotes` 경로로 실행되어야 한다. normal/LN/mine과 함께 key beam,
   note explosion, judge/combo가 해당 스킨 정의대로 반응하는지 확인한다. `Full`에서
   Timer Control의 140 Rhythm이 활성화되고 BPM 150에 따라 증가하다가 1.6초 간격의
   measure event에서 0부터 다시 시작하는지 확인한다. `Simple`은 sample 파일의 BPM과
   measure event를 따라야 한다. 140 tooltip은 ms가 아니라 `1000 = 1 beat`인 beat
   phase로 표시되어야 한다.
   `#SCRATCHSIDE,1,0`인 7KEY skin에서도 note key 1~7과 key beam timer 101~107이
   같은 lane에서 반응해야 하며, scratch lane 0만 오른쪽에 표시되어야 한다.
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
11. 실제 M.H/IIDX 형태 스킨에서 `File > Save OLRskin`을 실행하고
    일반 ZIP viewer로 `manifest.json`, `skin.json`,
    `compatibility/source-map.json`, `compatibility/path-map.json`,
    `lr2/main.lr2skin`, `lr2/.olr-export-main.txt`, V0.9의
    `lr2/.olr-compatibility-baseline.lr2skin`,
    `lr2/.olr-preserve-original-main`,
    `lr2/vfs/LR2files/Theme/<skin>/*`를 확인한다. map에 드라이브명이나
    로컬 절대 경로가 없어야 한다. 일반 LR2 workspace의 원본 script는 이 명령만으로
    바뀌지 않아야 한다. 같은 패키지를 `File > Import OLR package`로
    빈 parent 폴더에 가져오면 새 `<name>-olr-workspace` 폴더가 생기고 추출한
    `main.lr2skin`이 열려야 한다. 정상/LN/mine, 폭발, judge/combo, gauge와
    font를 원본과 같은 LR2 Preview flow로 다시 확인한다. Import한 workspace를
    열었을 때 M.H의 1P/2P 레이아웃이 모두 유지되고 오른쪽 scratch만 남는 현상이
    없어야 한다. 특히 원본과 Import 결과의 `#DST_BARGRAPH`에서
    `loop,timer,op1,op2,op3` 열을 비교한다.
    Import한 workspace를
    편집한 뒤 `Save OLRskin`을 다시 실행하면 원래 package 경로가 제안되고, 재Import한
    script에 편집이 남아 있어야 하며, 이전 `.olrskin` 파일이 새 package 내부
    `lr2/vfs/`에 재귀 포함되지 않아야 한다. 이 workspace에서
    `File > Export install-ready LR2 folder`를
    새 대상으로 실행하면
    `<target>/LR2files/...`가 생겨야 한다. 무편집 V0.9 package는 성공 메시지가 원본
    include-based main 보존을 표시하고, exported main과 include tree가 원본과
    같아야 한다. 단, main header는 `#INFORMATION` 6·7번 칸이 package canvas와 같고
    활성 `#RESOLUTION`이 없어야 하며 이 의도된 정규화는 byte-for-byte 비교에서
    제외한다. 나머지 빈 timer/cycle/angle, `div_x/div_y=0`,
    `#CUSTOMOPTION`과 `#ENDOFHEADER`도 비교한다. 편집 후에는 compatibility script
    materialization을 표시하고 main CSV에 `vfs/`가 남지 않아야 한다.
    기존 대상 폴더를 덮어쓰지 않고, 해결불가/과도하게 긴 경로는 Export 결과의
    외부 의존성/누락 경고와 manifest 개수에 나타나야 한다.
12. Simple Mode의 Notes에서 흰 건반/검은 건반/scratch scope를 각각 적용해 같은
    note part만 바뀌는지 확인한다. 판정/콤보는 1P/2P pair를 확인하고, atlas grid와
    맞지 않는 image import가 거부되는지 확인한다. Hue/Saturation/Brightness variant가
    `simple-assets`의 새 PNG를 사용하고 Undo 시 원본 PNG가 변하지 않는지도 확인한다.
13. V0.9 package의 `skin.json.simple_mode.slots[].asset` 하나를 바꾸어 Import하면
    대응 `#SRC_*`의 `gr/x/y/w/h/div_x/div_y/cycle`만 바뀌는지 비교한다. 잘못된
    `source_row` 또는 `source_command` package는 새 import folder 없이 실패해야 한다.
    KCOOL처럼 `#SRC_IMAGE`에 `w/h=-1` 또는 음수 width를 쓰는 legacy crop은
    `simple_mode.slots`에서 제외되거나 Import 시 raw LR2 행으로 유지되어야 하며,
    이 때문에 전체 package Import가 실패해서는 안 된다.
    실제 package core 검증은 `SKINEDITOR_TEST_OLR_PACKAGE`에 읽을 `.olrskin` 경로를
    지정한 뒤 `--self-test-olr-package`를 실행한다. 테스트는 임시 폴더에서 Import를
    실행하고, V0.2+이면 설치용 LR2 Export와 exported main의 `vfs/LR2files` 제거까지
    확인한 뒤 임시 결과를 지운다. 원본 package는 수정하지 않는다.
    `#SRC_GROOVEGAUGE`, `#SRC_SCORECHART`, `#SRC_GAUGECHART_*`도 Gauge 그룹에서
    같은 계약으로 편집되는지 확인한다.
14. Object Inspector의 Layout에서 DST rectangle을 편집하고 Preview의 흰 handle로
    resize한다. Inspector는 현재 선택 Object에서 complete semantic contract를 가진 첫
    DST command family만 Layout/Timeline/Conditions로 보여 주며 nested part 또는 다른
    destination family selector는 제공하지 않는다. 다른 family는 `Advanced LR2`에서
    확인한다. 이 범위 안에서 Timeline frame 0/1의 time, alpha, position, rotation,
    blend를 바꾼 뒤
    `skin.json.objects.items[].parts[].destinations[]`와 Import 결과의 같은 `#DST_*`
    행을 비교한다. Conditions에서 알려진 OP와 custom OP 948을 서로 다른 1~3 `slot`에
    지정해 Simulator의 VISIBLE/HIDDEN과 exported semantic/raw 구분을 확인한다.
    `timer` 또는 `loop`를 `null`로 바꾸거나 한 OP slot을 JSON에서 생략하면 해당 원본
    CSV 필드는 Import 뒤에도 그대로여야 한다. 중복 slot, 잘못된 part source row,
    destination row/command 또는 Layout과 frame 0이 다른 package는 새 import folder
    없이 실패해야 한다.
15. 실제 kamh 스킨을 `Save OLRskin`하고 BUTTON Object를 확인한다. 이 단계가
    `WORKSPACE::ExportOlrSkin()`의 part 경계 도출을 검증하는 기준이며 자동
    `olr-package` self-test가 대신하지 않는다.
    `#SRC_BUTTON -> #DST_BUTTON -> #SRC_BUTTON -> #DST_BUTTON` 순서는 두 part가 되어야
    하고 각 destination은 바로 앞 source-bound part 아래 있어야 한다. 두 part의
    Layout을 서로 다르게 바꿔 재Import한 뒤 양쪽 BUTTON이 Preview에서 독립적으로
    유지되는지와 관련 없는 raw 행이 바뀌지 않았는지 비교한다. NOTE Object는 DST 전의
    여러 `#SRC_NOTE/#SRC_MINE/#SRC_LN_END`가 같은 part에 있고 DST 뒤 새 SRC만 다음
    part를 시작하는지 확인한다. manifest의 `object_count`, `part_count`,
    `destination_count`도 실제 nested array 합계와 일치해야 한다.
    `objects.items`의 첫 compiler row가 단조 증가하고, 같은 `sections` category의
    id도 그 순서를 따르는지 확인한다. Import된 `main.lr2skin`은 semantic target
    열 외에 행 순서가 바뀌면 안 된다.
16. 보관 중인 V0.1-V0.8 package를 Import하여 각 version의 기존 parser/authority가
    그대로 선택되는지 확인한다. 특히 V0.7의 flat `objects.items`가 V0.9 nested part로
    추측 변환되지 않아야 한다. 현재 writer는 알지 못하는 manifest/`skin.json` 확장
    필드를 재저장할 때 보존하지 않으므로, 이 동작을 forward-compatible round trip으로
    기록하거나 보장하지 않는다.

## 회귀 테스트

### A. 로딩 안정성

- tricoro `20th tricoro for HD - 7KEYS`를 단독으로 연다.
- tricoro 로딩 성공 후 bluewhite를 이어서 연다.
- 다시 tricoro를 열어 세 번째 load에서도 이전 배열/texture가 남지 않는지 본다.
- CP932 이름과 대형 include tree가 있는 스킨은 일반 스킨 전후 양방향으로 연다.
  이미 절대 경로로 해석한 리소스를 다시 filesystem 경로로 변환하지 않아야 한다.
- 실패 시 `Release\SkinEditor_load_crash.log`의 마지막 완료 stage를 기록한다.
- malformed include, 존재하지 않는 내부 Theme 폴더명, 대형 CSV를 확인한다.
- Workspace 두 개에서 각각 PLAY 스킨을 열고 `Timer Control > Restart scene`을 누른다.
  한쪽 Preview를 비활성 dock tab으로 둔 채 5초 이상 기다린 뒤 다시 열었을 때 note와
  chart가 진행된 위치에 있고 양쪽 timer 41이 모두 계속 증가했는지 확인한다. 비활성
  Workspace도 scene tick 뒤 같은 frame에 draw buffer를 소비해 폭발/judge/combo가
  누적되거나 멈추지 않아야 한다.

같은 경계를 실제 스킨 두 개로 자동 검사할 때는 UI thread에서 두 Workspace를 번갈아
4 frame 진행하는 smoke 명령을 사용한다. 환경 변수는 서로 다른 PLAY 스킨을 가리켜야
하며, 종료 코드 0이면 두 `game`의 core 초기화, Scene 시작, draw-buffer 소비와 timer 41
진행을 모두 통과한 것이다.

```powershell
$env:SKINEDITOR_RELOAD_FIRST = 'D:\skins\first.lr2skin'
$env:SKINEDITOR_RELOAD_SECOND = 'D:\skins\second.lr2skin'
$test = Start-Process .\SkinEditor_DX9\Release\SkinEditor_DX9.exe `
  -ArgumentList '--skin-multi-workspace-smoke' -Wait -PassThru
$test.ExitCode
```

로컬에 실제 스킨 두 개가 있을 때는 숨겨진 runtime smoke 경로로 같은 프로세스의
연속 로드를 자동 확인할 수 있다. CI에는 사용자 스킨을 포함하지 않으므로 넣지 않는다.

```powershell
$env:SKINEDITOR_RELOAD_FIRST = 'D:\skins\first.lr2skin'
$env:SKINEDITOR_RELOAD_SECOND = 'D:\skins\second.lr2skin'
$test = Start-Process .\SkinEditor_DX9\Release\SkinEditor_DX9.exe `
  -ArgumentList '--skin-reload-smoke' -Wait -PassThru
$test.ExitCode # 0이면 두 스킨 모두 같은 WORKSPACE에서 load 완료
```

`Workspace > New Workspace` 경계는 별도 mode로 확인한다. 첫 Workspace를 유지한
채 두 번째 Workspace에서 다른 스킨을 load하고, 두 scene을 시작해 여러 frame 동안
번갈아 `ProcGame`/draw를 실행한다. 두 Workspace의 timer 41이 모두 전진해야 하며,
비활성 Preview dock tab도 멈추면 안 된다. `tricoro -> m.h(IIDX)`처럼 첫 스킨이 큰
경우도 이 경로를 사용한다.

```powershell
$env:SKINEDITOR_RELOAD_FIRST = 'D:\skins\tricoro\play_7.lr2skin'
$env:SKINEDITOR_RELOAD_SECOND = 'D:\skins\mh\play_single.lr2skin'
$test = Start-Process .\SkinEditor_DX9\Release\SkinEditor_DX9.exe `
  -ArgumentList '--skin-multi-workspace-smoke' -Wait -PassThru
$test.ExitCode # 0이면 두 Workspace의 load, 다중 frame scene 진행과 Preview draw 완료
```

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

- 단일 DST의 `a/r/g/b` 네 숫자 입력 대신 `ARGB` 색상 선택기 하나가 표시되는지
- 여러 DST animation frame 표에서도 각 열마다 `ARGB` 선택기가 하나씩 표시되고,
  색상 변경이 Preview에 반영되며 Ctrl+Z 한 번으로 조작 전 네 값이 모두 복구되는지
- DST 1개에서 `- DST`가 비활성인지
- `+ DST` 후 개수가 즉시 바뀌고 DST 탭이 유지되는지
- 추가 후 다른 Object Inspector로 바뀌지 않는지
- `- DST` 후 최소 하나가 남는지
- 추가/삭제 각각 Ctrl+Z가 되는지
- Undo 후 Preview object와 점멸 사각형 좌표가 일치하는지
- `#SRC_NUMBER`의 `keta`를 4 이상으로 설정했을 때 Preview 점멸 사각형의
  폭이 단일 glyph가 아니라 `DST w * keta`인지
- NUMBER `align=0/1/2`를 각각 선택해도 점멸 사각형의 왼쪽은 DST `x`이고,
  숫자 glyph만 그 필드 안에서 right/left/middle로 배치되는지. 특히 right에서
  사각형 전체가 DST `x` 왼쪽으로 이동하면 안 된다.
- TEXT는 NUMBER와 다른 `align` 순서인 `0=left`, `1=middle`, `2=right`를
  사용하는지. 점멸 사각형은 실제 렌더링된 문자열 폭을 사용하고 middle은 그 폭의
  절반, right는 전체 폭만큼 DST `x` 왼쪽에 표시되는지
- PLAY의 `NOWJUDGE_1P/2P`, `NOWCOMBO_1P/2P`를 선택했을 때도 점멸
  사각형이 표시되고 DST 프레임 위치를 따라가는지
- Object Browser에서 같은 파일의 Object를 같은 Branch 또는 다른
  IF/ELSEIF/ELSE Branch의 Object 위/아래로 drag하면 SRC/DST와
  `$SE_OBJECT_NAME/$SE_OBJECT_ID`가 함께 이동하는지
- 다른 include 파일의 Object에 drop할 때 주황색 삽입선과 확인창이 표시되는지.
  Cancel하면 문서가 바뀌지 않고, 승인하면 SRC/DST와 `$SE_OBJECT_ID/NAME`의 파일
  소유권이 대상 include로 바뀌는지
- 재정렬 직후 선택/Inspector/Preview가 같은 Object를 유지하고 Ctrl+Z 한 번으로
  원래 CSV 순서가 복구되는지

### E. 선택 동기화

- Object Browser 선택이 Inspector, DST View, Preview에 반영되면서 Preview 탭이
  활성화되고 DST View가 포커스를 빼앗지 않는지
- Object Browser, Preview, DST View 또는 Text Editor에서 Object를 선택하면
  Image Manager가 그 Object의 SRC crop으로 이동하고 atlas 위에 주황색 점멸
  사각형을 표시하는지
- Preview scene timer를 멈추거나 skin을 막 연 상태에서도 Image Manager의
  주황색 사각형이 투명하게 고정되지 않고 계속 점멸하는지
- Image Manager atlas의 crop 위에 mouse를 올리면 파란 점멸 사각형과 좌표
  tooltip이 표시되는지. `w/h == -1`인 crop도 texture 끝까지 올바르게 잡히는지
- Image Manager 왼쪽 목록에서 현재 선택되지 않은 crop에 mouse를 올려도, 실제
  같은 texture를 쓰는 crop이라면 atlas에 파란 점멸 사각형이 표시되는지. 다른
  IF Branch 소속이라는 이유만으로 제외되지 않는지
- Object Browser에서 선택되지 않은 Object row에 mouse를 올리면 현재 Object와
  Inspector 선택은 유지되고, 같은 texture를 사용하는 hover Object의 SRC crop이
  Image Manager atlas에 파란 점멸 사각형으로 표시되는지
- Undo 또는 Object 모델 재구성 뒤에도 복원된 Object와 Image Manager crop이
  일치하는지
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
- 겹친 Object 목록이 LR2 draw order의 역순, 즉 화면 앞쪽 Object부터 표시되는지
- 현재 op와 IF Branch가 성립하지 않는 Object는 제외되는지
- 각 목록 thumbnail이 해당 Object의 SRC command/index와 일치하는지
- hover한 항목의 얇은 노란 사각형이 실제 Object 위치인지
- 같은 gr 번호를 여러 `#IMAGE`가 공유할 때 Image Manager 상단에서 각 파일을
  전환할 수 있는지
- Image Manager의 동일 gr 파일 콤보와 선택 경로에서 CP932 한글·일본어 파일명이
  UTF-8 replacement 문자 없이 표시되는지
- Image Manager 왼쪽 crop 목록과 이름 편집, `Usage`로 연 Image status에서 CP932
  한글·일본어 이름과 경로가 깨지지 않는지
- Image Manager의 `grReload` 옆 `Folder`를 누르면 현재 이미지 파일이 선택된
  Windows Explorer가 열리는지. 후보 파일이 없으면 가장 가까운 기존 폴더가
  열리는지
- 디스크에서 현재 이미지를 바꾼 뒤 `grReload`를 누르면 다음 frame에 새 내용이
  나타나고, 미저장 Pixel paint가 있을 때는 reload가 차단되는지
- Image Manager의 `Replace`에서 같은 크기 파일은 즉시 `#IMAGE` 경로만 바뀌고,
  다른 크기 파일은 영향 crop/경계 이탈 수를 확인한 뒤 바뀌는지. crop 좌표와 논리
  gr/IF Branch가 유지되고 Ctrl+Z 한 번으로 원래 경로가 복구되는지
- Pixel paint가 저장되지 않은 상태에서는 `Replace`가 비활성화되는지
- `Split grid`가 SRC의 `div_x/div_y`를 초기 Columns/Rows로 제안하고 전체 crop
  preview에서 선택한 cell만 순번 이름의 `$SRC_IMAGE`로 등록하는지. 중복 cell은
  건너뛰고 저장·재로드 후 이름/좌표가 유지되며 Ctrl+Z 한 번에 batch 전체가
  제거되는지
- `Image status`가 missing/unloadable 파일, bounds 밖 crop, 중복 crop, 미사용
  `$SRC_IMAGE`, Asset 없는 Object SRC와 다음/남은 gr slot을 표시하는지. 진단 행을
  누르면 대응 Asset 또는 Object 선택으로 이동하는지
- `Add image...`에서 기존 이미지 파일을 선택하면 modal의 대상 목록에 gr 번호,
  Fixed/Wildcard, IF group과 선언 경로가 표시되는지. `New fixed #IMAGE`는 마지막 gr에
  새 선언과 전체 크기 `$SRC_IMAGE`를 등록하고 ImageManager/Asset Browser가 새 Asset을
  선택해야 한다. 기존 fixed/wildcard gr는 선택 파일이 그 선언의 실제 후보일 때만
  전체 크기 Asset을 추가하고 `#IMAGE` 수와 뒤쪽 gr 번호를 바꾸지 않아야 한다. 다른
  fixed 파일이나 wildcard 비후보는 등록 버튼이 비활성화되어야 하며, 이미지가 없는
  스킨에서도 새 trailing gr 흐름이 가능해야 한다.
- Asset Browser가 `#SRC` crop을 썸네일 카드로 표시하고 검색/크기 조절이 되는지
- Asset Browser card와 Image Manager crop 목록에 실제 사용 중인 고유 Object 수
  또는 `Unused`가 표시되는지. 한 Object의 중복 SRC를 두 번 세지 않는지
- `Unused only`를 켜면 Object SRC에서 참조하지 않는 Asset만 남고 CSV/선택 상태는
  바뀌지 않는지
- Object Browser의 활성/비활성 배경이 label 폭까지만 표시되고, 왼쪽 상태 stripe와
  중첩 IF Branch 행이 서로 겹치지 않아 이름을 읽을 수 있는지
- Object Browser에서 같은 파일의 다른 IF/ELSEIF/ELSE Branch Object 위·아래로
  drag하면 원본 Object 행과 `$SE_OBJECT_ID/NAME`이 대상 Branch 내부로 함께 이동하고,
  조건 directive 자체는 유지되며 Ctrl+Z 한 번에 원래 Branch/순서로 돌아오는지
- 서로 다른 include 파일 사이로 drop하면 확인 modal이 source/destination 파일을
  보여주는지. Cancel은 무변경이어야 하며, 승인 후 Save하면 원본 파일에서 Object가
  빠지고 대상 파일의 선택 Branch에 기록되며 Ctrl+Z 한 번으로 양쪽이 복원되는지
- 파일 간 이동 후 Inspector가 `DST (1)`인 Object는 Preview에서 선택 Object의 실제
  좌표에 파란 사각형 하나만 표시되는지. 인접 Object의 마지막 DST를 빨간 사각형으로
  잘못 표시하지 않는지
- OVER ACTiVE DX+ 7keys처럼 SRC가 1만 개 이상인 스킨에서 Object Browser와 Image
  Manager를 열고 scroll해도 화면 밖 행을 전부 제출할 때의 심한 입력 지연이 없는지.
  Object 선택 자동 scroll, hover, 우클릭, drag reorder도 그대로 동작하는지
- Asset Browser에서 사용 중인 Asset과 Object `#SRC`에서 파생된 Asset은 Delete가
  차단되는지. 미사용 `$SRC_IMAGE`는 Delete key/우클릭 확인 modal로 제거되며 실제
  texture 파일은 남고, 저장·재로드 후 crop만 사라지는지
- 사용 중인 Asset을 우클릭해 `Used by Objects`의 항목을 선택하면 Object Browser와
  Inspector가 열리고 해당 Object로 자동 스크롤되는지
- Object와 Asset을 선택하고 `Use in selected Object`를 누르면 SRC가 하나인
  Object는 즉시 `gr/x/y/w/h`가 교체되는지. 같은 명령이 card 우클릭에도 있는지
- Object에 적용 가능한 SRC가 여러 개면 modal이 뜨고, 선택한 SRC 행만 바뀌는지
- `Copy animation`을 끄면 기존 `div_x/div_y/cycle/timer`와 NUMBER/SLIDER/BUTTON
  고유 값이 유지되고, 켜면 animation 네 필드만 Asset 원본 값으로 바뀌는지
- Asset 적용과 Object Inspector의 Tagged image 변경이 각각 Ctrl+Z 한 번으로
  SRC 행 전체를 복구하며 Object/Inspector/Preview/Image Manager 선택이 유지되는지
- Image Manager의 같은 gr texture 후보에 그 논리 gr를 사용하는 고유 Object 수가
  표시되는지
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
- 기존 IF/ELSEIF/ELSE 안의 gr 번호가 생성 전후 동일하고, 새 gr가 현재 OP에서
  LR2가 실제 활성화한 `#IMAGE` 수로 배정되는지. tricoro처럼 1P/2P가 서로 배타적인
  연속 `#IF` 블록이면 두 블록의 수를 합산하지 않고, 생성 Asset/썸네일은 바로 앞의
  새 `#IMAGE` 선언을 가리키는지
- `Merge image`가 선택 Asset crop 전체가 들어가는 첫 alpha=0 빈 영역을 자동으로
  찾아 합성하고, 빈 영역이 없으면 오른쪽/아래 확장 중 작은 canvas를 선택하는지
- `GIF to sprite...`에서 동일 delay GIF와 서로 다른 delay GIF를 각각 변환하고,
  부분 frame offset·투명도·disposal이 있는 GIF가 full canvas frame으로 올바르게
  합성되는지. 기존 PNG 경로는 덮어쓰지 않아야 한다.
- transparent color index가 있는 indexed GIF를 변환했을 때 해당 palette pixel이
  WIC의 BGRA 변환 결과와 무관하게 출력 PNG에서 alpha 0으로 남는지
- 서로 다른 delay가 안전하게 펼쳐지는 GIF는 modal의 cell 수가 delay 최대공약수에
  따른 복제 수와 같고, 등록된 `$SRC_IMAGE`의 `div_x/div_y/cycle`이 표시된
  grid/전체 시간과 일치하는지. 256 cell을 넘는 사례는 원본 frame 수와 timing 근사
  경고를 사용해야 한다.
- GIF를 CSV에 등록한 직후 Asset Browser에서 그 Asset을 Preview에 drop하면 New
  Object의 `#SRC_IMAGE`에 같은 `div_x/div_y/cycle`이 자동 입력되는지
- 이미지가 하나도 없는 스킨에서도 GIF 변환 modal을 열 수 있고, CSV 등록을 끄면
  PNG만 생성되며 켜면 trailing gr의 `#IMAGE`와 animated full-size Asset이 함께
  생기는지
- 638x388·147 frame처럼 완성 BGRA sheet가 100MiB를 넘는 GIF도 대형 스킨을 연
  Release Win32에서 4096x4096/16M pixel 안으로 자동 축소되어 변환되고, 등록 후 LR2
  graphic handle이 유효한지. modal에는 원본 frame과 축소된 출력 frame 크기가 함께
  보여야 하며 Asset을 Preview에 drop한 기본 DST는 원본 frame 크기를 유지해야 한다.
  변환 실패 시 프로세스가 종료되는 대신 modal에 메모리/경로 오류가
  표시되고 `.skineditor-gif.tmp`가 남지 않아야 한다.
- 현재 스킨과 같은 LR2 설치의 `LR2files` 아래 출력 파일은 CSV에
  `LR2files\...`로 저장되고 stock LR2에서도 유효한 graphic handle로 로드되는지.
  LR2files 바깥이지만 skin 폴더 안인 파일의 fallback 상대 경로도 ImageManager와
  Preview texture에서 올바르게 해석되는지
- tricoro처럼 include Object를 선택한 상태에서 새 editor-only Asset을 drop해도
  새 Object가 해당 Asset 선언 뒤에 배치되고 저장 전 Preview에 실제 texture가 보이는지
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
- 위 스킨을 해상도 modal로 저장하면 `#INFORMATION` 6·7번 칸이 1280,720이 되고
  활성 `#RESOLUTION`은 `$OLR_IGNORED_RESOLUTION`로 바뀌는지
- 명시 해상도가 없고 lane은 화면 왼쪽에 있지만 좌상단 1280x720 backdrop이 있는
  스킨이 TenRiff 규칙으로 HD 판정되며 toolbar/status에 `inferred`로 표시되는지
- 화면 밖 애니메이션/전환 panel이 backdrop으로 오인되어 HD/FHD로 승격되지 않는지
- 추정 상태로 열기만 하면 원본이 바뀌지 않고, modal Apply 뒤에만 명시
  `#INFORMATION`으로 기록되는지
- PLAY 5/7/9/10/14 keys, 5/7/9 battle
- SELECT, DECIDE, RESULT
- 해상도 변경 즉시 원본 파일에 반영되는지
- 로드 후 toolbar 해상도 modal에서도 같은 즉시 저장 정책이 적용되고 정상적으로
  재로드되는지
- PLAY 무음 내장 chart note가 정상적으로 내려오는지
- 14KEYS Simple Preview가 `sample_14.bme`를 LR2 Skin Select와 같은
  `scratchSide=0`으로 읽어 1P/2P 샘플 lane이 대칭으로 표시되는지
- COURSERESULT를 연 뒤 같은 Workspace에서 PLAY Simple Preview를 열어도 로그에
  빈 `courseFilepath`와 `BMSを開けません`이 나타나지 않는지
- Start 전 PLAY 정적 Preview에서 LN 몸체 뒤에 가려지는 가운데 단노트가 표시되지
  않고, LN이 없는 lane의 단노트 샘플은 그대로 유지되는지

### H. New 프리셋

- 각 지원 Scene/key mode로 새 폴더를 만든다.
- 기존 파일이 있는 경로는 덮어쓰지 않는지 확인한다.
- 생성 직후 skin이 열리고 Preview가 표시되는지 확인한다.
- `#SCENETIME`은 DECIDE에만 있고 PLAY, SELECT, RESULT, COURSERESULT에는
  생성되지 않는지 확인한다.
- PLAY의 note/mine/LN, lane별 Bomb, measure line/judge line/gauge/FAST/SLOW와 플레이어별
  NOWJUDGE/NOWCOMBO를 확인한다. BMS note 판정 시 해당 lane Bomb과 판정/콤보가
  정해진 시간 뒤 사라지는지도 확인한다.
- PLAY BGA에는 zero-sized `#SRC_BGA`와 `#DST_BGA`가 이 순서로 한 쌍 존재하고,
  실제 BMS의 정지 이미지/동영상 BGA가 지정한 영역 안에 표시되는지 확인한다.
- 생성한 PLAY/BATTLE 스킨을 실제 LR2에서 열고 BMS 재생을 시작했을 때
  Measure Line 누락으로 `ProcI_Play` 접근 위반이 발생하지 않는지 확인한다.
- 생성한 main에는 해상도가 `#INFORMATION` 6·7번 칸에만 있고 활성
  `#RESOLUTION`이 없어, LR2 스킨 목록 진입/전환 시 종료되지 않는지 확인한다.
- Groove Gauge가 50칸으로 lane 폭을 채우고, gauge 값 변화에 따라 밝은 채움과
  어두운 빈칸 및 80% 경계 색상이 바뀌는지 확인한다.
- PLAY 생성 직후 첫 Note가 Object Browser에서 자동 선택·스크롤되고 Inspector에
  같은 Note의 Normal/Mine/LN/DST_NOTE 행이 함께 표시되는지 확인한다.
- 공용 `preset.bmp`의 NUMBER source가 알 수 없는 색/기호가 아니라 또렷한 숫자
  `0~9`로 표시되는지 확인한다. LR2 number frame 대응상 atlas cell 순서는 0부터다.
- SELECT bar가 NOWJUDGE용 무지개 strip을 늘린 형태가 아니라 어두운 단색 면과 밝은
  테두리로 표시되고, OFF/ON tint와 title이 정상인지 확인한다. DECIDE panel/flash와
  `$st 10`의 실제 곡 제목 TEXT가 중앙에 표시되는지 확인한다. RESULT는 전용 dark panel 위에
  EX SCORE/MAX COMBO와 PERFECT/GREAT/GOOD/BAD/POOR label 및 값이 좌우 column으로
  정렬되고, 하단 gauge/score chart가 각 backdrop 안에 표시되는지 확인한다. 일반
  Groove Gauge의 result graph는 80% 미만에서 초록색, 80% 이상에서 빨간색으로 바뀌며 두 구간 모두
  backdrop 안에서 아래쪽 기준으로 위로 그려지는지 확인한다.
- COURSERESULT에서 1~5스테이지 제목/레벨과 EX Score, Max Combo,
  Perfect/Great/Good/Bad/Poor 누적 숫자를 확인한다.
- 임의 해상도에서 화면 밖으로 심하게 벗어나는 필수 Object가 없는지 확인한다.
- 생성 CSV에서 BGA와 font-backed TEXT/BAR_TITLE을 제외한 모든 raster Object의
  `SRC w/div_x`, `SRC h/div_y`가 `DST w`, `DST h`와 같은지 확인한다. ImageManager의
  crop과 Preview가 같은 native-size sprite를 보여야 하며, DST 배치는 이전과 같아야 한다.

### I. 인코딩과 저장

- 일본어/한국어 제목과 Customize label이 깨지지 않는지
- TextEdit의 문자열 표시와 편집 후 저장이 CP932를 유지하는지
- Save As merged/split 양쪽을 확인한다.
- Save As 성공 후 추가 편집이 새 main script에 저장되는지
- 실패 또는 overwrite 취소 시 원본이 유지되는지

## 장애 기록 형식

재현 가능한 문제는 다음 내용을 남긴다.

```text
Build: AI_2 branch commit/dirty status, Release Win32
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
