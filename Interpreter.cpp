#include "Interpreter.hpp"
#include <algorithm>

// Uses softmax function
size_t get_action(std::vector<float> raw_logits, snake_args args, std::vector<float> *out_probabilities = 0)
{
	float max_raw_logit = raw_logits[0];
	for (size_t i = 1; i < raw_logits.size(); i++) if (raw_logits[i] > max_raw_logit) max_raw_logit = raw_logits[i];
	
	for (size_t i = 0; i < raw_logits.size(); i++) raw_logits[i] -= max_raw_logit;


	float exp_sum = 0;
	for (size_t i = 0; i < raw_logits.size(); i++) exp_sum += exp(raw_logits[i] / (args.temperature + 1e-8));

	std::vector<float> probabilities;
	for (size_t i = 0; i < raw_logits.size(); i++) probabilities.push_back(exp(raw_logits[i] / (args.temperature + 1e-8)) / exp_sum);

	if (out_probabilities) *out_probabilities = probabilities;

	if (args.deterministic_actions)
	{
		size_t action_i = 0;
		float max_probability = 0;

		for (size_t i = 0; i < 0; i++)
		{
			action_i += (i - action_i) * (probabilities[i] > max_probability);
			max_probability += (probabilities[i] - max_probability) * (probabilities[i] > max_probability);
		}
		return action_i;
	}

	size_t action_i = 0;
	float value = (xorshift64() % 10000) / 10000.0;
	float selection_probability = 0;
	for (size_t i = 0; i < probabilities.size() && value > selection_probability; i++)
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
		std::string action_str[3] {"Right", "Straight", "Left"};

		std::cout << vec_env.to_string(0);

		action_result result = nothing;
		SnakeGameData d = vec_env.get_board_data(0);
		while (result != game_over && result != game_won)
		{
			// Clear screen

			args.timeout = d.snake_positions.size() * 5;

			std::string state = vec_env.get_state(0);
			std::vector<float> Q_values = table.get_Q_values(state);
			std::vector<float> probs;
			size_t action_i = get_action(Q_values, args, &probs);

			size_t len = vec_env.get_board_data(0).snake_positions.size();

			std::cout << "\033[2J";
			std::cout << vec_env.to_string(0);
			std::cout << "Len: " << len << std::endl;

			std::cout << "Action: " << action_str[action_i] << std::endl;

			for (size_t prob_i = 0; prob_i < probs.size(); prob_i++)
			{
				std::cout << action_str[prob_i] << ":\n\t\033[38;2;250;168;22m";
				for (size_t i = 0; i < probs[prob_i] * 10; i++) std::cout << "▆";
				std::cout << "\e[0m\n\n";
			}

			result = vec_env.update(0, action_i);

			/*if (result == action_result::game_over)
			{
				std::cout << "Last action: " << action_str[action_i] << std::endl;
				return;
			}*/

			usleep(args.print_interval_s * 1e6);
		}
		return;
	}

	size_t multi_step_mean_calculation_size = 600;
	float mean_r[multi_step_mean_calculation_size];
	
	for (size_t i = 0; i < multi_step_mean_calculation_size; i++)
	{
		mean_r[i] = 0;
	}

	size_t mean_death_len[vec_env.get_nenvs()];
	for (size_t i = 0; i < vec_env.get_nenvs(); i++)
	{
		mean_death_len[i] = 0;
	}

	long max_len = 0;

	std::vector<std::vector<std::tuple<std::string, size_t, float>>> state_action_reward;
	state_action_reward.resize(vec_env.get_nenvs());
	for (size_t iter_i = 0; iter_i < args.n_iters; iter_i++)
	{
		if (iter_i % args.iters_between_saves == 0 && iter_i)
			table.save(args.save_filename);

		float mean_reward = 0;
		for (size_t env_i = 0; env_i < vec_env.get_nenvs(); env_i++)
		{
			size_t current_len = vec_env.get_board_data(env_i).snake_positions.size();

			bool got_high_score = current_len > max_len;
			max_len += (current_len - max_len) * (current_len > max_len);

			SnakeGameData prev_board_data = vec_env.get_board_data(env_i);
			size_t time_since_eat = prev_board_data.time_since_eat;

			std::string state = vec_env.get_state(env_i);
			size_t action_i = get_action(table.get_Q_values(state), args);
			action_result result = vec_env.update(env_i, action_i);

			float reward = 0;
			switch (result)
			{
			case action_result::size_increase:
				//reward = time_since_eat > args.timeout / 2 ? 1 : time_since_eat < args.timeout / 3? 5: 2.5;
				reward = time_since_eat / args.timeout;
				reward = 2; //- reward * reward;
				//reward *= 5;
				//reward = (1 - time_since_eat / args.timeout);
				break;
			case action_result::size_decrease:
				reward = -.9;
				break;
			case action_result::game_over:
				mean_death_len[env_i] = current_len;
				reward = -5;
				//reward = -(float)time_since_eat / (args.timeout);
				break;
			case action_result::game_won:
				reward = 1;
			case action_result::nothing:
			default:
				reward = 1 - 2 * vec_env.is_head_at_edge(env_i) * (time_since_eat > 3) - (float)time_since_eat / (args.timeout);
				break;
			}
			mean_reward += reward;

			state_action_reward[env_i].push_back({state, action_i, reward});

			if (result == game_over || result == game_won)
			{
				table.update_Q_values(state_action_reward[env_i]);
				state_action_reward[env_i].clear();
			}
		}
		mean_reward /= vec_env.get_nenvs();
		mean_r[iter_i % multi_step_mean_calculation_size] = mean_reward;

		// Printing_secion
		if (iter_i % 1000) continue;

		//args.temperature = 7 / max_len;
		args.timeout = 200;//max_len * 420;//* max_len;

		mean_reward = 0;
		for (size_t i = 0; i < multi_step_mean_calculation_size; i++) mean_reward += mean_r[i];
		mean_reward /= multi_step_mean_calculation_size;

		float mean_len = 0;
		for (size_t i = 0; i < vec_env.get_nenvs(); i++) mean_len += mean_death_len[i];
		mean_len /= vec_env.get_nenvs();

		std::cout << std::setprecision(4);
		std::cout << mean_len << " | ";

		std::cout << std::setprecision(4);
		std::cout << mean_reward << " " << iter_i << " / " << args.n_iters << " - max_len: " << max_len << std::endl;
	}
}