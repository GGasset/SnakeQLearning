#include "Interpreter.hpp"
#include <iomanip>


// TODO: Simplify Snake body

int main(int argc, char **argv)
{
	snake_args args {};

	args.save_training_max_score = true;

	args.load_filename = "./test93_penalize_edge";
	args.save_filename = "./test93_penalize_edge";
	args.iters_between_saves = 20000;
	args.n_iters = 100000000;

	args.deterministic_actions = false;//argc > 1;
	args.print_run = argc > 1;
	args.print_interval_s = .1;
	args.nenvs = 128;
	args.discount_factor = .8;
	
	args.max_adjacent_0s = 1;
	args.max_adjacent_body = 1;

	args.learning_rate = .1;

	args.timeout = 4269;

	args.temperature = args.print_run ? .2 : .5;
	//args.temperature = args.print_run ? .0271 : .7;


	VecSnakeGame vec_env = VecSnakeGame(args.nenvs, args, args.board_size, args.timeout);
	QTable<std::string> table = QTable<std::string>(args.discount_factor, args.learning_rate);

	if (args.load_filename.size() > 0) table.load(args.load_filename);

	run(vec_env, table, args);
}
