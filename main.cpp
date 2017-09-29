#include "./http/ServiceManager.h"
#include "./http/HTTPService.h"
#include "./base/RJson.h"
#include "./base/Logger.h"

int main(/*int argc, char** argv*/)
{
    using namespace raver;
    Logger::setLevel(Logger::LogLevel::Trace);

    rjson::RJSON parser(readFile("config.json"));
    auto ret = parser.parseJson(); (void)ret;
    assert(ret == rjson::PARSE_OK);
    auto value = parser.getValue();
    assert(value->getType() == rjson::RJSON_OBJECT);
    auto thread_num = value->getValueFromObject("thread-num")->getNumber();
    auto port = value->getValueFromObject("port")->getNumber();

    ServiceManager manager(thread_num);
    HTTPService http_service(port, &manager);

    manager.run();
}
