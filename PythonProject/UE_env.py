import gymnasium as gym
import numpy as np
import struct

HOST = '127.0.0.1'
PORT = 5050

class PPO_environment(gym.Env):
    def __init__(self, id, num_input):
        self.id = id
        self.num_input = num_input
        self.action_space = gym.spaces.Discrete(5)
        self.minimum_distance = 25
        # 1. 앞으로 갈 것인지 말 것인지 (2)
        # 2. 회전할 것인지 아닌지 (3)
        # 3. 점프할 것인지 아닌지 (2)
        self.observation_space = gym.spaces.Dict(
            {
                "observationDistance[0 - wall]": gym.spaces.Box(low=0, high=25),
                "observationDistance[0 - stair]": gym.spaces.Box(low=0, high=25),

                "observationDistance[1 - wall]": gym.spaces.Box(low=0, high=25),
                "observationDistance[1 - stair]": gym.spaces.Box(low=0, high=25),

                "observationDistance[2 - wall]": gym.spaces.Box(low=0, high=25),
                "observationDistance[2 - stair]": gym.spaces.Box(low=0, high=25),

                "observationDistance[3 - wall]": gym.spaces.Box(low=0, high=25),
                "observationDistance[3 - stair]": gym.spaces.Box(low=0, high=25),

                "observationDistance[4 - wall]": gym.spaces.Box(low=0, high=25),
                "observationDistance[4 - stair]": gym.spaces.Box(low=0, high=25),

                "observationDistance[5 - wall]": gym.spaces.Box(low=0, high=25),
                "observationDistance[5 - stair]": gym.spaces.Box(low=0, high=25),

                "observationDistance[6 - wall]": gym.spaces.Box(low=0, high=25),
                "observationDistance[6 - stair]": gym.spaces.Box(low=0, high=25),

                "observationDistance[7 - wall]": gym.spaces.Box(low=0, high=25),
                "observationDistance[7 - stair]": gym.spaces.Box(low=0, high=25),

                "observationDistance[8 - wall]": gym.spaces.Box(low=0, high=25),
                "observationDistance[8 - stair]": gym.spaces.Box(low=0, high=25),

                "observationDistance[9 - wall]": gym.spaces.Box(low=0, high=25),
                "observationDistance[9 - stair]": gym.spaces.Box(low=0, high=25),

                "observationDistance[10 - wall]": gym.spaces.Box(low=0, high=25),
                "observationDistance[10 - stair]": gym.spaces.Box(low=0, high=25),

                "observationDistance[11 - wall]": gym.spaces.Box(low=0, high=25),
                "observationDistance[11 - stair]": gym.spaces.Box(low=0, high=25),

                "observationDistance[12 - wall]": gym.spaces.Box(low=0, high=25),
                "observationDistance[12 - stair]": gym.spaces.Box(low=0, high=25),

                "observationDistance[13 - wall]": gym.spaces.Box(low=0, high=25),
                "observationDistance[13 - stair]": gym.spaces.Box(low=0, high=25),

                "observationDistance[14 - wall]": gym.spaces.Box(low=0, high=25),
                "observationDistance[14 - stair]": gym.spaces.Box(low=0, high=25),

                "observationDistance[15 - wall]": gym.spaces.Box(low=0, high=25),
                "observationDistance[15 - stair]": gym.spaces.Box(low=0, high=25),

                "time": gym.spaces.Box(low=-1, high=1),
                "goalDistance": gym.spaces.Box(low=0, high=25),
                "goalAngle": gym.spaces.Box(low=-180, high=180),

                "movingSpeed": gym.spaces.Discrete(4),
                "isJumping": gym.spaces.Discrete(2),
                "isCollide": gym.spaces.Discrete(2),
                "CharacterGround": gym.spaces.Discrete(5),
                "isReached": gym.spaces.Discrete(3, start=-1)
            }
        )

        ''' 여기부터 소켓통신 부분 주석'''
        #self.hasConnection = False
        #self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        #try:
        #    self.socket.connect((HOST, PORT))
        #    print("connect")
        #    self.hasConnection = True
        #except socket.error:
        #    print("failed to connect: ", socket.error)

    def step(self, client, action):
        # 시행할 action을 python -> unreal 전송
        # 전송할 배열
        array = np.array([self.id, action], dtype=np.float32)
        # 배열 크기 추가
        #array = np.insert(array, 0, len(action))
        # 배열 크기 및 내용 전송
        array_bytes = array.tobytes()
        client.sendall(array_bytes)

        # 이제 state를 python -> unreal 받아 옴
        state = []
        while True:
            array_size_bytes = client.recv(4)
            if not array_size_bytes: break
            array_size = self.num_input

            array_data_bytes = client.recv(array_size * 4)
            array_data = np.frombuffer(array_data_bytes, dtype=np.float32)
            state = array_data.copy()
            break

        state = list(map(int, state))
        #if self.id == 0:
        #    print(state)

        reward = 0

        # 점프 억제
        #if state[70] == 1:
        #    reward -= 0.05

        # if round(state[39]) == 1:
        #     reward += 0.2
        # elif round(state[39]) == 2:
        #     reward += 0.3
        # elif round(state[39]) == 3:
        #     reward += 0.4
        # elif state[36] == 1 and state[39] == 4:
        #     reward += 0.2

        if state[33] < self.minimum_distance:
            self.minimum_distance = state[33]
            dist_reward = 0.03
        else: dist_reward = -0.0005

        time_reward = 0
        if round(state[32]) < 0:
            time_reward = 3 * state[32] / 10000 # -0.0003 ~ 0

        collide_reward = 0
        if round(state[37]) == 1:
            collide_reward = -0.005

        reward = dist_reward + time_reward + collide_reward

        # goal에 도착하면 reward
        done = False
        if round(state[39]) == 1:
            done = True
            reward = 1
            self.minimum_distance = 25
        elif round(state[39]) == -1:
            done = True
            self.minimum_distance = 25

        return state, reward, done

    def reset(self, client):
        array = np.array([self.id, 10], dtype=np.float32)
        array_bytes = array.tobytes()
        client.sendall(array_bytes)

        while True:
            array_size_bytes = client.recv(4)
            if not array_size_bytes: break
            #array_size = struct.unpack('I', array_size_bytes)[0]
            array_size = self.num_input
            array_data_bytes = client.recv(array_size * 4)
            array_data = np.frombuffer(array_data_bytes, dtype=np.float32)
            break

        state = [25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
                 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
                 100, 0, 25, 0, 0, 0, 0, 0]
        return state