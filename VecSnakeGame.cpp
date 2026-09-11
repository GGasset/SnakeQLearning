#include "VecSnakeGame.hpp"

SnakeGameData VecSnakeGame::generate_board()
{
	SnakeGameData out {};

	size_t head_pos = rand() % (board_size * board_size);
	size_t head_pos_x = head_pos % board_size;
	size_t head_pos_y = head_pos / board_size;
	
	bool can_pointing_up = head_pos_y < board_size - 2;
	bool can_pointing_right = head_pos_x >= 2;
	bool can_pointing_down = head_pos_y >= 2;
	bool can_pointing_left = head_pos_x < board_size - 2;
	
	if (!can_pointing_up && !can_pointing_down && !can_pointing_left && !can_pointing_right) return generate_board();

	out.snake_positions.push_back(head_pos);
	if (can_pointing_up)
	{
		out.snake_positions.push_back((head_pos_y - 1) * board_size + head_pos_x);
		out.snake_positions.push_back((head_pos_y - 2) * board_size + head_pos_x);

		out.direction = 0;
	}
	else if (can_pointing_right)
	{
		out.snake_positions.push_back(head_pos_y * board_size + (head_pos_x - 1));
		out.snake_positions.push_back(head_pos_y * board_size + (head_pos_x - 2));

		out.direction = 1;
	}
	else if (can_pointing_down)
	{
		out.snake_positions.push_back((head_pos_y + 1) * board_size + head_pos_x);
		out.snake_positions.push_back((head_pos_y + 2) * board_size + head_pos_x);

		out.direction = 3;
	}
	else
	{
		out.snake_positions.push_back(head_pos_y * board_size + (head_pos_x + 1));
		out.snake_positions.push_back(head_pos_y * board_size + (head_pos_x + 2));

		out.direction = 4;
	}

	out.green_apple_pos[0] = -1;
	out.green_apple_pos[1] = -1;
	out.red_apple_pos = -1;

	int n_inserted_green_apples = 0;
	while (n_inserted_green_apples < 2)
	{
		set_at_random_empty_pos(out, out.green_apple_pos[n_inserted_green_apples]);

		n_inserted_green_apples++;
	}

	set_at_random_empty_pos(out, out.red_apple_pos);
	return out;
}

// Check if there is space at the board
void VecSnakeGame::set_at_random_empty_pos(SnakeGameData board, size_t &pos)
{
	pos = -1;

	size_t candidate = rand() % (board_size * board_size);
	char cell;
	while ((cell = get_at_cell(board, candidate)) != '0')
	{
		candidate++;
		candidate *= (candidate < board_size * board_size);
	}
	pos = candidate;
}

char VecSnakeGame::get_at_cell(size_t env_i, size_t i)
{
	return get_at_cell(envs[env_i], i);
}

char VecSnakeGame::get_at_cell(SnakeGameData board, size_t i)
{
	char out = '0';
	out += ('H' - out) * (board.snake_positions[0] == i);
	for (size_t j = 1; j < board.snake_positions.size() && out == '0'; j++)
		out += ('S' - out) * (board.snake_positions[j] == i);
	out += ('G' - out) * (board.green_apple_pos[0] == i);
	out += ('G' - out) * (board.green_apple_pos[1] == i);
	out += ('R' - out) * (board.red_apple_pos == i);

	return out;
}

VecSnakeGame::VecSnakeGame(size_t nenvs, size_t max_adjacent_0s, size_t board_size, size_t timeout)
{
	this->max_adjacent_0s = max_adjacent_0s;
	this->board_size = board_size;
	this->timeout = timeout;

	for (size_t i = 0; i < nenvs; i++)
		envs.push_back(generate_board());
}

size_t VecSnakeGame::get_nenvs()
{
	return envs.size();
}

