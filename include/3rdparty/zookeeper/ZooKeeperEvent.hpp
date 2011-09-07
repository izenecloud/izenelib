/**
 * @file ZooKeeperEevent.hpp
 * @author Zhongxia Li
 * @date Sep 7, 2011
 * @brief 
 */
#ifndef ZOO_KEEPER_EVENT_HPP_
#define ZOO_KEEPER_EVENT_HPP_

namespace zookeeper{

class ZooKeeperEventHandler
{
public:
    ZooKeeperEventHandler() {}
    virtual ~ZooKeeperEventHandler() {}

public:
    virtual void onNodeCreated(const std::string& path) {}
    virtual void onNodeDeleted(const std::string& path) {}
    virtual void onDataChanged(const std::string& path) {}
    virtual void onChildrenChanged(const std::string& path) {}
};

}

#endif /* ZOO_KEEPER_EVENT_HPP_ */
