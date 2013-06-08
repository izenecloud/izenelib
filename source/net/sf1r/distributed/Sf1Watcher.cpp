/* 
 * File:   Sf1Watcher.cpp
 * Author: Paolo D'Apice
 * 
 * Created on February 21, 2012, 9:42 AM
 */

#include "Sf1Watcher.hpp"
#include "ZooKeeperNamespace.hpp"
#include "net/sf1r/distributed/ZooKeeperRouter.hpp"
#include <boost/regex.hpp>
#include <glog/logging.h>

NS_IZENELIB_SF1R_BEGIN

using iz::ZooKeeperEvent;
using std::string;


Sf1Watcher::Sf1Watcher(ZooKeeperRouter& r, bool rw) 
        : router(r), rewatch(rw) {
    DLOG(INFO) << "watcher created";
}


Sf1Watcher::~Sf1Watcher() {
    DLOG(INFO) << "watcher destroyed";
}


void 
Sf1Watcher::process(ZooKeeperEvent& zkEvent) {
    LOG(INFO) << zkEvent.toString();
    if (zkEvent.type_ == ZOO_SESSION_EVENT && zkEvent.state_ == ZOO_EXPIRED_SESSION_STATE)
    {
        while(true)
        {
            try
            {
            LOG(INFO) << "session expired, reconnect.";
            router.reconnect();
            break;
            }
            catch(...)
            {
                sleep(10);
            }
        }
    }
    else if (zkEvent.type_ == ZOO_SESSION_EVENT && zkEvent.state_ == ZOO_CONNECTED_STATE)
    {
        LOG(INFO) << "auto connected";
        router.reloadTopology();
    }
}


void 
Sf1Watcher::onNodeCreated(const string& path) {
    LOG(INFO) << "created: " << path;
    if (boost::regex_match(path, TOPOLOGY_REGEX)) {
        LOG(INFO) << "adding " << path << " ...";
        router.addSearchTopology(path);
    }
}

void 
Sf1Watcher::onNodeDeleted(const string& path) {
    LOG(INFO) << "deleted: " << path;
    if (boost::regex_match(path, SF1R_NODE_REGEX)) {
        LOG(INFO) << "removing " << path << " ...";
        router.removeSf1Node(path);
    } else if (boost::regex_match(path, TOPOLOGY_REGEX)) {
        LOG(INFO) << "watching " << path << " ...";
        router.watchChildren(path);
    }
}
    

void 
Sf1Watcher::onDataChanged(const string& path) {
    DLOG(INFO) << "changed: " << path;
    if (boost::regex_match(path, SF1R_NODE_REGEX)) {
        LOG(INFO) << "reloading " << path << " ...";
        router.updateNodeData(path);
    }
}


void 
Sf1Watcher::onChildrenChanged(const string& path) {
    LOG(INFO) << "children changed: " << path;
    if (boost::regex_search(path, SF1R_ROOT_REGEX) || path == ROOT_NODE) {
        router.watchChildren(path);
    }
}


NS_IZENELIB_SF1R_END
