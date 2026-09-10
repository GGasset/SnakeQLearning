
#include "unistd.h"
#include "cmath"
#include "random"

#include "VecSnakeGame.hpp"
#include "QTable.hpp"

struct snake_args
{
	bool print_run = false;
	float print_interval = .5;

	float learning_rate = .01;
	float discount_factor = .98;
	float temperature = .1;

	size_t max_adjacent_0s = 2;
	size_t board_size = 10;
	size_t nenvs = 64;
	size_t n_iters;

};

void run(VecSnakeGame &vec_env, QTable<std::string> &table, snake_args);
