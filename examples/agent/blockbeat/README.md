# BLOCKBEAT — OVERWORLD v0.1

마인크래프트의 잔디·돌·광석에서 착안한 LR2 7키 + 스크래치 플레이 스킨.
1280×720 기준이며, 기존 스킨을 수정하지 않는 독립 폴더로 제공한다.

## 열기

- 편집기에서 `LR2files/Theme/BLOCKBEAT-Overworld-v01/BLOCKBEAT-Overworld-7K.lr2skin`을 연다.
- `preview.png`는 실제 편집기 렌더러로 만든 정적 미리보기다. 샘플 노트/수치가 표시된다.
- LR2에서 사용하려면 압축을 별도 폴더에 푼 뒤, `LR2files/Theme/BLOCKBEAT-Overworld-v01`만
  LR2의 같은 경로에 **새 폴더로** 복사하고 PLAY 7KEY 스킨에서 선택한다. 동명 폴더가 이미
  있으면 덮어쓰지 않는다. 이 작업에서는 설치나 기존 스킨 교체를 수행하지 않았다.

## 구성

- 석영색 흰 건반 노트, 다이아몬드색 검은 건반 노트, 금색 스크래치.
- 에메랄드 롱노트, 붉은 지뢰, 픽셀 파티클 판정 효과, 건반을 누를 때 켜지는 빛.
- 돌 블록 레인 테두리, 핫바 형태의 50칸 게이지, 숲·강·광산 일러스트.
- EX SCORE, BPM, COMBO, MAX COMBO, FAST/SLOW, 게이지 퍼센트는 실제 LR2 숫자에 연결된다.
- BGA가 있는 곡에서는 우측 풍경 영역에 곡의 BGA가 표시된다.
- 7K 플레이 장면 전용이다. 선택/결과/더블 플레이 스킨은 포함하지 않는다.

## 제작과 검증

`AI_experimental`의 `tools/skin_agent.py`로 기본 스킨을 만든 뒤, 예제 전용
`build_blockbeat.py`가 새 출력물에 원본 픽셀 UI/노트 아틀라스와 LR2 행을 작성한다.
기존 네이티브 프리셋의 노트 index, LINE, LN, gauge, 판정 timer를 유지했다.
콤보는 LR2의 판정 위치에 대한 상대 좌표를 사용하고 숫자 정렬은 전체 자릿수 폭을 반영한다.
이미지 import를 범용 유틸의 새 API로 추가하지 않고 한 스킨의 제작 예제로 분리했다.
OLRskin 0.9 포맷에는 변경이 없다.

실행에는 Python + Pillow, AI 유틸을 포함한 Win32 편집기 실행 파일이 필요하다.
저장소 루트에서 실행하며, 기존 출력 폴더는 거부한다.

```powershell
python -B examples/agent/blockbeat/build_blockbeat.py --editor .build/agent-utility-win32-20260912/SkinEditor_DX9.exe --out .build/blockbeat-new
```

제작 흐름은 기본 프리셋을 생성한 뒤 아틀라스 작성, 네이티브 parser readback,
네이티브 PNG 렌더링 순서로 진행한다. 최종 폴더의 `build-report.json`과
`SHA256SUMS.txt`로 결과와 파일 무결성을 확인할 수 있다. `recipe.json`은 기본
배치 지시이며 전체 테마 재생성에는 예제 스크립트와 일러스트 파일도 필요하다.
47개 Object, 8개 레인의 노트/LN/지뢰 대응, PNG 영역과 frame 분할, 숫자 바인딩을
검사한다. 정적 미리보기의 게이지 채움/숫자/BPM은 편집기가 각자 제공하는 샘플 값이다.
실제 LR2에서 곡을 플레이한 검증은 아직 하지 않았다.

## 아트

- `overworld.png`: Codex 내장 imagegen으로 생성한 테마용 일러스트. 원본 PNG를 그대로
  복사하고 LR2가 화면에 맞게 그린다. CLI/API fallback은 사용하지 않았다.
- 프롬프트: `art-prompt.txt`.
- `blockbeat-atlas.png`: 예제 코드로 만든 픽셀 UI, 글자, 노트, 숫자, 판정/게이지.
- Minecraft의 원본 텍스처·로고·게임 파일은 포함하지 않는다. Mojang/Microsoft와 관련된
  공식 제품이 아닌 팬 테마다.
