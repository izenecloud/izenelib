/* 
 * File:   ZooKeeperNameSpace.hpp
 * Author: Paolo D'Apice
 *
 * Created on February 21, 2012, 2:14 PM
 */

#ifndef SF1TOPOLOGY_HPP
#define	SF1TOPOLOGY_HPP

#include <boost/regex.hpp>
#include <string>

/**
 * Here is the namspace used by SF1 and ZooKeeper:
 * 
 * /                                # ZooKeeper Root
 * |-- SF1R-[CLUSTERID]             # Root of distributed SF1 namespace, [CLUSTERID] is specified by user configuration.
 *     |--- SearchTopology          # Topology of distributed search cluster
 *          |--- ReplicaN           # A replica of search cluster
 *               |--- NodeN         # A SF1 node in the replica of search cluster, it can be a Master or Worker or both.
 * 
 * For further information please refer to the sf1-engine project:
 *  sf1r-engine/source/core/node-manager/ZooKeeperNamespace.h
 */

namespace {
    const std::string       ROOT_NODE = "/";
    const std::string SEARCH_TOPOLOGY = "/SearchTopology";
    const std::string         REPLICA = "Replica";
    const std::string            NODE = "Node";
    
    const boost::regex CLUSTER_REGEX("\\/SF1R-\\w+\\d?\\/SearchTopology");
    const boost::regex    NODE_REGEX("\\/SF1R-\\w+\\d?\\/SearchTopology\\/Replica\\d+\\/Node\\d+");
}

#endif	/* SF1TOPOLOGY_HPP */

