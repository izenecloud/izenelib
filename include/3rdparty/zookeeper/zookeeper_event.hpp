#ifndef _ZOOKEEPER_EVENT_HPP_
#define _ZOOKEEPER_EVENT_HPP_

#include <string>

class ZooKeeperWatcher;

class ZooKeeperEvent
{
public:
    ZooKeeperEvent(ZooKeeperWatcher* w) : w(w) {}
    virtual ~ZooKeeperEvent() {}

public:
    // callbacks
    virtual void onNodeCreated(const std::string& path) {}
    virtual void onNodeDeleted(const std::string& path) {}
    virtual void onDataChanged(const std::string& path) {}
    virtual void onChildrenChanged(const std::string& path) {}

protected:
    ZooKeeperWatcher* w;
};

#endif /* Not def: _ZOOKEEPER_EVENT_HPP_ */
