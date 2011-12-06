#include <net/distribute/Msg.h>

namespace net{
namespace distribute{

const char* MsgHead::MSG_KEY_FILETYPE = "filetype";
const char* MsgHead::MSG_KEY_FILENAME = "filename";
const char* MsgHead::MSG_KEY_DATALENGTH = "datalen";
const char* MsgHead::MSG_KEY_COLLECTION = "collection";

const char* MsgHead::MSG_KEY_STATUS = "status";
const char* MsgHead::MSG_KEY_ERRORINFO = "errorinfo";
const char* MsgHead::MSG_KEY_RECV_DATALENGTH = "recvdatalen";

const char* MsgHead::MSG_ERROR_OK = "ok";
const char* MsgHead::MSG_ERROR_FAILEDTOCREATE = "failedtocreate";

std::string MsgHead::size2String(uint64_t size)
{
    uint64_t GBase = 1024*1024*1024;
    uint64_t MBase = 1024*1024;
    uint64_t KBase = 1024;

    std::ostringstream oss;
    if (size >= GBase)
    {
        oss<<(size>>30)<<"."<<(((size*10)>>30)%10)<<"G";
    }
    else if (size >= MBase)
    {
        oss<<(size>>20)<<"."<<(((size*10)>>20)%10)<<"M";
    }
    else if (size >= KBase)
    {
        oss<<(size>>10)<<"."<<(((size*10)>>10)%10)<<"K";
    }
    else
    {
        oss<<size<<"B";
    }

    return oss.str();
}

}
}
