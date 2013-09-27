/* 
 * File:   Sf1Watcher.hpp
 * Author: Paolo D'Apice
 *
 * Created on February 21, 2012, 9:42 AM
 */

#ifndef SF1WATCHER_HPP
#define	SF1WATCHER_HPP

#include "net/sf1r/config.h"
#include "3rdparty/zookeeper/ZooKeeperEvent.hpp"
#include <boost/noncopyable.hpp>

NS_IZENELIB_SF1R_BEGIN

namespace iz = izenelib::zookeeper;

class ZooKeeperRouter;


/**
 * ZooKeeper event handler for the SF1 distributed driver.
 */
class Sf1Watcher : public iz::ZooKeeperEventHandler, private boost::noncopyable {
public:
    
    Sf1Watcher(ZooKeeperRouter& router, bool rewatch = true);
    ~Sf1Watcher();

    void process(iz::ZooKeeperEvent& zkEvent);
    
    void onNodeCreated(const std::string& path);
    void onNodeDeleted(const std::string& path);
    void onDataChanged(const std::string& path);
    void onChildrenChanged(const std::string& path);

    void onSessionClosed() {};
    void onSessionConnected() {};
    void onSessionExpired() {};
    void onAuthFailed() {};

private:
    ZooKeeperRouter& router;
    const bool rewatch;
};

NS_IZENELIB_SF1R_END

#endif	/* SF1WATCHER_HPP */
