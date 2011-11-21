#include <net/distribute/DataReceiver.h>

#include <iostream>

using namespace net::distribute;

int main(int argc, char** argv)
{

    unsigned int port = 18121;

    DataReceiver recv(port);

    recv.start();

    return 0;
}

