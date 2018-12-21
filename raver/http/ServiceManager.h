#ifndef SERVICE_MANAGER_H
#define SERVICE_MANAGER_H

#include <functional>
#include <memory>
#include <vector>
#include "../base/noncopyable.h"

namespace raver {

class IOManager;
class Acceptor;

class ServiceManager : noncopyable {
 public:
  using AcceptorCallback = std::function<void(int)>;

  explicit ServiceManager(int thread_num);

  ~ServiceManager();

  void run();

  void registerAcceptor(int port, const AcceptorCallback& acceptor_cb);

  IOManager* ioManager() const { return iomanager_.get(); }

 private:
  std::unique_ptr<IOManager> iomanager_;

  // using Acceptors = std::vector<Acceptor*>;
  // Acceptors acceptors_;
  std::unique_ptr<Acceptor> acceptor_;
};

}  // namespace raver

#endif
