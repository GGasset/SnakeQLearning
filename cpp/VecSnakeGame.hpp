
#pragma once

#include <cstddef>
#include "string"
#include "vector"
#include "iostream"

#include "functionality.hpp"

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
	std::vector<std::string> game;

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
	std::vector<std::string> high_score;

	snake_args args;

	size_t timeout;

	SnakeGameData generate_board();

	void set_at_random_empty_pos(SnakeGameData board, size_t &pos);

	char get_at_cell(size_t env_i, size_t cell_i);
	char get_at_cell(SnakeGameData board, size_t cell_i);

public:
	VecSnakeGame(size_t nenvs, snake_args args,  size_t board_size, size_t timeout);
	size_t get_nenvs();
	inline SnakeGameData &get_board_data(size_t env_i) { return envs[env_i]; }

	std::string get_state(size_t env_i);
	action_result update(size_t env_i, size_t action_i);

	std::string to_string(size_t env_i);

	bool is_head_at_edge(size_t env_i);

	
};

std::string remove_consecutives(std::string in, char c, size_t max_consecutives);
