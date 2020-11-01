FLAGS=-std=c++17 -Wall -Wextra

all: main.cpp
	g++ main.cpp -o main $(FLAGS)

run: main
	./main