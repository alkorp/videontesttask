#include "clientregistry.h"
#include "device.h"

#include <fstream>
#include <thread>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

ClientRegistry::ClientRegistry()
{

}

void ClientRegistry::connectClient(const string &clientInFifo, const string &clientOutFifo)
{
    std::cout << "conn to in " << clientInFifo  << " out " << clientOutFifo << std::endl;
    thread t(ClientRegistry::connect, clientInFifo, clientOutFifo, this);
    std::lock_guard<std::mutex> lock(_clientThreadsMutex);
    thread::id tID = t.get_id();
    _clientThreads.emplace(tID, thread(move(t)));
}

void ClientRegistry::connect(const string &clientInFifo, const string &clientOutFifo,
                             ClientRegistry* registry)
{
    ofstream cliIn(clientInFifo);
    ifstream cliOut(clientOutFifo);

    for (string line; getline(cliOut, line);) {

        //cout << "got line:" << line << endl;
        stringstream sstream(line);
        string cmd;
        vector<string> args;
        sstream >> cmd;
        for(string w; sstream >> w;) {
            args.push_back(move(w));
        }
        auto it = commands.find(cmd);
        if (it != commands.end()) {
            cliIn << it->second(args) << endl;
        } else {
            cerr << "Invalid command" << endl;
            cliIn << "FAILED" << endl;
        }
    }
    std::lock_guard<std::mutex> lock(registry->_clientThreadsMutex);
    auto it = registry->_clientThreads.find(std::this_thread::get_id());
    it->second.detach();
    registry->_clientThreads.erase(it);
}
