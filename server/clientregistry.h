#ifndef CLIENTREGISTRY_H
#define CLIENTREGISTRY_H

#include <string>
#include <list>
#include <future>

class ClientRegistry
{
public:
    ClientRegistry();
    void connectClient(const std::string& clientInFifo, const std::string& clientOutFifo);
private:
    class Client
    {
    public:
        Client(int pipeIn, int pipeOut, std::future<void>&& future):
            _pipeIn(pipeIn), _pipeOut(pipeOut), _future(std::move(future)) {}
        ~Client();
        bool isReady() const;
    private:
        int _pipeIn;
        int _pipeOut;
        std::future<void> _future;
    };
    std::list<Client> _clients;
};

void dispatchClient(const std::string& clientInFifo, const std::string& clientOutFifo, int ctlFD);

#endif // CLIENTREGISTRY_H
