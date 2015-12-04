#include "clientregistry.h"
#include "device.h"
#include "fifoiostream.h"

#include <thread>
#include <iostream>
#include <sstream>
#include <vector>
#include <future>

#include <unistd.h>

using namespace std;

ClientRegistry::ClientRegistry()
{

}

void ClientRegistry::connectClient(const string &clientInFifo, const string &clientOutFifo)
{
    std::clog << "connecting to [in] " << clientInFifo  << " [out] " << clientOutFifo << std::endl;
    int pipefd[2];
    //anonymous pipe for signaling to dispatcher thread
    if (pipe(pipefd)) {
        perror("pipe");
        return;
    }
    _clients.emplace_back(pipefd[0], pipefd[1], async(std::launch::async, dispatchClient, clientInFifo, clientOutFifo, pipefd[1]));
    //cleanup
    auto it = _clients.begin();
    while (it != _clients.end()) {
        if(it->isReady()) {
            it = _clients.erase(it);
        } else {
            ++it;
        }
    }
}

ClientRegistry::Client::~Client()
{
    write(_pipeIn, "Abort", 6);
    close(_pipeIn);
    _future.wait();
    close(_pipeOut);
}

bool ClientRegistry::Client::isReady() const
{
    auto status = _future.wait_for(chrono::milliseconds(0));
    return status == std::future_status::ready;
}

void dispatchClient(const std::string& clientInFifo, const std::string& clientOutFifo, int ctlFD)
{
    try {
        FifoIStreamBuf ib(clientOutFifo, 60, ctlFD);
        FifoOStreamBuf ob(clientInFifo, 5, ctlFD);
        istream is(&ib);
        ostream os(&ob);
        for (string line; getline(is, line);) {
            clog << "got line:" << line << endl;
            stringstream sstream(line);
            string cmd;
            vector<string> args;
            sstream >> cmd;
            for(string w; sstream >> w;) {
                args.push_back(move(w));
            }
            auto it = commands.find(cmd);
            if (it != commands.end()) {
                os << it->second(args) << endl;
            } else {
                cerr << "Invalid command" << endl;
                os << "FAILED" << endl;
            }
        }
    } catch (const runtime_error &e) {
        clog << e.what();
    }
    clog << "disconnecting from "  << clientInFifo  << " , " << clientOutFifo << std::endl;
}
