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
//#include <auto_ptr.h>

namespace izenelib{
namespace zookeeper{

class ZooKeeperEventHandler;

class ZooKeeperWatcher
{
public:
    ZooKeeperWatcher();

    ~ZooKeeperWatcher();

    void handleEvent(int eventType, int connState, const char *znodePath);

    void registerEventHandler(ZooKeeperEventHandler* evtHandler);

protected:
    std::vector<ZooKeeperEventHandler*> eventHandlerList;
};


class UniqueZooKeeperWatcher : public ZooKeeperWatcher
{
public:
    static UniqueZooKeeperWatcher* Instance()
    {
        return &instance_;
    }

private:
    UniqueZooKeeperWatcher() {}
    UniqueZooKeeperWatcher(const UniqueZooKeeperWatcher&);

    static UniqueZooKeeperWatcher instance_;
};


}} // namesapce

#endif /* ZOO_KEEPER_WATCHER_HPP_ */
