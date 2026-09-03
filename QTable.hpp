
#include "string"
#include "vector"
#include "unordered_map"


template<typename StateT = std::string>
class QTable
{
private:
	std::unordered_map<StateT, std::vector<float>> table;

	const float discount_factor;
	const float learning_rate;

public:
	static const size_t N_actions;

	QTable(float discount_factor, float learning_rate);

	std::vector<float> get_Q_values(StateT state);
	void update_Q_values(std::vector<std::tuple<StateT, size_t, float>> state_action_index_reward);

	void save(std::string filename);
	void load(std::string filename);
};

template <typename StateT>
inline std::vector<float> QTable<StateT>::get_Q_values(StateT state)
{
	std::vector<float> out;
	if (auto search = table.find(state); search != table.end())
		out = search->second;
	else
	{
		out.resize(N_actions);
		table.insert({state, out})
	}

	return out;
}

template <typename StateT>
inline void QTable<StateT>::update_Q_values(std::vector<std::tuple<StateT, size_t, float>> t_values)
{
	for (size_t i = 0; i < t_values.size(); i++)
	{
		float discounted_reward = 0;
		{float discount = 1;
		for (size_t j = i; j < t_values.size(); j++)
		{
			auto [_, _, r] = t_values[i];

			discounted_reward += r * discount;
			discount *= discount_factor;
		}}
		auto [_, _, current_expected] = table.at(StateT);

		float new_expectation = (1 - learning_rate) * current_expected + learning_rate * discounted_reward;
	}
		
}
