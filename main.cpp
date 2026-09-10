#include "Interpreter.hpp"

int main(int argc, char **argv)
{
	snake_args args;


	VecSnakeGame vec_env = VecSnakeGame(args.nenvs, args.max_adjacent_0s, args.board_size);
	QTable<std::string> table = QTable<std::string>(args.discount_factor, args.learning_rate);

	if (args.load_filename.size() > 0) table.load(args.load_filename);

	run(vec_env, table, args);
}
