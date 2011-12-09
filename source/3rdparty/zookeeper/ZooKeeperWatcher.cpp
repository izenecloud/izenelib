#include <ZooKeeperWatcher.hpp>
#include <ZooKeeperEvent.hpp>

using namespace std;
using namespace izenelib::zookeeper;

ZooKeeperWatcher::ZooKeeperWatcher()
{

}

ZooKeeperWatcher::~ZooKeeperWatcher()
{

}

void ZooKeeperWatcher::handleEvent(int eventType, int connState, const char *znodePath)
{
    //cout <<"ZooKeeperWatcher::handleEvent ( state="<< ZooKeeperEvent::state2String(connState)
    //        << " event="<<ZooKeeperEvent::watcherEvent2String(eventType)<<" path="<<znodePath<<" )"<<endl;

    ZooKeeperEvent evt(eventType, connState, znodePath);
    for (unsigned int i = 0; i < eventHandlerList.size(); i++)
        eventHandlerList[i]->process(evt);

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
        if (connState == 0)
        {
            for (unsigned int i = 0; i < eventHandlerList.size(); i++)
                eventHandlerList[i]->onSessionClosed();
        }
        else if (connState == ZOO_CONNECTED_STATE)
        {
            for (unsigned int i = 0; i < eventHandlerList.size(); i++)
                eventHandlerList[i]->onSessionConnected();
        }
        else if (connState == ZOO_EXPIRED_SESSION_STATE)
        {
            for (unsigned int i = 0; i < eventHandlerList.size(); i++)
                eventHandlerList[i]->onSessionExpired();
        }
        else if (connState == ZOO_AUTH_FAILED_STATE)
        {
            for (unsigned int i = 0; i < eventHandlerList.size(); i++)
                eventHandlerList[i]->onAuthFailed();
        }

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


UniqueZooKeeperWatcher UniqueZooKeeperWatcher::instance_;



/* EOF */

