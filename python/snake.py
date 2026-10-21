
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
            0: np.array(1),   # Move right (column + 1)
            1: np.array(-self.size),  # Move up (row - 1)
            2: np.array(-1),  # Move left (column - 1)
            3: np.array(self.size),   # Move down (row + 1)
        }

    def _get_obs(self):
        out = []

        horizontal_indices = [self.size * self.snake_positions[0][1] + i for i in range(self.size)]

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
        # Map the discrete action (0-3) to a movement direction
        direction = self._action_to_direction[action]

        # Update agent position, ensuring it stays within grid bounds
        # np.clip prevents the agent from walking off the edge
        self._agent_location = np.clip(
            self._agent_location + direction, 0, self.size - 1
        )

        # Check if agent reached the target
        terminated = np.array_equal(self._agent_location, self._target_location)

        # We don't use truncation in this simple environment
        # (could add a step limit here if desired)
        truncated = False

        # Simple reward structure: +1 for reaching target, 0 otherwise
        reward = 1 if terminated else 0

        observation = self._get_obs()
        info = self._get_info()

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

    def get_at_cell(cell_i: int) -> str:
        pass


# Register the environment so we can create it with gym.make()
gym.register(
    id="gymnasium_env/SnakeEnv-v0",
    entry_point=SnakeEnv,
    max_episode_steps=300,  # Prevent infinite episodes
)
