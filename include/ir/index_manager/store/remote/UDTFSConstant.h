/**
* @file        UDTFSConstant.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief 
*/
#ifndef UDTFS_H
#define UDTFS_H

#include <types.h>

#include <string>
#include <map>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

#define FILEOPEN_MODE_TRUNK    1
#define FILEOPEN_MODE_APPEND   2
#define FILEOPEN_MODE_MODIFY   3

enum UDTFSRequest
{
    UDTFS_OPEN = 100,
    UDTFS_WRITE = 200,
    UDTFS_UPLOAD = 400,
    UDTFS_CLOSE = 800
};

enum UDTFSResponse
{
    UDTFS_OPEN_ACK = 101,
    UDTFS_WRITE_ACK = 201,
    UDTFS_UPLOAD_ACK = 401,
    UDTFS_CLOSE_ACK = 801
};

class UDTFSMessage
{
public:
    UDTFSMessage();

    ~UDTFSMessage();

public:
    int resize(const int& len);

    int32_t getType() const;

    void setType(const UDTFSRequest& type);

    char* getData() const;

    void setData(const int& offset, const char* data, const int& len);

    int32_t getDataLength() const;
public:
    static const int hdrSize_ = 8;  //  4byte cmd + 4byte length

    char* pcBuffer_;

    int dataLength_;

    int bufLength_;
};


class UDTFSError
{
public:
    static const int OK = 0;
    static const int E_UNKNOWN = -1; 	// unknown error
    static const int E_SOCKET = -1000;
    static const int E_BIND = -1001;
    static const int E_CONNECTION = - 1002;	// cannot connect to master
    static const int E_SEND = -1003;
    static const int E_RECEIVE = -1004;
    static const int E_TIMEDOUT = -5000; 	// timeout
    static const int E_INVALID = -6000;		// invalid parameter

public:
    static int init();
    static std::string getErrorMsg(int ecode);

private:
    static std::map<int, std::string> errorMsg_;
};

}

NS_IZENELIB_IR_END

#endif
