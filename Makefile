
NAME=LearnToSlither

CPP_FILES=functionality.cpp Interpreter.cpp main.cpp VecSnakeGame.cpp
O_FILES=$(patsubst %.cpp,%.o,$(CPP_FILES))

all: $(O_FILES)
	g++ -o $(NAME) $(O_FILES)

%.o: %.cpp
	g++ -O3 -g -std=c++17 -o $@ -c $?

clean:
	rm -f $(O_FILES)

.PHONY: all
