/**
 * @file ZooKeeperEevent.hpp
 * @author Zhongxia Li
 * @date Sep 7, 2011
 * @brief 
 */
#ifndef ZOO_KEEPER_EVENT_HPP_
#define ZOO_KEEPER_EVENT_HPP_

#include <3rdparty/zookeeper/zookeeper.h>
#include <sstream>

namespace izenelib{
namespace zookeeper{

class ZooKeeperEvent
{
public:
    int type_;
    int state_;
    std::string path_;

public:
    ZooKeeperEvent()
    : type_(0), state_(0), path_()
    {
    }

    ZooKeeperEvent(int type, int state, const std::string path)
    : type_(type), state_(state), path_(path)
    {
    }

    std::string toString()
    {
        std::stringstream ss;
        ss<<watcherEvent2String(type_)<<" - "<<state2String(state_)<<" - "<<path_<<std::endl;
        return ss.str();
    }

    static std::string state2String(int state)
    {
        if (state == 0) {
            return "ZOO_CLOSED_STATE";
        }
        else if (state == ZOO_CONNECTING_STATE) {
            return "ZOO_CONNECTING_STATE";
        }
        else if (state == ZOO_ASSOCIATING_STATE) {
            return "ZOO_ASSOCIATING_STATE";
        }
        else if(state == ZOO_CONNECTED_STATE)
        {
            return "ZOO_CONNECTED_STATE";
        }
        else if (state == ZOO_EXPIRED_SESSION_STATE) {
            return "ZOO_EXPIRED_SESSION_STATE";
        }
        else if (state == ZOO_AUTH_FAILED_STATE) {
            return "ZOO_AUTH_FAILED_STATE";
        }
        return "INVALID_STATE";
    }

    static std::string watcherEvent2String(int ev)
    {
        if (ev == 0) {
            return "ZOO_ERROR_EVENT";
        }
        else if (ev == ZOO_CREATED_EVENT) {
            return "ZOO_CREATED_EVENT";
        }
        else if (ev == ZOO_DELETED_EVENT) {
            return "ZOO_DELETED_EVENT";
        }
        else if (ev == ZOO_CHANGED_EVENT) {
            return "ZOO_CHANGED_EVENT";
        }
        else if (ev == ZOO_CHILD_EVENT) {
            return "ZOO_CHILD_EVENT";
        }
        else if (ev == ZOO_SESSION_EVENT) {
            return "ZOO_SESSION_EVENT";
        }
        else if (ev == ZOO_NOTWATCHING_EVENT) {
            return "ZOO_NOTWATCHING_EVENT";
        }
        return "INVALID_EVENT";
    }
};

class ZooKeeperEventHandler
{
public:
    ZooKeeperEventHandler() {}
    virtual ~ZooKeeperEventHandler() {}

public:
    /// will be called on any event.
    virtual void process(ZooKeeperEvent& zkEvent) {}

    virtual void onNodeCreated(const std::string& path) {}
    virtual void onNodeDeleted(const std::string& path) {}
    virtual void onDataChanged(const std::string& path) {}
    virtual void onChildrenChanged(const std::string& path) {}

    virtual void onSessionClosed() {}
    virtual void onSessionConnected() {}
    virtual void onSessionExpired() {}
    virtual void onAuthFailed() {}
};

}} // namespace

#endif /* ZOO_KEEPER_EVENT_HPP_ */
