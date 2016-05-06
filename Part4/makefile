all: server

server: Create.cpp Tweet.cpp Delete.cpp Follow.cpp Signin.cpp server1.cpp Create.h Delete.h Follow.h Tweet.h Signin.h
	g++ -std=c++11 server1.cpp Create.cpp Delete.cpp Follow.cpp Tweet.cpp Signin.cpp -o server -pthread

clean:
	rm -f server