std::string VecSnakeGame::get_state(size_t env_i)
{
	std::string vision[2];
	SnakeGameData board = envs[env_i];

	size_t head_pos = board.snake_positions[0];

	size_t head_pos_x = head_pos % board_size;
	size_t head_pos_y = head_pos / board_size;

	vision[0] += 'W';
	vision[1] += 'W';
	for (size_t i = 0; i < board_size; i++)
	{
		// Fill vision along the y axis (horizontally)
		vision[0] += get_at_cell(env_i, board_size * head_pos_y + i);

		// Fill vision along the x axis (vertically)
		vision[1] += get_at_cell(env_i, board_size * i + head_pos_x); 
	}
	vision[0] += 'W';
	vision[1] += 'W';

	std::string concat = vision[0] + "H" + vision[1];

	std::string proc_vision[4];
	for (size_t i = 0, v_i = 0; i < concat.size(); i++)
	{
		bool is_head = concat[i] == 'H';
		if (is_head)
		{
			proc_vision[v_i] += '#';
			v_i++;
			continue;
		}

		proc_vision[v_i] += concat[i];
	}

	std::string out;
	int dir = board.direction;
	switch (dir)
	{
	case 0: // Up
		out += proc_vision[2];
		out += proc_vision[1];
		out += proc_vision[3];
		out += proc_vision[0];
		break;
	case 1: // Right
		out += proc_vision[1];
		out += proc_vision[3];
		out += proc_vision[0];
		out += proc_vision[2];
		break;
	case 2: // Down
		out += proc_vision[3];
		out += proc_vision[0];
		out += proc_vision[2];
		out += proc_vision[1];
		break;
	case 3: // Left
		out += proc_vision[0];
		out += proc_vision[2];
		out += proc_vision[1];
		out += proc_vision[3];
		break;
	
	default:
		std::cerr << "Invalid direction\n";
		throw;
		break;
	}

	// Simplify state
	size_t consecutive_0_count = 0;
	for (size_t i = 0; i < out.size(); i++)
	{
		if (out[i] != '0')
		{
			consecutive_0_count = 0;
			continue;
		}

		if (consecutive_0_count >= max_adjacent_0s)
		{
			out.erase(out.begin() + i);
			i--;
			continue;
		}

		consecutive_0_count++;
	}

	return out;
}

action_result VecSnakeGame::update(size_t env_i, size_t action_i)
{
	SnakeGameData &board = envs[env_i];
	board.episode_len++;
	

	if (board.snake_positions.size() >= board_size * board_size - 1 - 4) 
	{
		board = generate_board();;
		return action_result::game_won;
	}

	size_t head = board.snake_positions[0];
	board.direction = board.direction - (action_i == 0) + (action_i == 2);
	if (board.direction < 0) board.direction = 3;
	else if (board.direction > 3) board.direction = 0;

	size_t head_x = head % board_size;
	int dx = (board.direction == 1) - (board.direction == 3);

	size_t head_y = head / board_size;
	int dy = (board.direction == 0) - (board.direction == 2);

	size_t new_x = head_x + dx;
	size_t new_y = head_y + dy;
	if (new_x >= board_size || new_y >= board_size)
	{
		board = generate_board();
		return action_result::game_over;
	}

	size_t new_pos = board_size * new_y + new_x;

	char object_at_new_pos = get_at_cell(board, new_pos);
	if (object_at_new_pos == 'S')
	{
		board = generate_board();
		return action_result::game_over;
	}

	// Insert head
	board.snake_positions.insert(board.snake_positions.begin(), new_pos);
	if (object_at_new_pos == 'G')
	{
		board.time_since_eat = 0;

		size_t apple_i = board.green_apple_pos[1] == new_pos;
		size_t &apple_pos = board.green_apple_pos[apple_i];

		set_at_random_empty_pos(board, apple_pos);

		return action_result::size_increase;
	}

	board.snake_positions.pop_back();
	if (object_at_new_pos == 'R')
	{
		set_at_random_empty_pos(board, board.red_apple_pos);

		board.snake_positions.pop_back();

		if (!board.snake_positions.size())
		{
			board = generate_board();
			return action_result::game_over;
		}

		return action_result::size_decrease;
	}

	board.time_since_eat++;

	if (board.time_since_eat >= timeout)
	{
		board = generate_board();
		return action_result::game_over;
	}

	return action_result::nothing;
}

std::string VecSnakeGame::to_string(size_t env_i)
{
	std::string out;
	
	// Clear screen
	out += "\033[2J";

	// Set background color to black
	//out += "\033[48;7;10;9m";
	for (size_t i = 0; i < board_size * board_size; i++)
	{
		if (i % board_size == 0) out += '\n';
		char c = get_at_cell(env_i, i);
		switch (c)
		{
		case 'H':
			out += "\e[0;34m";
			break;
		case 'S':
			out += "\e[0;36m";
			break;
		case 'G':
			out += "\e[0;32m";
			break;
		case 'R':
			out += "\e[0;31m";
			break;
		case '0':
		default:
			out += "\e[0;30m";
			break;
		}
		//out += "█";
		out += c;
	}
	out += "\n\e[0;37m";
	return out;
}
