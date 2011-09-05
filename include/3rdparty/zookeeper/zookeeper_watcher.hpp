#ifndef _ZOOKEEPER_WATCHER_HPP_
#define _ZOOKEEPER_WATCHER_HPP_

#include <zookeeper.h>
#include <string>
#include <vector>
#include <iostream>

class ZooKeeperEvent;

class ZooKeeperWatcher
{
public:
    ZooKeeperWatcher();
    virtual ~ZooKeeperWatcher();

    int initialize(const std::string& zkServer, const std::string& zkPath);
    int registerEvent(ZooKeeperEvent* e);
    zhandle_t* getZooKeeper() const
    {
        return zk;
    }

public:
    // should be protected, but required for callback.
    int callback(int type, int state, const std::string& path);

protected:
    int ensureNodeExistence(const std::string& path);

protected:
    zhandle_t *zk;
    std::vector<ZooKeeperEvent*> events;
};

#endif /* Not def: _ZOOKEEPER_WATCHER_HPP_ */
