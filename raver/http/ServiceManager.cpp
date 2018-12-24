#include <raver/http/ServiceManager.h>
#include <raver/http/Acceptor.h>
#include <raver/http/IOManager.h>

#include <raver/base/Logger.h>
#include <raver/base/Utils.h>
#include <raver/base/RJson.h>

#include <csignal>
#include <memory>

namespace raver {

namespace {

class IgnoreSigPipe {
 public:
  IgnoreSigPipe() { ::signal(SIGPIPE, SIG_IGN); }
};

IgnoreSigPipe init_obj;

}  // namespace

ServiceManager::ServiceManager():
 acceptor_{nullptr} 
{
  LOG_TRACE << "ServiceManager ctor";
  Init();
}

void ServiceManager::Init() {
  using namespace rjson;
  RJSON parser{utils::ReadFile("../conf/raver-config.json")};

  if (auto ret = parser.parseJson(); ret != PARSE_OK) {
    LOG_FATAL << "raver-config.json parse error";
  }
  
  auto value = parser.getValue(); 
  if (value->getType() == RJSON_OBJECT) {
    LOG_FATAL << "config is invalid json object"; 
  }

  thread_num_ = static_cast<uint16_t>(value->getValueFromObject("thread_num")->getNumber());
  http_port_ = static_cast<uint16_t>(value->getValueFromObject("http_port")->getNumber());

  iomanager_ = std::make_unique<IOManager>(thread_num_, this);
}

void ServiceManager::Run() 
{
  iomanager_->run();
}

void ServiceManager::RegisterAcceptor(int port,
                                      const AcceptorCallback& acceptor_cb) {
  acceptor_ = std::make_unique<Acceptor>(iomanager_.get(), http_port_, acceptor_cb);
}

}  // namespace raver
