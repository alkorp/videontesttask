#include "server.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

Server::Server()
{
}

void Server::listen(const std::string &ctrlFifoName)
{
    struct stat st;
    if (!stat(ctrlFifoName.c_str(), &st)) {
        if (!S_ISFIFO(st.st_mode)) {
            cerr << ctrlFifoName << " is not a FIFO" << endl;
            return;
        } else {
            cerr << "Warning: " << ctrlFifoName << " already exists" << endl;
        }
    } else if (mkfifo(ctrlFifoName.c_str(), 0666)) {
        perror("fifo");
        return;
    }

    ifstream istr(ctrlFifoName);
    bool done = false;
    while (!done) {
        for (string line; getline(istr, line);) {
            stringstream sstream(line);
            vector<string> args;
            copy(istream_iterator<string>(sstream),
                 istream_iterator<string>(),
                 back_inserter(args));
            if (args.size() == 1 && args.front() == "exit") {
                done = true;
                break;
            } else if (args.size() == 2) {
                _clientRegistry.connectClient(args[0], args[1]);
            } else {
                cerr << "Invalid number of parameters" << endl;
            }
        }
        if (!istr.eof()) {
            cerr << "Error reading from " << ctrlFifoName << endl;
            done = true;
        } else {
            // wait for new connections
            istr.clear();
        }
    }
}
