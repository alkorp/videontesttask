#include "server.h"

int main(int argc, char *argv[])
{
    Server srv;
    srv.listen((argc == 2)? argv[1]: "/var/tmp/srv_fifo");
    return 0;
}
