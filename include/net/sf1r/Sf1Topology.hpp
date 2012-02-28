/* 
 * File:   Sf1Topology.hpp
 * Author: Paolo D'Apice
 *
 * Created on February 22, 2012, 9:55 AM
 */

#ifndef SF1TOPOLOGY_HPP
#define	SF1TOPOLOGY_HPP

#include "config.h"
#include "Sf1Node.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <map>
#include <set>
#include <vector>


NS_IZENELIB_SF1R_BEGIN

namespace bm = boost::multi_index;


/** Index tag for searching by node path. */
struct path {};

/** Index tag for nodes list. */
struct list {};

/**
 * Multi indexed container for the actual SF1 topology.
 */
typedef boost::multi_index_container<
    Sf1Node,
    bm::indexed_by<
        bm::hashed_unique<
            bm::tag<path>,
            bm::const_mem_fun<
                Sf1Node, std::string, &Sf1Node::getPath
            >
        >,
        bm::random_access<
            bm::tag<list>
        >
    >
> NodesContainer;

/** Index on tag path. */
typedef NodesContainer::index<path>::type NodesPathIndex;
/** Iterator on tag path. */
typedef NodesPathIndex::iterator NodesPathIterator;

/** Index on tag list.*/
typedef NodesContainer::index<list>::type NodesListIndex;
/** Iterator on tag list. */
typedef NodesListIndex::iterator NodesListIterator;


/** Container providing a view on collections. */
typedef std::multimap<std::string, std::string> CollectionsContainer;
/** Iterator on \ref CollectionsContainer. */
typedef CollectionsContainer::iterator CollectionsIterator;
/** Iterator range on \ref CollectionsContainer. */
typedef std::pair<CollectionsIterator, CollectionsIterator> CollectionsRange;
    
/** Index of available collections (keys of \ref CollectionsContainer). */
typedef std::set<std::string> CollectionsIndex;


/**
 * Representation of the SF1 cluster topology.
 * 
 * The cluster topology is composed of a set of nodes hosting a set of 
 * collections and accessible through their ZooKeeper path. 
 * This class keeps two view on the topology:
 * - nodes by path: each node is accessible through its path
 * - collections by nodes: each collections is hosted on a set of nodes
 *
 * Please note that this class is <b>not</b> thread-safe.
 * Thread-safety is delegated to the user class.
 */
class Sf1Topology {
public:
    
    /**
    * Add a new node to the current topology.
    * @param path The new node path.
    * @param data The new node data string.
    */
    void addNode(const std::string& path, const std::string& data);

    /**
    * Update a node in the current topology.
    * @param path The path of the node to be updated.
    * @param data The new node data string.
    */
    void updateNode(const std::string& path, const std::string& data);

    /**
    * Remove a node from the current topology.
    * @param path The path of the node to be removed.
    */
    void removeNode(const std::string& path);

    /**
     * Get the list of nodes in the actual topology having the specified collection.
     * @param nodes The list that will contain the requested nodes.
     * @param collection The collection name.
     */
    void getNodes(std::vector<Sf1Node>& nodes, const std::string& collection);
    
    /**
     * Get the list of all the nodes in the actual topology.
     * @param nodes The list that will contain the requested nodes.
     */
    void getNodes(std::vector<Sf1Node>& nodes);
    
    /**
     * @return The number of actual nodes.
     */
    size_t count() const {
        return nodes.size();
    }
    
    /**
     * Checks for node presence.
     * @param node The node path.
     * @return true if the searched node is present.
     */
    bool isPresent(const std::string& node) {
        return not (nodes.find(node) == nodes.end());
    }
    
private:
    // nodes in topology
    NodesContainer nodes;
    
    // collections by node
    CollectionsContainer collections;
    // collections index
    CollectionsIndex index;
    
    
#ifdef ENABLE_ZK_TEST
    
public: // for tests only 
    NodesContainer& _nodes() { return nodes; }
    CollectionsContainer& _colls() { return collections; }
    CollectionsIndex& _index() { return index; }
    
#endif
    
};


NS_IZENELIB_SF1R_END

#endif	/* SF1TOPOLOGY_HPP */
