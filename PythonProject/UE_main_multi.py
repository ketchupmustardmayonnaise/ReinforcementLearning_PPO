from UE_multiEnv import UE_multi_env
from UE_PPO import Agent
import torch
from torch.utils.tensorboard import SummaryWriter
writer = SummaryWriter()

ENVCOUNT = 15
n_games = 30000000000
N = 500
n_steps = 0
learn_iters = 0

alpha = 0.003
n_epochs = 8
batch_size = 64
num_input = 40

multi_env = UE_multi_env(ENVCOUNT, num_input)
agent = Agent(n_actions=5, batch_size=batch_size,
            alpha=alpha, n_epochs=n_epochs,
            input_dims=(num_input,))

episode_history = [[] for _ in range(ENVCOUNT)]
episode_rewards = [0 for x in range(ENVCOUNT)]
#agent.load_models()
states = []
saving = 0

for id in range(ENVCOUNT):
    states.append(multi_env.reset(id))

for timestep in range(n_games):
    actions = []
    probs = []
    vals = []

    for id in range(ENVCOUNT):
        action, prob, value = agent.choose_action(states[id])
        actions.append(action)
        probs.append(prob)
        vals.append(value)

    n_states, rewards, dones = multi_env.step(actions)

    for i in range(ENVCOUNT):
        agent.remember(states[i], actions[i], probs[i], vals[i], rewards[i], dones[i])
        episode_rewards[i] += rewards[i]
        n_steps += 1

        if dones[i]:
            if i == 0:
                writer.add_scalar("Reward", episode_rewards[i], len(episode_history[i]))
            multi_env.reset(i)
            episode_history[i].append(episode_rewards[i])
            episode_rewards[i] = 0

    probs.clear()
    actions.clear()
    states.clear()
    vals.clear()

    states = n_states #다음 스텝으로 넘어감

    if n_steps % N == 0:
        agent.learn()
        learn_iters += 1
        agent.save_models(saving)
        saving += 1

    if timestep % 1000 == 0:
        print("timestep: " + str(timestep))