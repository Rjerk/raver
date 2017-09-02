#include "ServiceManager.h"
#include "HTTPService.h"

#include <iostream>

int main(int argc, char** argv)
{
    if (argc != 3) {
        std::cerr << "Usage: ./raver <port> <num-thread>" << std::endl;
        exit(1);
    }

    int port = atoi(argv[1]);
    int num_thread = atoi(argv[2]);

    raver::ServiceManager manager(num_thread);
    raver::HTTPService http_service(port, &manager);

    manager.run();
}
