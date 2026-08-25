# 빌드, 실행 및 회귀 테스트

## 개발 환경

- Windows
- Visual Studio 2022 C++ toolchain (`v143`)
- Windows 10 SDK
- Project: `SkinEditor_DX9\SkinEditor_DX9.vcxproj`
- 기본 검증 구성: `Release | Win32(x86)`
- Direct3D 9 / DxLib

프로젝트에는 Win32와 x64 구성이 모두 있지만 현재 배포와 반복 검증 기준은 x86다.
새 PC에서는 Visual Studio Installer에서 **Desktop development with C++**와
Windows 10/11 SDK를 설치한다.

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
2. New/Open toolbar와 Windows menu를 열고 Text Editor 항목이 있는지 확인한다.
   스킨 목록에는 별도의 `LOAD(TEXT)` 버튼이 없어야 한다.
3. 640x480 스킨 하나와 1280x720 이상 HD 스킨 하나를 각각 연다.
4. Preview, Image Manager, Asset Browser, DST View, Object Browser, Object Inspector가 보이는지
   확인한다. DST View를 처음 열었을 때 오른쪽 미리보기 열이 남은 폭을 즉시
   채우며, 폭을 확보하기 위해 splitter를 먼저 드래그할 필요가 없어야 한다.
5. Preview, Image Manager, DST View 캔버스 위에서 Ctrl+MouseWheel을 사용해
   마우스 아래의 이미지 지점이 움직이지 않은 채 확대/축소되는지 확인한다.
   Fit과 100%도 확인하고, 100% 초과 확대에서 선형 보간으로 흐려지지 않고
   원본 픽셀 경계가 선명하게 표시되는지 확인한다.
6. Object 속성 하나를 변경하고 Ctrl+Z로 복구한다.
7. 변경 후 toolbar가 `MODIFIED`, Ctrl+S 성공 후 `SAVED`인지 확인한다.
8. Save As의 BROWSE가 파일 선택기를 열고, 성공 후 새 경로가 workspace의 현재
   경로가 되는지 확인한다.
9. toolbar의 해상도 버튼에서 크기를 바꾸면 즉시 저장·재로드되며 Object 좌표가
   자동 확대되지 않는지 확인한다. 미저장 script/pixel edit 중에는 Apply가
   차단되는지도 확인한다.

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
- symbolic 이름이 없는 Object가 op 이름, 그마저 없으면 non-zero timer 이름을
  쓰는지
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
- PLAY 5/7/9/10/14 keys, 5/7/9 battle
- SELECT, DECIDE, RESULT
- 해상도 변경 즉시 원본 파일에 반영되는지
- 로드 후 toolbar 해상도 modal에서도 같은 즉시 저장 정책이 적용되고 정상적으로
  재로드되는지
- PLAY sample BMS note가 정상적으로 내려오는지

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
