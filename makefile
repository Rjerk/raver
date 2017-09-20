objects = main.o ServiceManager.o HTTPService.o HTTPParser.o HTTPRequest.o HTTPResponse.o  HTTPConnection.o Channel.o Acceptor.o IOManager.o Poller.o ThreadPool.o Timer.o TimeStamp.o Logger.o Utils.o RJson.o

flags = -Wall -O -std=c++11 -pthread

all: server

server: $(objects)
	g++ $(flags) -o server $(objects)

main.o: main.cpp
	g++ $(flags) -c main.cpp

ServiceManager.o: ServiceManager.cpp
	g++ $(flags) -c ServiceManager.cpp

HTTPService.o: HTTPService.cpp
	g++ $(flags) -c HTTPService.cpp

HTTPConnection.o: HTTPConnection.cpp
	g++ $(flags) -c HTTPConnection.cpp

HTTPResponse.o: HTTPResponse.cpp
	g++ $(flags) -c HTTPResponse.cpp

HTTPRequest.o: HTTPRequest.cpp
	g++ $(flags) -c HTTPRequest.cpp

HTTPParser.o: HTTPParser.cpp
	g++ $(flags) -c HTTPParser.cpp

Channel.o: Channel.cpp
	g++ $(flags) -c Channel.cpp

Acceptor.o: Acceptor.cpp
	g++ $(flags) -c Acceptor.cpp

IOManager.o: IOManager.cpp
	g++ $(flags) -c IOManager.cpp

Poller.o: Poller.cpp
	g++ $(flags) -c Poller.cpp

ThreadPool.o: ThreadPool.cpp
	g++ $(flags) -c ThreadPool.cpp

Timer.o: Timer.cpp
	g++ $(flags) -c Timer.cpp

TimeStamp.o: TimeStamp.cpp
	g++ $(flags) -c TimeStamp.cpp

Logger.o: Logger.cpp
	g++ $(flags) -c Logger.cpp

RJson.o: RJson.cpp
	g++ $(flags) -c RJson.cpp

Utils.o: Utils.cpp
	g++ $(flags) -c Utils.cpp

.PHONY: clean

clean:
	rm -f server $(objects)
