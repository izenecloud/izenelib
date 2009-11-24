/**
* @file        UDTFSClient.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief It is used when indexmanager works as a client:
* dispatch inverted index data locally to remote index server
* the transmission utility is based on UDT library
*/

#ifndef UDTFSCLIENT_H
#define UDTFSCLIENT_H

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#else
#include <windows.h>
#endif

#include <fstream>
#include <iostream>
#include <string>

#ifndef UDT3
#include <3rdparty/udt/udt.h>
#else
#include <3rdparty/udt3/udt.h>
#endif

#include "UDTFSConstant.h"

using namespace std;

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class UDTFSClient
{
public:
    UDTFSClient();

    virtual ~UDTFSClient();
public:

    static int init(const string& server, const int port);

    static int try_connect();

    static int destroy();
protected:
    static string serverIP_;

    static int serverPort_;

    static int reusePort_;

    static UDTFSError errorInfo_;
};

class UDTFile:public UDTFSClient
{
public:
    UDTFile();

    virtual ~UDTFile();
public:

    int open(const char* filename, const string& mode);

    int64_t write(const char* buf, const int64_t& size);

    int upload();

    int seek(int64_t pos);

    int close();

private:
    int send(const char* buf, int size);

    int recv(char* buf, int size);

private:
    UDTSOCKET socket_;

    int64_t curWritePos_;

    int64_t size_;

    string filename_;

    string filepath_;
};

}

NS_IZENELIB_IR_END

#endif

