## Reinforcement Learning - PPO Algorithm with Unreal Engine

2023 여름방학 때 강형엽 교수님의 연구실에서 강화학습을 공부하며,  
언리얼 엔진 내 Agent에 강화학습의 PPO 알고리즘을 적용시켜 보았지만...  
학습 속도가 너무 느려 보류 중인 프로젝트입니다  


## 실행 방법
1. UnrealProject 내 `RL_PPO.uproject` 파일을 실행시켜 언리얼 프로젝트를 엽니다.
2. 마찬가지로, PythonProject 내 `UE_main_multi.py`를 엽니다.
3. UE_main_multi.py를 실행시키긴 위해서는 필요 라이브러리들이 설치되어 있어야 합니다.
   ```
   pip install torch, gymnasium, numpy, socket, tensorboard
   ```
5. 환경이 모두 준비되었다면, 언리얼 프로젝트를 실행시킨 후 `UE_main_multi.py`를 실행시킵니다.  
   실행 순서가 잘못된다면 학습이 안 될 수도 있습니다!  
   Python의 cmd창에서 `successfully connected`, `timestep`이 뜨면서, 언리얼 엔진상 Agent들이 움직이면 학습 중인 겁니다.
6. `PythonProject > runs` 폴더로 이동하여,
   ```
   tensorboard --logdir=runs
   ```
   명령어를 입력하시어 링크를 클릭하시면, 학습이 잘되고 있는지 Reward 그래프를 보실 수 있습니다.
