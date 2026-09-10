#include "Interpreter.hpp"

uint64_t xorshift64() {
	static uint64_t x = (uint64_t)time(0);
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	return x;
}

size_t get_action(std::vector<float> raw_logits, snake_args args)
{
	float max_raw_logit = raw_logits[0];
	for (size_t i = 1; i < raw_logits.size(); i++) if (raw_logits[i] > max_raw_logit) max_raw_logit = raw_logits[i];
	
	for (size_t i = 0; i < raw_logits.size(); i++) raw_logits[i] -= max_raw_logit;
	

	float exp_sum = 0;
	for (size_t i = 0; i < raw_logits.size(); i++) exp_sum += exp(raw_logits[i] / (args.temperature + 1e-8));

	std::vector<float> probabilities;
	for (size_t i = 0; i < raw_logits.size(); i++) probabilities.push_back(exp(raw_logits[i] / (args.temperature + 1e-8)));

	size_t action_i = 0;
	float value = (xorshift64() % 10000) / 10000.0;
	float selection_probability = 0;
	for (size_t i = 0; i < probabilities.size() && value < selection_probability; i++)
	{
		selection_probability += probabilities[i];
		action_i = i;
	}
	return action_i;
}

void run(VecSnakeGame &vec_env, QTable<std::string> &table, snake_args args)
{
	if (args.print_run)
	{
		std::cout << vec_env.to_string(0);

		action_result result = nothing;
		while (result != game_over && result != game_won)
		{
			std::string state = vec_env.get_state(0);
			std::vector<float> Q_values = table.get_Q_values(state);
			size_t action_i = get_action(Q_values, args);
			result = vec_env.update(0, action_i);

			std::cout << vec_env.to_string(0);

			sleep(.5);
		}
		return;
	}

	for (size_t i = 0; i < args.n_iters; i++)
	{

	}
}