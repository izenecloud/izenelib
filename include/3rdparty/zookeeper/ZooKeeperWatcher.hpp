/**
 * @file ZooKeeperWatcher.hpp
 * @author Zhongxia Li
 * @date Sep 7, 2011
 * @brief 
 */
#ifndef ZOO_KEEPER_WATCHER_HPP_
#define ZOO_KEEPER_WATCHER_HPP_

#include <iostream>
#include <string>
#include <vector>

namespace zookeeper{

class ZooKeeperEventHandler;

class ZooKeeperWatcher
{
public:
    ZooKeeperWatcher();

    ~ZooKeeperWatcher();

    void handleEvent(int eventType, int connState, const char *znodePath);

    void registerEventHandler(ZooKeeperEventHandler* evtHandler);

private:
    std::string state2String(int state);

    std::string watcherEvent2String(int ev);

private:
    std::vector<ZooKeeperEventHandler*> eventHandlerList;
};

}

#endif /* ZOO_KEEPER_WATCHER_HPP_ */
