FLAGS=-std=c++17 -Wall -Wextra -Wpedantic -Werror

all: main.cpp
	g++ main.cpp -o main $(FLAGS)

run: main
	./main