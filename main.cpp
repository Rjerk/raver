#include "./http/ServiceManager.h"
#include "./http/HTTPService.h"
#include "./base/RJson.h"

#include <iostream>

int main(/*int argc, char** argv*/)
{
    rjson::RJSON parser(readFile("config.json"));
    auto ret = parser.parseJson(); (void)ret;
    assert(ret == rjson::PARSE_OK);
    auto value = parser.getValue();
    assert(value->getType() == rjson::RJSON_OBJECT);
    auto thread_num = value->getValueFromObject("thread-num")->getNumber();
    auto port = value->getValueFromObject("port")->getNumber();
    /*
    if (argc != 3) {
        std::cerr << "Usage: ./raver <port> <num-thread>" << std::endl;
        exit(1);
    }

    int port = atoi(argv[1]);
    int num_thread = atoi(argv[2]);
    */

    raver::ServiceManager manager(thread_num);
    raver::HTTPService http_service(port, &manager);

    manager.run();
}
