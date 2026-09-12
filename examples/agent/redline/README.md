# REDLINE — Combat Terminal

스타크래프트 1의 붉은 프레임/녹색 군용 터미널 분위기에서 착안한
1280 x 720 PLAY 7KEY + SCRATCH 스킨. 원작 로고, 이미지, 폰트, 음원은 포함하지 않는다.
금속 프레임은 내장 imagegen으로 새로 생성한 독자 그래픽이다. 원본 PNG는 그대로
사용하고 LR2의 SRC/DST로 배율과 상단 가림 영역을 지정한다. 프롬프트는
`armored-console-prompt.txt`, 원본은 `armored-console.png`에 남겼다.
실시간 숫자와 판정은 Windows Bahnschrift로 래스터화하며 폰트 파일은 배포하지 않는다.

## 사용

압축을 별도 폴더에 풀고 `LR2files/Theme/REDLINE-Combat-Terminal-7K`를
LR2의 동일 경로에 새 폴더로 복사한다. PLAY 7KEY에서 REDLINE을 선택한다.
동명 폴더가 있다면 덮어쓰지 말고 먼저 보관한다. 이 작업은 설치를 자동 수행하지 않는다.
편집기에서는 해당 폴더의 `REDLINE-7K.lr2skin`을 연다.

- 붉은 스크래치, 밝은 일반 건반/짙은 녹색 검은 건반, 녹색 롱노트, 주황 지뢰.
- 판정 6종과 상대좌표 NOWCOMBO, 판정 타이머 기반 녹색 폭발 효과.
- 실제 EX SCORE/BPM/COMBO/MAX COMBO/FAST/SLOW/GROOVE 수치 연결.
- 50칸 게이지, 장갑 프레임에 매립한 검은 BGA 화면.
- `preview.png`는 정지 샘플, `states/index.html`은 PLAY 고정 상태 갤러리.
- 네이티브 parser 및 Preview는 확인한다. 실제 LR2 곡 플레이는 미검증이다.
  정지 Preview의 개별 샘플 수치는 실제 플레이 중 동기화 수치를 뜻하지 않는다.

## 재생성

AI_experimental의 유틸 포함 실행 파일과 Windows Bahnschrift, Python/Pillow가 필요하다.
`build_redline.py`는 기존 비트맵 글자 helper를 재사용한다. 새 폴더만 허용한다.

```powershell
python -B examples/agent/redline/build_redline.py --editor SkinEditor_DX9/Release/SkinEditor_DX9.exe --out .build/redline-new
```

범용 JSON 생성 후, 예제 전용 후처리로 gauge 간격/noshift를 지정하고 실제 숫자
오브젝트를 추가한다. 재생성에는 JSON만이 아니라 이 예제 스크립트도 필요하다.
기존 스킨과 편집기 코드, OLRskin 0.9 포맷은 수정하지 않는다.

## 이번 제작 검증

- 38개 Object의 고유 ID와 노트/LN/지뢰의 index 0..7 구성을 검사했다.
- 네이티브 Preview 14개 고정 상태를 모두 렌더링했고 최대 숫자 화면을 직접 확인했다.
- 진단에는 특수 명령의 bounds_unknown/special_runtime_semantics와 비활성 bomb timer가
  남는다. BGA의 프리셋 zero-size SRC도 crop 경고를 내므로 진단 0건이라는 뜻은 아니다.
- 실제 곡 BGA, 입력, 플레이 시작부터 종료까지의 LR2 검증은 아직 하지 않았다.
