#include "Interpreter.hpp"
#include <iomanip>

int main(int argc, char **argv)
{
	snake_args args {};

	args.save_filename = "./test";
	args.load_filename = "./test";
	args.iters_between_saves = 100000;
	args.n_iters = 1000000;
	args.print_run = true;
	args.print_interval_s = .3;
	args.max_adjacent_0s = 0;
	args.temperature = 1;
	args.nenvs = 128;

	args.timeout = 142;


	VecSnakeGame vec_env = VecSnakeGame(args.nenvs, args.max_adjacent_0s, args.board_size, args.timeout);
	QTable<std::string> table = QTable<std::string>(args.discount_factor, args.learning_rate);

	if (args.load_filename.size() > 0) table.load(args.load_filename);

	run(vec_env, table, args);
}
