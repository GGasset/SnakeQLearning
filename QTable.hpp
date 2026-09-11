
#pragma once

#include "iostream"
#include "fstream"
#include "string"
#include "vector"
#include "unordered_map"


template<typename StateT = std::string>
class QTable
{
private:
	std::unordered_map<StateT, std::vector<float>> table;

	float discount_factor;
	float learning_rate;

public:
	static const size_t N_actions = 3;

	QTable(float discount_factor, float learning_rate);

	std::vector<float> get_Q_values(StateT state);
	void update_Q_values(std::vector<std::tuple<StateT, size_t, float>> state_action_index_reward);

	void save(std::string filename);
	void load(std::string filename);
};

template <typename StateT>
inline QTable<StateT>::QTable(float discount_factor, float learning_rate)
{
	this->discount_factor = discount_factor;
	this->learning_rate = learning_rate;
}

template <typename StateT>
inline std::vector<float> QTable<StateT>::get_Q_values(StateT state)
{
	std::vector<float> out;
	if (auto search = table.find(state); search != table.end())
		out = search->second;
	else
	{
		out.resize( QTable::N_actions);
		table.insert({state, out});
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
			auto [_, __, r] = t_values[i];

			discounted_reward += r * discount;
			discount *= discount_factor;
		}}
		auto [state, action_i, _] = t_values[i];
		std::vector<float> &current_expected = table.at(state);

		current_expected[action_i] = (1 - learning_rate) * current_expected[action_i] + learning_rate * discounted_reward;
	}
		
}

template <typename StateT>
inline void QTable<StateT>::save(std::string filename)
{
	std::fstream s{filename, s.binary | s.trunc | s.in | s.out};

	if (!s.is_open()) { std::cerr << "ERROR opening \"" << filename << "\" for writing. Nothing has been done" << std::endl; return; }

	s << table.size();
	s << "\n";
	for (const auto& [key, value] : table)
	{
		s << key << " ";
		for (const auto& Q: value)
			s << Q << " ";
		s << std::endl;
	}
	s.close();
}

template <typename StateT>
inline void QTable<StateT>::load(std::string filename)
{
	std::fstream s{filename, s.binary | s.in | s.out};

	if (!s.is_open()) { std::cerr << "ERROR opening \"" << filename << "\" for reading. Nothing has been done" << std::endl; return; }

	std::unordered_map<std::string, std::vector<float>>::size_type n_items;

	char hole;

	s >> n_items;

	for (size_t i = 0; i < n_items && s.good(); i++)
	{
		StateT key;
		std::vector<float> value;
		value.resize(3);

		s >> key >> value[0] >> value[1] >> value[2];

		table[key] = value;
	}
}
