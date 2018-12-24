#include <raver/base/Logger.h>
#include <raver/http/ServiceManager.h>
#include <raver/http/HTTPService.h>

int main()
{
    using namespace raver;

    Logger::setLevel(Logger::LogLevel::Trace);

    ServiceManager manager;

    HTTPService http_service{&manager};
    // FTPService ftp_service{&manager};
    manager.Run();
}
