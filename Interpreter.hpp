
#pragma once

#include "unistd.h"
#include "cmath"
#include "random"
#include "iostream"
#include "math.h"
#include <iomanip>

#include "VecSnakeGame.hpp"
#include "QTable.hpp"


void run(VecSnakeGame &vec_env, QTable<std::string> &table, snake_args);
