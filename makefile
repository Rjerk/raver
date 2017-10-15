objects = main.o ServiceManager.o HTTPService.o HTTPParser.o HTTPRequest.o \
		  HTTPResponse.o HTTPConnection.o Channel.o Acceptor.o IOManager.o EPoller.o \
		  ThreadPool.o Logger.o Utils.o RJson.o FileCache.o Buffer.o

flags = -Wall -Wextra -Werror -Wconversion -Wno-unused-parameter -O -std=c++11 -pthread

all: server

server: $(objects)
	g++ $(flags) -o server $(objects)

main.o: main.cpp
	g++ $(flags) -c main.cpp

ServiceManager.o: http/ServiceManager.cpp
	g++ $(flags) -c http/ServiceManager.cpp

HTTPService.o: http/HTTPService.cpp
	g++ $(flags) -c http/HTTPService.cpp

HTTPConnection.o: http/HTTPConnection.cpp
	g++ $(flags) -c http/HTTPConnection.cpp

HTTPResponse.o: http/HTTPResponse.cpp
	g++ $(flags) -c http/HTTPResponse.cpp

HTTPRequest.o: http/HTTPRequest.cpp
	g++ $(flags) -c http/HTTPRequest.cpp

HTTPParser.o: http/HTTPParser.cpp
	g++ $(flags) -c http/HTTPParser.cpp

Channel.o: http/Channel.cpp
	g++ $(flags) -c http/Channel.cpp

Acceptor.o: http/Acceptor.cpp
	g++ $(flags) -c http/Acceptor.cpp

IOManager.o: http/IOManager.cpp
	g++ $(flags) -c http/IOManager.cpp

EPoller.o: http/EPoller.cpp
	g++ $(flags) -c http/EPoller.cpp

ThreadPool.o: base/ThreadPool.cpp
	g++ $(flags) -c base/ThreadPool.cpp

#Timer.o: base/Timer.cpp
#	g++ $(flags) -c base/Timer.cpp

#TimeStamp.o: base/TimeStamp.cpp
#	g++ $(flags) -c base/TimeStamp.cpp

Logger.o: base/Logger.cpp
	g++ $(flags) -c base/Logger.cpp

RJson.o: base/RJson.cpp
	g++ $(flags) -c base/RJson.cpp

FileCache.o: base/FileCache.cpp 
	g++ $(flags) -c base/FileCache.cpp

Buffer.o: base/Buffer.cpp 
	g++ $(flags) -c base/Buffer.cpp

Utils.o: base/Utils.cpp
	g++ $(flags) -c base/Utils.cpp

.PHONY: clean

clean:
	rm -f server $(objects)
