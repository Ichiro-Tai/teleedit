all:
	g++ -g -lpthread -std=c++11 thread.cpp host.cpp -o host
