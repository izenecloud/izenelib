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
    while ((optchar = getopt(argc, argv, "h:p:s:d:r")) != -1)
    {
        switch (optchar) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 's':
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

    if (help || filename.empty())
    {
        std::cout<<"Usage: "<<argv[0]<<" -s <src:file-or-dir> [-h <host> -p <port> -d <dest:dir> -r] "<<std::endl;
        std::cout<<"    -h  destination host, default is locahost"<<std::endl;
        std::cout<<"    -p  destination port, default is 18121"<<std::endl;
        std::cout<<"    -s  source to be sent: file or directory path name"<<std::endl;
        std::cout<<"    -d  destination directory to store source, default is same as source dir"<<std::endl;
        std::cout<<"    -r  sent dir recursively if use this option"<<std::endl;
        return 0;
    }

    DataTransfer tfer(host, port);

    tfer.syncSend(filename, dir, isRecur);

    return 0;
}
