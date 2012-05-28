#ifndef IZENE_NET_DISTRIBUTE_MSG_H__
#define IZENE_NET_DISTRIBUTE_MSG_H__

#include <util/kv2string.h>
#include <string.h>
#include <sstream>

using namespace izenelib::util;

namespace net{
namespace distribute{

/**
 * Message struct for transferring data
 * | header |  data  |
 *
 */
struct MsgHead
{
    static const char* MSG_KEY_FILETYPE;
    static const char* MSG_KEY_FILENAME;
    static const char* MSG_KEY_DATALENGTH;
    static const char* MSG_KEY_COLLECTION;
    static const char* MSG_KEY_DESTINATION;

    static const char* MSG_KEY_STATUS;
    static const char* MSG_KEY_ERRORINFO;
    static const char* MSG_KEY_RECV_DATALENGTH;

    static const char* MSG_ERROR_OK;
    static const char* MSG_ERROR_FAILEDTOCREATE;

    static std::string size2String(uint64_t size);

    /**
     * msgRef().setValue(k, v)
     * @return ref to msg
     */
    kv2string& msgRef()
    {
        return msgpack_;
    }

    void clear()
    {
        msgpack_.clear();
    }

    std::string toString()
    {
        return msgpack_.serialize();
    }

    std::string toStringWithEnd()
    {
        char* eoh = NULL;
        size_t len;
        getEndOfHeader(eoh, len);

        return msgpack_.serialize()+eoh;
    }

    void loadMsg(std::string msg)
    {
        msgpack_.loadKvString(msg);
    }

    static void getEndOfHeader(char*& eoh, size_t& len)
    {
        static char s_eoh[] = "\r\n\r\n";
        eoh = s_eoh;
        len = strlen(s_eoh);
    }

    void setErrorInfo(const std::string& err)
    {
        msgpack_.setValue(MSG_KEY_ERRORINFO, err);
    }

    std::string getErrorInfo()
    {
        return msgpack_.getStrValue(MSG_KEY_ERRORINFO);
    }

protected:
    kv2string msgpack_;
};

struct SendFileReqMsg : public MsgHead
{
    enum eFileType
    {
        FTYPE_SCD,
        FTYPE_BIN
    };

    void setFileType(eFileType etype)
    {
        std::string stype;
        switch (etype)
        {
            case FTYPE_SCD:
                stype = "scd";
                break;
            case FTYPE_BIN:
                stype = "bin";
                break;
            default:
                stype = "undefined";
                break;
        }

        msgpack_.setValue(MSG_KEY_FILETYPE, stype);
    }

    bool isSCD()
    {
        if ( msgpack_.hasKey(MSG_KEY_FILETYPE)
             && msgpack_.getStrValue(MSG_KEY_FILETYPE) == "scd")
        {
            return true;
        }
        return false;
    }

    void setCollection(const std::string& collection)
    {
        msgpack_.setValue(MSG_KEY_COLLECTION, collection);
    }

    std::string getCollection()
    {
        return msgpack_.getStrValue(MSG_KEY_COLLECTION);
    }

    void setFileName(const std::string& fileName)
    {
        msgpack_.setValue(MSG_KEY_FILENAME, fileName);
    }

    std::string getFileName()
    {
        return msgpack_.getStrValue(MSG_KEY_FILENAME);
    }

    /**
     * The maximum size can be represented by int32_t is 4G,
     * a file can be much bigger.
     */
    void setFileSize(uint64_t fileSize)
    {
        msgpack_.setValue(MSG_KEY_DATALENGTH, fileSize);
    }

    uint64_t getFileSize()
    {
        return msgpack_.getUInt64Value(MSG_KEY_DATALENGTH);
    }
    
    // temporary
    void setDestination(const std::string& d) {
        msgpack_.setValue(MSG_KEY_DESTINATION, d);
    }
    
    std::string getDestination() {
        return msgpack_.getStrValue(MSG_KEY_DESTINATION);
    }
};



struct ResponseMsg : public MsgHead
{
    void setStatus(const std::string& stat)
    {
        msgpack_.setValue(MSG_KEY_STATUS, stat);
    }

    std::string getStatus()
    {
        return msgpack_.getStrValue(MSG_KEY_STATUS);
    }

    void setReceivedSize(uint64_t fileSize)
    {
        msgpack_.setValue(MSG_KEY_RECV_DATALENGTH, fileSize);
    }

    uint64_t getReceivedSize()
    {
        return msgpack_.getUInt64Value(MSG_KEY_RECV_DATALENGTH);
    }
};

}
}

#endif /* IZENE_NET_DISTRIBUTE_MSG_H__ */
