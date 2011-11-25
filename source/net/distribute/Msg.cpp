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

}
}
