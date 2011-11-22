#include <net/distribute/DataTransfer.h>

#include <iostream>
#include <string.h>

using namespace net::distribute;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout<<"Usage: "<<argv[0]<<" <filename> <dir> [-r] "<<std::endl;
        return 0;
    }

    bool isRecur = false;
    std::string filename = argv[1];
    std::string dir = "";

    for (int i = 1; i < argc; i++)
    {

        if (strcasecmp(argv[i], "--build_info") == 0)
        {
            // disable auto test
            return 0;
        }
        else if (strcasecmp(argv[i], "-r") == 0)
        {
            isRecur = true;

            if (i >= 3)
                dir = argv[2];
        }
    }


    DataTransfer tfer("localhost", 18121);

    tfer.syncSend(filename, dir, isRecur);

    return 0;
}
