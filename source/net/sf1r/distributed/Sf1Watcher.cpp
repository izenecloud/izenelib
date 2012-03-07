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

using iz::ZooKeeper;
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
    DLOG(INFO) << zkEvent.toString();
}


void 
Sf1Watcher::onNodeCreated(const string& path) {
    DLOG(INFO) << "created: " << path;
    if (boost::regex_match(path, CLUSTER_REGEX)) {
        LOG(INFO) << "adding " << path << " ...";
        router.addClusterNode(path);
    }
}


void 
Sf1Watcher::onNodeDeleted(const string& path) {
    DLOG(INFO) << "deleted: " << path;
    if (boost::regex_match(path, NODE_REGEX)) {
        LOG(INFO) << "removing " << path << " ...";
        router.removeClusterNode(path);
    }
}
    

void 
Sf1Watcher::onDataChanged(const string& path) {
    DLOG(INFO) << "changed: " << path;
    if (boost::regex_match(path, NODE_REGEX)) {
        LOG(INFO) << "reloading " << path << " ...";
        router.updateNodeData(path);
    }
}


void 
Sf1Watcher::onChildrenChanged(const string& path) {
    DLOG(INFO) << "children changed: " << path;
    if (not boost::regex_match(path, NODE_REGEX)) {
        router.watchChildren(path);
    }
}


NS_IZENELIB_SF1R_END
