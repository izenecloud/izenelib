#include <net/distribute/DataTransfer.h>

#include <iostream>
#include <string.h>

using namespace net::distribute;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout<<"Usage: "<<argv[0]<<" <filename> [-d]"<<std::endl;
        return 0;
    }

    std::string filename = argv[1];

    bool isDir = false;
    if (argc >= 3)
        if (strcasecmp(argv[2], "-d") == 0)
            isDir = true;

    DataTransfer tfer;

    tfer.connectToServer("localhost", 18121);
    tfer.syncSendFile(filename);

    return 0;
}
