import gymnasium as gym
import numpy as np
from UE_PPO import Agent
from UE_env import PPO_environment
import torch
import socket

n_games = 3000
N = 500
n_steps = 0
learn_iters = 0

alpha = 0.003
n_epochs = 8
batch_size = 128
num_input = 41

if __name__ == '__main__':
    env = PPO_environment(0, num_input)

    agent = Agent(n_actions=5, batch_size=batch_size,
                  alpha=alpha, n_epochs=n_epochs,
                  input_dims=(num_input,))
    agent.load_models()
    n_games = 300

    best_score = env.reward_range[0]
    score_history = []

    learn_iters = 0
    avg_score = 0
    n_steps = 0

    HOST = '127.0.0.1'
    PORT = 5050

    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        client.connect((HOST, PORT))
        print("successfully connected")
    except socket.error:
        print("couldn't connect", socket.error)

    for i in range(n_games):
        observation = env.reset(client)
        done = False
        score = 0
        while not done:
            action, prob, val = agent.choose_action(observation)
            observation_, reward, done = env.step(client, action)
            n_steps += 1
            score += reward
            observation = observation_
        score_history.append(score)
        avg_score = np.mean(score_history[-100:])

        print('episode', i, 'score %.1f' % score, 'avg score %.1f' % avg_score,
              'time_steps', n_steps, 'learning_steps', learn_iters)