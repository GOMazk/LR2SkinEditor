# LR2 SkinEditor

LR2 스킨의 CSV 스크립트, 이미지 소스, 배치 정보와 조건 분기를 한 화면에서
편집하고 Preview로 확인하는 Windows/DX9 편집기입니다.

현재 개발 기준은 `AI` 브랜치의 Release x86 빌드입니다. 이 작업 트리에는
Object Editor와 현대화 UI 관련 미커밋 변경이 포함될 수 있으므로, 다른 PC에서
작업을 이어가기 전에 반드시 `git status`와 `git diff`를 먼저 확인하십시오.

## 문서

- [현재 개발 상태와 설계](docs/PROJECT_STATE.md)
- [빌드, 실행 및 회귀 테스트](docs/BUILD_AND_TEST.md)
- [UI 구조와 디버깅 규칙](docs/UI_ARCHITECTURE.md)

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

Visual Studio 2022 Developer PowerShell에서:

```powershell
cd D:\Github\SkinEditor\SkinEditor_DX9
msbuild SkinEditor_DX9.sln /t:Build /p:Configuration=Release /p:Platform=x86 /m
```

출력 파일은 `SkinEditor_DX9\Release\SkinEditor_DX9.exe`입니다. 자세한 준비물과
검증 절차는 [빌드 문서](docs/BUILD_AND_TEST.md)를 따르십시오.
