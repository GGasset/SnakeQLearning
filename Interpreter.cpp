#include "Interpreter.hpp"

uint64_t xorshift64() {
	static uint64_t x = (uint64_t)time(0);
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	return x;
}

// Uses softmax function
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

			sleep(args.print_interval_s);
		}
		return;
	}

	std::vector<std::vector<std::tuple<std::string, size_t, float>>> state_action_reward;
	state_action_reward.resize(vec_env.get_nenvs());
	for (size_t iter_i = 0; iter_i < args.n_iters; iter_i++)
	{
		for (size_t env_i = 0; env_i < vec_env.get_nenvs(); env_i++)
		{
			std::string state = vec_env.get_state(env_i);
			size_t action_i = get_action(table.get_Q_values(state), args);
			action_result result = vec_env.update(env_i, action_i);

			float reward = 0;
			switch (result)
			{
			case action_result::size_increase:
				reward = 1;
				break;
			case action_result::size_decrease:
				reward = -.9;
				break;
			case action_result::game_over:
				reward = -2;
				break;
			case action_result::game_won:
				reward = 2;
			case action_result::nothing:
			default:
				reward = .02;
				break;
			}

			state_action_reward[env_i].push_back({state, action_i, reward});

			if (result == game_over || result == game_won)
			{
				table.update_Q_values(state_action_reward[env_i]);
				state_action_reward[env_i].clear();
			}
		}
	}
}