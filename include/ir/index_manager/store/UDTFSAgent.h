/**
* @file        UDTFSAgent.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief It is used when indexmanager works as a server:
* receives inverted index data from remote client and flush them to local disk
* the transmission utility is based on UDT library
*/

#ifndef UDTFSAGENT_H
#define UDTFSAGENT_H

#ifndef WIN32
#include <cstdlib>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <fstream>
#include <iostream>
#include <string>

#ifndef UDT3
#include <3rdparty/udt/udt.h>
#else
#include <3rdparty/udt3/udt.h>
#endif

#include <ir/index_manager/index/BarrelInfo.h>
#include <ir/index_manager/index/IndexMergerAgent.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/UDTFSConstant.h>

using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class Indexer;
class UDTFSAgent
{
public:
    UDTFSAgent(string port, Indexer* pIndexer);

    ~UDTFSAgent();

public:
    void run();

private:
    UDTSOCKET accept(sockaddr* addr, int* addrlen);

    int recv(UDTSOCKET recver, char* buf, int size);

    int recvMsg(UDTSOCKET recver, UDTFSMessage* msg);

    void agentHandler(UDTSOCKET recver);
private:
    UDTSOCKET serv_;

    IndexMergerAgent* pIndexMergerAgent_;

    Indexer* pIndexer_;

    BarrelsInfo* pBarrelsInfo_;

    string home_;

    string currBarrelName_;
};

}

NS_IZENELIB_IR_END

#endif
