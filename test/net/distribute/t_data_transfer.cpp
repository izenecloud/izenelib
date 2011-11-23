#include <net/distribute/DataTransfer.h>

#include <iostream>
#include <string.h>
#include <stdlib.h>

using namespace std;
using namespace net::distribute;

int main(int argc, char** argv)
{
    std::string host = "localhost";
    unsigned int port = 18121;
    std::string filename;
    std::string dir;
    bool isRecur = false;

    char optchar;
    bool help = false;
    while ((optchar = getopt(argc, argv, "h:p:f:d:r")) != -1)
    {
        switch (optchar) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'f':
                filename = optarg;
                break;
            case 'd':
                dir = optarg;
                break;
            case 'r':
                isRecur = true;
                break;
            default:
                cout << "Unrecognized flag " << optchar << endl;
                help = true;
                break;
        }

        if (help)
            break;
    }

    if (filename.empty() || help)
    {
        std::cout<<"Usage: "<<argv[0]<<" [-h <host> -p <port>] -f <filename> [-d <dirname> -r] "<<std::endl;
        return 0;
    }

    DataTransfer tfer(host, port);

    tfer.syncSend(filename, dir, isRecur);

    return 0;
}
