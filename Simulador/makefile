CXX = g++

CXXFLAGS = -std=c++17 -Wall -Wextra

main: main.o Process.o IOs.o ListController.o
	$(CXX) $(CXXFLAGS) -o main main.o Process.o IOs.o ListController.o

main.o: enum.hh Process.hh declares.hh IOs.hh
	$(CXX) $(CXXFLAGS) -c main.cc

ListController.o: ListController.hh Process.hh

Process.o: Process.hh enum.hh declares.hh

IOs.o: IOs.hh enum.hh

clean:
	rm *.o main

run: main
	./main
