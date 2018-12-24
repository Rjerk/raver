#ifndef SERVICE_MANAGER_H
#define SERVICE_MANAGER_H

#include <functional>
#include <memory>
#include <vector>
#include <raver/base/noncopyable.h>
#include <raver/http/IOManager.h>
#include <raver/http/Acceptor.h>

namespace raver {

class IOManager;
class Acceptor;

class ServiceManager : noncopyable {
 public:
  using AcceptorCallback = std::function<void(int)>;

  explicit ServiceManager();

  void Init();

  void Run();

  void RegisterAcceptor(int port, const AcceptorCallback& acceptor_cb);

  IOManager* IoManager() const { return iomanager_.get(); }

  auto GetHttpPort() const { return http_port_; }

 private:
  std::unique_ptr<Acceptor> acceptor_;
  std::unique_ptr<IOManager> iomanager_;

  // config
  uint16_t http_port_;
  uint16_t thread_num_;
  
  // using Acceptors = std::vector<Acceptor*>;
  // Acceptors acceptors_;
};

}  // namespace raver

#endif
