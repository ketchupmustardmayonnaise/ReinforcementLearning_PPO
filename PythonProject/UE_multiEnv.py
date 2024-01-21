from UE_env import PPO_environment
import socket

HOST = '127.0.0.1'
PORT = 5050

class UE_multi_env:
    def __init__(self, num_env, num_input):
        self.envs = []

        for _ in range(num_env):
            env = PPO_environment(_, num_input)
            self.envs.append(env)

        self.hasConnection = False
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        try:
            self.socket.connect((HOST, PORT))
            print("successfully connected")
            self.hasConnection = True
        except socket.error:
            print("couldn't connect", socket.error)

    def reset(self, id):
        state = self.envs[id].reset(self.socket)
        return state

    def step(self, actions):
        observations = []
        rewards = []
        dones = []

        for i in range(len(actions)):
            env = self.envs[i]
            action = actions[i]
            observation, reward, done = env.step(self.socket, action)
            observations.append(observation)
            rewards.append(reward)
            dones.append(done)

            if done:
                env.reset(self.socket)

        return observations, rewards, dones