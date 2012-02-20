/* 
 * File:   ZooKeeperRouter.cpp
 * Author: Paolo D'Apice
 * 
 * Created on February 17, 2012, 2:32 PM
 */

#include "net/sf1r/ZooKeeperRouter.hpp"
#include "util/kv2string.h"
#include <glog/logging.h>

NS_IZENELIB_SF1R_BEGIN

using iz::ZooKeeper;
using izenelib::util::kv2string;
using std::string;
using std::vector;

const bool AUTO_RECONNECT = true;

ZooKeeperRouter::ZooKeeperRouter(const string& hosts, int recvTimeout) 
        : client(hosts, recvTimeout, AUTO_RECONNECT) {
    // TODO
    
    // 1. obtain the list of actually running SF1 servers
    setSf1List();
    
    // 2. connect to each of them using the Sf1Driver class
    // 3. register a watcher that will control the Sf1Driver instances
}


ZooKeeperRouter::~ZooKeeperRouter() {
    // free resources
}


namespace {
typedef vector<string> strvector;
typedef strvector::iterator iterator;

const string ROOT_NODE = "/";
const string SEARCH_TOPOLOGY = "/SearchTopology";
}

/*
 * /                                # ZooKeeper Root
 * |-- SF1R-[CLUSTERID]             # Root of distributed SF1 namespace, [CLUSTERID] is specified by user configuration.
 *     |--- SearchTopology          # Topology of distributed search cluster
 *          |--- ReplicaN           # A replica of search cluster
 *               |--- NodeN         # A SF1 node in the replica of search cluster, it can be a Master or Worker or both.
 */

void
ZooKeeperRouter::setSf1List() {
    strvector clusters;
    client.getZNodeChildren(ROOT_NODE, clusters);
    
    for (iterator it = clusters.begin(); it != clusters.end(); ++it) {
        string cluster = *it + SEARCH_TOPOLOGY;
        if (client.isZNodeExists(cluster)) {
            DLOG(INFO) << "got: " << cluster;
            strvector replicas;
            client.getZNodeChildren(cluster, replicas);
            
            for (iterator it = replicas.begin(); it != replicas.end(); ++it) {
                DLOG(INFO) << "replica: " << *it;
                strvector nodes;
                client.getZNodeChildren(*it, nodes);
                
                for (iterator it = nodes.begin(); it != nodes.end(); ++it) {
                    string data;
                    client.getZNodeData(*it, data);
                    
                    Sf1Node node(*it, data);
                    DLOG(INFO) << "node: " << node;
                    
                    sf1List.push_back(node);
                }
            }
        }
    }
}

NS_IZENELIB_SF1R_END
