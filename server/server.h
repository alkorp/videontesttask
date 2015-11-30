#ifndef SERVER_H
#define SERVER_H

#include "clientregistry.h"

#include <thread>
#include <list>
#include <map>
#include <string>

namespace Constants {
const std::string CtrlFifoDefault = "/var/tmp/srv_fifo";
}

class Server
{
public:
    Server();
    void listen(const std::string &ctrlFifoName = Constants::CtrlFifoDefault);
private:
    ClientRegistry _clientRegistry;
};

#endif // SERVER_H
