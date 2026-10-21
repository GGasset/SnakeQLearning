
from typing import Optional
import numpy as np
from numpy.typing import ArrayLike
import gymnasium as gym


class SnakeEnv(gym.Env):
    def __init__(self, size=10):
        self.size: int = size

        self.snake_positions: list[int] = []
        self.green_apple_pos: tuple[int, int] = (-1, -1)
        self.red_apple_pos: int = -1

        self.timeout: int = 50
        self.time_since_eat: int = 0

        self.observation_space = gym.spaces.Discrete(size * 2, dtype=np.int32)
        # Define what actions are available (4 directions)
        self.action_space = gym.spaces.Discrete(4)

        # Map action numbers to actual movements on the grid
        # This makes the code more readable than using raw numbers
        self._action_to_direction = {
            0: 1,   # Move right (column + 1)
            1: -self.size,  # Move up (row - 1)
            2: -1,  # Move left (column - 1)
            3: self.size,   # Move down (row + 1)
        }

    def _get_obs(self):
        out = []

        horizontal_indices: list[int] = [int(self.size * (self.snake_positions[0] / self.size) + i) for i in range(self.size)]
        vertical_indices: list[int] = [int(self.size * i + self.snake_positions[0] % self.size) for i in range(self.size)]

        obs_indices = horizontal_indices + vertical_indices

        for indice in obs_indices:
            out.append(self.get_at_cell(indice))

        return np.array(out)

    def _get_info(self):
        """Compute auxiliary information for debugging.

        Returns:
            dict: Info with distance between agent and target
        """
        return {"info 0": 0}

    def reset(self, seed: Optional[int] = None, opts: Optional[dict] = None):
        """Start a new episode.

        Args:
            seed: Random seed for reproducible episodes
            options: Additional configuration (unused in this example)

        Returns:
            tuple: (observation, info) for the initial state
        """
        # IMPORTANT: Must call this first to seed the random number generator
        super().reset(seed=seed)



        observation = self._get_obs()
        info = self._get_info()

        return observation, info

    def step(self, action):
        """Execute one timestep within the environment.

        Args:
            action: The action to take (0-3 for directions)

        Returns:
            tuple: (observation, reward, terminated, truncated, info)
        """

        self.time_since_eat += 1

        # Map the discrete action (0-3) to a movement direction
        direction = self._action_to_direction[action]

        old_head_pos = self.snake_positions[0]
        new_head_pos = self.snake_positions[0] + direction
        scaped_hor = new_head_pos < 0 or new_head_pos > self.size * self.size
        scaped_ver = direction == -1 and old_head_pos % self.size == 0
        scaped_ver = scaped_ver or direction == 1 and old_head_pos % self.size == self.size - 1

        front_obj = self.get_at_cell(new_head_pos)
        terminated = scaped_hor or scaped_ver or front_obj == ord('S')
        terminated = terminated or (front_obj == ord('R') and len(self.snake_positions) == 1)

        truncated = self.time_since_eat >= self.timeout

        terminated = terminated or truncated

        # Simple reward structure: +1 for reaching target, 0 otherwise
        reward = int(front_obj == ord('G'))
        reward -= int(front_obj == ord('R') or terminated)

        observation = self._get_obs()
        info = self._get_info()


        if terminated:
            self.time_since_eat = 0
            return observation, reward, terminated, truncated, info

        self.snake_positions.insert(0, new_head_pos)
        if front_obj != ord('G'):
            self.snake_positions.pop()
        if front_obj == ord('R'):
            self.snake_positions.pop()
        return observation, reward, terminated, truncated, info


    def render(self):
        """Render the environment for human viewing."""
        if self.render_mode == "human":
            # Print a simple ASCII representation
            for y in range(self.size - 1, -1, -1):  # Top to bottom
                row = ""
                for x in range(self.size):
                    if np.array_equal([x, y], self._agent_location):
                        row += "A "  # Agent
                    elif np.array_equal([x, y], self._target_location):
                        row += "T "  # Target
                    else:
                        row += ". "  # Empty
                print(row)
            print()

    def get_at_cell(self, cell_i: int) -> int:
        out = ord('0')

        for pos in self.snake_positions:
            out += (ord('S') - out) * int(cell_i == pos)
        out += (ord('H') - out) * int(cell_i == self.snake_positions[0])

        for green_pos in self.green_apple_pos:
            out += (ord('G') - out) * int(cell_i == green_pos)
        out +=  (ord('R') - out) * int(cell_i == self.red_apple_pos)

        return out


# Register the environment so we can create it with gym.make()
gym.register(
    id="gymnasium_env/SnakeEnv-v0",
    entry_point=SnakeEnv,
    max_episode_steps=300,  # Prevent infinite episodes
)
