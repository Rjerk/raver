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

    auto value = parser.getValue();
    assert(value->JsonValue() == RJON_OBJECT);

    auto thread_num = value->getValueFromObject("thread_num")->getNumber();
    auto port = value->getValueFromObject("port")->getNumber();

    ServiceManager manager(thread_num);

    HTTPService http_service(&manager, port);

    manager.run();
}
