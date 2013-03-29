/* 
 * File:   ZooKeeperNameSpace.hpp
 * Author: Paolo D'Apice
 *
 * Created on February 21, 2012, 2:14 PM
 */

#ifndef ZOOKEEPERNAMESPACE_HPP
#define	ZOOKEEPERNAMESPACE_HPP

#include "net/sf1r/config.h"
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

NS_IZENELIB_SF1R_BEGIN

/** The zookeeper hierarchy root. */
const std::string ROOT_NODE = "/";

/** Topology namespace within a node. */
//const std::string  TOPOLOGY = "/Topology";
const std::string  TOPOLOGY = "/Servers";

/**
 * Regular expression matching a topology namespace. 
 * Example: /SF1R-host23
 */
const boost::regex SF1R_ROOT_REGEX("\\/SF1R-[^\\/]+");

/**
 * Regular expression matching a search topologu namespace. 
 * Example: /SF1R-host23/SearchTopology
 */
const boost::regex TOPOLOGY_REGEX("\\/SF1R-[^\\/]+\\/Servers");

/**
 * Regular expression matching a SF1 node on search topology.
 * Example: /SF1R-host23/SearchTopology/Replica1/Node1
 */
const boost::regex SF1R_NODE_REGEX("\\/SF1R-[^\\/]+\\/Servers\\/Server\\d+");

const std::string SearchService = "search";
const std::string RecommendService = "recommend";
/**
 * Noda data key for the host.
 */
const std::string       HOST_KEY = "host";

/**
 * Noda data key for the SF1 baport.
 */
const std::string     BAPORT_KEY = "baport";
const std::string     REPLICAID_KEY = "replicaid";

/**
 * Noda data key for the SF1 masterport.
 */
const std::string MASTERPORT_KEY = "masterport";

const std::string MASTER_NAME_KEY = "mastername";
/**
 * Noda data key for the collection.
 */
const std::string COLLECTION_KEY = "collection";
const std::string SERVICE_STATE_KEY = "service_state";
const std::string SERVICE_NAMES_KEY = "service_names";

/**
 * Noda data delimiter character.
 */
const char   DELIMITER_CHAR = ',';

NS_IZENELIB_SF1R_END

#endif	/* ZOOKEEPERNAMESPACE_HPP */
