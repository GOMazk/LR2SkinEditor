# LR2 SkinEditor

LR2 스킨의 CSV 스크립트, 이미지 소스, 배치 정보와 조건 분기를 한 화면에서
편집하고 Preview로 확인하는 Windows/DX9 편집기입니다.

현재 개발 기준은 `AI_2` 브랜치의 Release x86 빌드입니다. 이 작업 트리에는
Object Editor와 현대화 UI 관련 미커밋 변경이 포함될 수 있으므로, 다른 PC에서
작업을 이어가기 전에 반드시 `git status`와 `git diff`를 먼저 확인하십시오.

## 문서

- [현재 개발 상태와 설계](docs/PROJECT_STATE.md)
- [빌드, 실행 및 회귀 테스트](docs/BUILD_AND_TEST.md)
- [UI 구조와 디버깅 규칙](docs/UI_ARCHITECTURE.md)
- [OLR Skin V0.7 포맷](docs/OLRSKIN_FORMAT.md)
- [외부 코드 고지](docs/THIRD_PARTY_NOTICES.md)
- [AI/UI 지도 생성 및 활용](docs/UI_MAP.md)
- [AI 협업 시작 가이드](docs/AI_COLLABORATION.md)
- [변경 인계 템플릿](docs/HANDOFF_TEMPLATE.md)

## 핵심 데이터 흐름

```text
skinHelper.txt                  skinObjGroup.txt
Command/argument schema        Object grouping schema
          \                         /
           +----> Object Model <---+
                     |
          +----------+----------+
          |          |          |
   Object Editor   CSV/Text   Preview/Image/DST
```

개발 중에는 두 스키마 파일을 외부 TXT에서 읽습니다. 파일이 없으면 실행 파일에
포함된 RCDATA를 사용하므로 배포본은 별도의 `skinHelper.txt`와
`skinObjGroup.txt` 없이도 실행할 수 있습니다.

## 빠른 빌드

저장소 루트에서 현재 기준인 `Release | Win32` 빌드와 자동 테스트를 실행합니다.
산출물과 로그는 모두 `.build` 아래에 생성됩니다.

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\ui-map.ps1 -Check
powershell -ExecutionPolicy Bypass -File .\scripts\docs-check.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\test.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\ai-context.ps1 -Check
```

64비트 실행 파일이 필요하면 같은 환경에서 다음 명령을 추가로 실행합니다. x86
배포 폴더를 덮지 않고 `SkinEditor_DX9\Release-x64\SkinEditor_DX9.exe`에 출력됩니다.

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Platform x64
```

GitHub Actions도 같은 스크립트로 Release x86 테스트와 Release x64 빌드를 확인하므로
로컬과 CI의 검증 경로가 같습니다.
다른 AI에게 작업을 넘길 때는 `.build\ai-context\context-pack.md`와
`context-manifest.json`을 함께 전달하십시오.

수동으로 MSBuild를 실행해야 한다면 Visual Studio 2022 Developer PowerShell에서:

```powershell
cd D:\Github\SkinEditor\SkinEditor_DX9
msbuild SkinEditor_DX9.sln /t:Build /p:Configuration=Release /p:Platform=x86 /m
```

출력 파일은 `SkinEditor_DX9\Release\SkinEditor_DX9.exe`입니다. 자세한 준비물과
검증 절차는 [빌드 문서](docs/BUILD_AND_TEST.md)를 따르십시오.
