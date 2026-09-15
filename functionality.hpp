
#pragma once

#include "string"

struct snake_args
{
	bool print_run = false;
	float print_interval_s = .5;

	float learning_rate = .01;
	float discount_factor = .98;
	float temperature = 1;

	size_t max_adjacent_0s = 0;
	size_t max_adjacent_body = 1;
	size_t board_size = 10;
	size_t nenvs = 64;
	size_t n_iters = 1e4;
	size_t timeout = 40;

	std::string load_filename = "";

	std::string save_filename = "";
	size_t iters_between_saves = 50;
};

uint64_t xorshift64();