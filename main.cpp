#include "./base/Logger.h"
#include "./base/RJson.h"
#include "./http/ServiceManager.h"
#include "./http/HTTPService.h"

int main()
{
    using namespace raver;
    using namespace rjson;

    Logger::setLevel(Logger::LogLevel::Trace);

    RJSON parser(readFile("config.json"));
    auto ret = parser.parseJson(); (void) ret;
    assert(ret == PARSE_OK);

    auto value = parser.getValue(); (void) value;
    assert(value->getType() == RJSON_OBJECT);

    int thread_num = 10;
    //int thread_num = static_cast<int>(value->getValueFromObject("thread_num")->getNumber());
    //int port = static_cast<int>(value->getValueFromObject("port")->getNumber());
    int port = 8888;

    ServiceManager manager(thread_num);

    HTTPService http_service(&manager, port);

    manager.run();
}
