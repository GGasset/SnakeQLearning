import gymnasium as gym

from stable_baselines3 import PPO
from stable_baselines3.common.env_util import make_vec_env
from stable_baselines3.common.vec_env import VecFrameStack
from os import getcwd
from os.path import exists
from sys import argv

import snake_env

training = not '-v' in argv

nenvs = 32 * int(training) + int(not training)
print(f'number of parallel environments: {nenvs}')

# Parallel environments
vec_env = make_vec_env("SnakeEnv", n_envs=nenvs, env_kwargs={'render_mode': 'ansi'})

policy_kwargs={'net_args': [128, 128, 128, 128]}
model = PPO("MlpPolicy", vec_env, verbose=1, device='cpu')

if exists("ppo_snake.zip") and not '-nl' in argv:
    model = PPO.load("ppo_snake", vec_env, device='cpu')
    print('Loaded')
if training:
    while True:
        model.learn(total_timesteps=100000, progress_bar=True)
        model.save("ppo_snake")


from time import sleep
obs = vec_env.reset()
print('Finished training')
while True:
    action, _states = model.predict(obs)
    obs, rewards, dones, info = vec_env.step(action)
    vec_env.render("ansi")
    sleep(1 / 10)