
from typing import Optional
import numpy as np
import numpy.random as rand

# from numpy.typing import ArrayLike
import gymnasium as gym


class SnakeEnv(gym.Env):
    def __init__(self, size=10):
        self.size: int = size

        self.snake_positions: list[int] = []
        self.green_apple_pos: dict[int, int] = {0: -1, 1: -1}
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

        self.prev_action = -1

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

        self.green_apple_pos[0] = -1
        self.green_apple_pos[1] = -1
        self.red_apple_pos = -1
        self.snake_positions = []

        self.place_objs()


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
        terminated = terminated or (front_obj == ord('R') and
                                    len(self.snake_positions) == 1)

        truncated = self.time_since_eat >= self.timeout

        terminated = terminated or truncated

        # Simple reward structure: +1 for reaching target, 0 otherwise
        reward = int(front_obj == ord('G'))
        reward -= int(front_obj == ord('R') or terminated)

        if terminated:
            self.time_since_eat = 0
            return self._get_obs(), reward, terminated, truncated, self._get_info()

        self.snake_positions.insert(0, new_head_pos)
        if front_obj != ord('G'):
            self.snake_positions.pop()
            if self.green_apple_pos[0] == new_head_pos:
                self.green_apple_pos[0] = -1
            else:
                self.green_apple_pos[1] = -1
        if front_obj == ord('R'):
            self.snake_positions.pop()
            self.red_apple_pos = -1

        self.place_objs()

        self.prev_action = action
        observation = self._get_obs()
        info = self._get_info()
        return observation, reward, terminated, truncated, info

    def place_objs(self):
        if self.red_apple_pos == -1:
            self.red_apple_pos = self.get_empty_random_pos()
        for green_apple_key in self.green_apple_pos.keys():
            if self.green_apple_pos[green_apple_key] == -1:
                self.green_apple_pos[green_apple_key] = self.get_empty_random_pos()

        if self.snake_positions.__len__() > 0:
            return

        head_pos = rand.random_integers(0, self.size - 1)
        direction = rand.random_integers(0, 3)
        new_snake_positions = [head_pos + self._action_to_direction[direction] * i for i in range(3)]

        lowest_position = new_snake_positions[0]
        highest_position = new_snake_positions[0]

        for pos in new_snake_positions:
            lowest_position += (pos - lowest_position) * int(pos < lowest_position)
            highest_position += (pos - highest_position) * int(pos > highest_position)


        vertical_shift: int = 0
        if lowest_position < 0:
            vertical_shift = int(abs(lowest_position) / self.size)
        if highest_position > self.size:
            vertical_shift = int(-(highest_position - self.size) / self.size)
        
        horizontal_shift: int = 0
        different_row = lowest_position / self.size != highest_position / self.size
        if direction == 0 or direction == 2 and different_row:
            horizontal_shift = -self._action_to_direction[direction]
            horizontal_shift *= self.size - abs(lowest_position % self.size - highest_position % self.size)

        snake_shift: int = vertical_shift * self.size + horizontal_shift
        self.snake_positions = [pos + snake_shift for pos in new_snake_positions]

    def get_empty_random_pos(self):
        while True:
            out = rand.random_integers(0, self.size - 1)
            if chr(self.get_at_cell(out) == ' '):
                return out

    def render(self):
        """Render the environment for human viewing."""
        if self.render_mode == "human":
            out = ''
            for i in range(self.size):
                if i % self.size == 0:
                    out += "\033[0m\n"
                cell = chr(self.get_at_cell(i))
                to_add = ''
                if cell == 'H':
                    to_add += "\033[38;2;114;132;208m"
                    to_add += "██"
                elif cell == 'S':
                    to_add += "\033[38;2;44;132;208m"
                    to_add += "██"
                elif cell == 'G':
                    to_add += "🍏"
                elif cell == 'R':
                    to_add += "🍎"
                else:
                    to_add += "\033[38;2;77;77;77m"
                    to_add += "██"
                out += to_add
            out += "\033[0m\n"
            return out

    def get_at_cell(self, cell_i: int) -> int:
        out = ord('0')

        for pos in self.snake_positions:
            out += (ord('S') - out) * int(cell_i == pos)
        out += (ord('H') - out) * int(cell_i == self.snake_positions[0])

        for green_pos in self.green_apple_pos:
            out += (ord('G') - out) * int(cell_i == green_pos)
        out += (ord('R') - out) * int(cell_i == self.red_apple_pos)

        return out


# Register the environment so we can create it with gym.make()
gym.register(
    id="gymnasium_env/SnakeEnv-v0",
    entry_point=SnakeEnv,
    max_episode_steps=300,  # Prevent infinite episodes
)
