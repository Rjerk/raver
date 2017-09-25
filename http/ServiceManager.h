#ifndef SERVICE_MANAGER_H
#define SERVICE_MANAGER_H

#include <vector>
#include <mutex>
#include <condition_variable>

#include "Acceptor.h"

namespace raver {

class IOManager;

class ServiceManager : noncopyable {
public:
    using AcceptorCallback = std::function<void (int)>;

    explicit ServiceManager(int num_thread = 1);

    ~ServiceManager();

    void run();

    void stop();

    void registerAcceptor(int port, AcceptorCallback cb);

    bool isStopped() const { return stopped_; }
    IOManager* ioManager() const { return io_.get(); }
private:
    using Acceptors = std::vector<Acceptor*>;
    const int num_thread_;
    bool stop_requested_;
    bool stopped_;
    Acceptors acceptors_;
    std::unique_ptr<IOManager> io_;
    std::mutex mtx_;
    std::condition_variable cv_;
};

}

#endif
