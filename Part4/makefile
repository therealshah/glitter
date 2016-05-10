all: server1 server2

server1: Create.cpp Tweet.cpp Delete.cpp Follow.cpp Signin.cpp server1.cpp Create.h Delete.h Follow.h Tweet.h Signin.h
	g++ -std=c++11 server1.cpp Create.cpp Delete.cpp Follow.cpp Tweet.cpp Signin.cpp -o server1 -pthread

server2: Create.cpp Tweet.cpp Delete.cpp Follow.cpp Signin.cpp server2.cpp Create.h Delete.h Follow.h Tweet.h Signin.h
	g++ -std=c++11 server2.cpp Create.cpp Delete.cpp Follow.cpp Tweet.cpp Signin.cpp -o server2 -pthread


clean:
	rm -f server1 server2


