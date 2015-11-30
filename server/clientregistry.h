#ifndef CLIENTREGISTRY_H
#define CLIENTREGISTRY_H

#include <string>
#include <map>
#include <thread>
#include <mutex>

class ClientRegistry
{
public:
    ClientRegistry();
    void connectClient(const std::string& clientInFifo, const std::string& clientOutFifo);
private:
    static void connect(const std::string& clientInFifo, const std::string &clientOutFifo,
                        ClientRegistry* registry);
    std::mutex _clientThreadsMutex;
    std::map<std::thread::id, std::thread> _clientThreads;
};

#endif // CLIENTREGISTRY_H
