/* 
 * File:   RoundRobinPolicy.cpp
 * Author: Paolo D'Apice
 * 
 * Created on March 5, 2012, 2:59 PM
 */

#include "net/sf1r/distributed/Sf1Topology.hpp"
#include "RoundRobinPolicy.hpp"
#include "share_data_def.h"
#include <boost/signals2.hpp>
#include <glog/logging.h>

NS_IZENELIB_SF1R_BEGIN

using std::string;

RoundRobinPolicy::RoundRobinPolicy(Sf1Topology &topology, Sf1Topology& backup_nodes)
        : RoutingPolicy(topology), counter(0), backup_topology(backup_nodes) {
    updateCollections();
    topology.changed.connect(boost::bind(&RoundRobinPolicy::resetCounter, this));
    topology.changed.connect(boost::bind(&RoundRobinPolicy::updateCollections, this));
    backup_topology.changed.connect(boost::bind(&RoundRobinPolicy::resetCounter, this));
    backup_topology.changed.connect(boost::bind(&RoundRobinPolicy::updateCollections, this));
    
    DLOG(INFO) << "initialized";
}


RoundRobinPolicy::~RoundRobinPolicy() {
    DLOG(INFO) << "destroyed";
}


void
RoundRobinPolicy::resetCounter() {
    DLOG(INFO) << "resetting counter due to topology changes";
    counter = 0;
}

void RoundRobinPolicy::increSlowCounter(const std::string& nodepath)
{
    bool ret = SlowCounterMgr::increSlowCounter(nodepath, slow_counter);
    if (!ret)
    {
        LOG(WARNING) << "increase the slow counter failed.";
    }
}

void RoundRobinPolicy::updateSlowCounterForAll()
{
    // reload counter from shared data
    bool ret = SlowCounterMgr::readSharedSlowCounter(slow_counter);
    if (!ret)
    {
        LOG(WARNING) << "update slow counter from shared memory failed.";
    }
}

void
RoundRobinPolicy::updateCollections() {
    DLOG(INFO) << "updating collections map due to topology changes";
    collections.clear();
    backup_collections.clear();
    slow_counter.clear();
    SlowCounterMgr::clearSharedSlowCounter();
    
    BOOST_FOREACH(const string& collection, topology.getCollectionIndex()) {
        NodeCollectionsRange range = topology.getNodesFor(collection);
        string k(collection);
        //for(NodeCollectionsIterator it = range.first; it != range.second; ++it)
        //    LOG(INFO) << "collection: " << k << ", node : " << (*it).second.getPath();
        collections.insert(k, new NodeCollectionsList(range.first, range.second));
        ccounter[collection] = 0;
    }
    BOOST_FOREACH(const string& collection, backup_topology.getCollectionIndex()) {
        NodeCollectionsRange range = backup_topology.getNodesFor(collection);
        string k(collection);
        //for(NodeCollectionsIterator it = range.first; it != range.second; ++it)
        //    LOG(INFO) << "collection: " << k << ", node : " << (*it).second.getPath();
        backup_collections.insert(k, new NodeCollectionsList(range.first, range.second));
        ccounter[collection] = 0;
    }
}


const Sf1Node& 
RoundRobinPolicy::getNode() {
    size_t index;
    if (topology.count() == 0)
    {
        // using backup.
        index = counter++ % backup_topology.count();
        return backup_topology.getNodeAt(index);
    }
    index = counter++ % topology.count();
    return topology.getNodeAt(index);
}


const Sf1Node& 
RoundRobinPolicy::getNodeFor(const std::string& collection) {
    try
    {
        const NodeCollectionsList& list = collections.at(collection);
        if (list.size() == 0)
        {
            return getBackupNodeFor(collection);
        }
        return getNodeFor(collection, list);
    }
    catch(const std::exception& e)
    {
        return getBackupNodeFor(collection);
    }
}
    
const Sf1Node& RoundRobinPolicy::getBackupNodeFor(const std::string& collection)
{
    return getNodeFor(collection, backup_collections.at(collection));
}
    
const Sf1Node& RoundRobinPolicy::getNodeFor(const std::string& collection, const NodeCollectionsList& chose_from)
{
    const NodeCollectionsList& list = chose_from;
    size_t index = ccounter[collection]++ % list.size();
    size_t trynext = 0;
    while(trynext < list.size())
    {
        const std::string& state_str = list[index].second.getServiceState();
        if (state_str.empty() || state_str == "ReadyForRead")
        {
            std::map<std::string, size_t>::const_iterator cit = slow_counter.find(list[index].second.getPath());
            if (cit != slow_counter.end() && cit->second >= SLOW_THRESHOLD)
            {
                LOG(INFO) << "!!! this node is slow currently, skipping this " << cit->first << ",ip :" << list[index].second.getHost();
            }
            else
                break;
        }
        ++trynext;
        LOG(INFO) << "!!!! one of node is busy, try next !!!!!!" << list[index].second.getPath();
        index = ccounter[collection]++ % list.size();
    }

    if (trynext == list.size())
    {
        LOG(INFO) << "!!!! all node is BusyForWrite, just choose any one !!!!!!";
    }

    DLOG(INFO) << "index[" << collection << "] = " << index;
    
    return list[index].second;
}

NS_IZENELIB_SF1R_END
