#include <ZooKeeperWatcher.hpp>
#include <ZooKeeperEvent.hpp>

#include <zookeeper.h>
#include <zk_adaptor.h>

using namespace std;
using namespace zookeeper;

ZooKeeperWatcher::ZooKeeperWatcher()
{

}

ZooKeeperWatcher::~ZooKeeperWatcher()
{

}

void ZooKeeperWatcher::eventHandle(int eventType, int connState, const char *znodePath)
{
    cout <<"ZooKeeperWatcher::eventHandle ( state="<< state2String(connState)
            << " event="<<watcherEvent2String(eventType)<<" path="<<znodePath<<" )"<<endl;

    if (eventType == ZOO_CREATED_EVENT)
    {
        for (unsigned int i = 0; i < eventHandlerList.size(); i++)
            eventHandlerList[i]->onNodeCreated(znodePath);
    }
    else if (eventType == ZOO_DELETED_EVENT)
    {
        for (unsigned int i = 0; i < eventHandlerList.size(); i++)
            eventHandlerList[i]->onNodeDeleted(znodePath);
    }
    else if (eventType == ZOO_CHANGED_EVENT)
    {
        for (unsigned int i = 0; i < eventHandlerList.size(); i++)
            eventHandlerList[i]->onDataChanged(znodePath);
    }
    else if (eventType == ZOO_CHILD_EVENT)
    {
        for (unsigned int i = 0; i < eventHandlerList.size(); i++)
            eventHandlerList[i]->onChildrenChanged(znodePath);
    }
    else if (eventType == ZOO_SESSION_EVENT)
    {
        // xxx
    }
    else if (eventType == ZOO_NOTWATCHING_EVENT)
    {
        // xxx
    }
}

void ZooKeeperWatcher::registerEventHandler(ZooKeeperEventHandler* evtHandler)
{
    eventHandlerList.push_back(evtHandler);
}

std::string ZooKeeperWatcher::state2String(int state)
{
    switch(state){
    case 0:
        return "ZOO_CLOSED_STATE";
    case CONNECTING_STATE_DEF:
        return "ZOO_CONNECTING_STATE";
    case ASSOCIATING_STATE_DEF:
        return "ZOO_ASSOCIATING_STATE";
    case CONNECTED_STATE_DEF:
        return "ZOO_CONNECTED_STATE";
    case EXPIRED_SESSION_STATE_DEF:
        return "ZOO_EXPIRED_SESSION_STATE";
    case AUTH_FAILED_STATE_DEF:
        return "ZOO_AUTH_FAILED_STATE";
    }
    return "INVALID_STATE";
}

std::string ZooKeeperWatcher::watcherEvent2String(int ev)
{
    switch(ev){
    case 0:
        return "ZOO_ERROR_EVENT";
    case CREATED_EVENT_DEF:
        return "ZOO_CREATED_EVENT";
    case DELETED_EVENT_DEF:
        return "ZOO_DELETED_EVENT";
    case CHANGED_EVENT_DEF:
        return "ZOO_CHANGED_EVENT";
    case CHILD_EVENT_DEF:
        return "ZOO_CHILD_EVENT";
    case SESSION_EVENT_DEF:
        return "ZOO_SESSION_EVENT";
    case NOTWATCHING_EVENT_DEF:
        return "ZOO_NOTWATCHING_EVENT";
    }
    return "INVALID_EVENT";
}
