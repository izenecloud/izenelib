#include <ir/index_manager/store/UDTFSConstant.h>
#include <string.h>

using namespace std;
using namespace izenelib::ir::indexmanager;

UDTFSMessage::UDTFSMessage()
        :dataLength_(hdrSize_),
        bufLength_(256)
{
    pcBuffer_ = new char[bufLength_];
    memset(pcBuffer_,0,bufLength_);
}

UDTFSMessage::~UDTFSMessage()
{
    delete [] pcBuffer_;
}

int UDTFSMessage::resize(const int& len)
{
    bufLength_ = len;

    if (bufLength_ < dataLength_)
        bufLength_ = dataLength_;

    char* temp = new char[bufLength_];
    memcpy(temp, pcBuffer_, dataLength_);
    delete [] pcBuffer_;
    pcBuffer_ = temp;

    return bufLength_;
}


int32_t UDTFSMessage::getType() const
{
    return *(int32_t*)pcBuffer_;
}

void UDTFSMessage::setType(const UDTFSRequest& type)
{
    *(int32_t*)pcBuffer_ = (int32_t)type;
    *(int32_t*)(pcBuffer_ + 4) = 0;
}

char* UDTFSMessage::getData() const
{
    return pcBuffer_ + hdrSize_;
}

int32_t UDTFSMessage::getDataLength() const
{
    return *(int32_t*)(pcBuffer_ + 4);
}

void UDTFSMessage::setData(const int& offset, const char* data, const int& len)
{
    while (hdrSize_ + offset + len > bufLength_)
        resize(bufLength_ << 1);

    memcpy(pcBuffer_ + hdrSize_ + offset, data, len);

    if (dataLength_ < hdrSize_ + offset + len)
        dataLength_ = hdrSize_ + offset + len;

    *(int32_t*)(pcBuffer_ + 4) = dataLength_ - hdrSize_;
}



map<int, string> UDTFSError::errorMsg_;

int UDTFSError::init()
{
    errorMsg_.clear();
    errorMsg_[-1] = "unknown error.";
    errorMsg_[-1000] = "socket error.";
    errorMsg_[-1001] = "bind error.";
    errorMsg_[-1002] = "connect error.";
    errorMsg_[-1003] = "send error.";
    errorMsg_[-1004] = "receive error.";
    errorMsg_[-5000] = "a timeout event happened.";
    errorMsg_[-6000] = "at least one parameter is invalid.";

    return errorMsg_.size();
}

string UDTFSError::getErrorMsg(int ecode)
{
    map<int, string>::const_iterator i = errorMsg_.find(ecode);
    if (i == errorMsg_.end())
        return "unknown error.";

    return i->second;
}

