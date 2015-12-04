#ifndef SERVER_H
#define SERVER_H

#include "clientregistry.h"

#include <thread>
#include <list>
#include <map>
#include <string>

class Server
{
public:
    Server();
    void listen(const std::string &ctrlFifoName);
private:
    ClientRegistry _clientRegistry;
};

#endif // SERVER_H
