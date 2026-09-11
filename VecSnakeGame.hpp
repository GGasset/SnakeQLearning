
#pragma once

#include <cstddef>
#include "string"
#include "vector"
#include "iostream"

enum action_result
{
	nothing,
	game_over,
	game_won,
	size_increase,
	size_decrease,
};

struct SnakeGameData
{
	std::vector<size_t> snake_positions;
	size_t green_apple_pos[2];
	size_t red_apple_pos;

	// 0 = up, continues is clockwise
	int direction;

	size_t episode_len = 0;
	size_t time_since_eat = 0;
};

class VecSnakeGame
{
private:
	size_t board_size;
	std::vector<SnakeGameData> envs;

	// Determines max the number of adjacent 0s, if this number is surpassed, the extra 0s will be removed
	size_t max_adjacent_0s;

	size_t timeout;

	SnakeGameData generate_board();

	void set_at_random_empty_pos(SnakeGameData board, size_t &pos);

	char get_at_cell(size_t env_i, size_t cell_i);
	char get_at_cell(SnakeGameData board, size_t cell_i);

public:
	VecSnakeGame(size_t nenvs, size_t max_adjacent_0s, size_t board_size, size_t timeout);
	size_t get_nenvs();
	inline SnakeGameData get_board_data(size_t env_i) { return envs[env_i]; }

	std::string get_state(size_t env_i);
	action_result update(size_t env_i, size_t action_i);

	std::string to_string(size_t env_i);

};
