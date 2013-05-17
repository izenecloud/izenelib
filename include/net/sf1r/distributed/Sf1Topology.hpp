/* 
 * File:   Sf1Topology.hpp
 * Author: Paolo D'Apice
 *
 * Created on February 22, 2012, 9:55 AM
 */

#ifndef SF1TOPOLOGY_HPP
#define	SF1TOPOLOGY_HPP

#include "../config.h"
#include "Sf1Node.hpp"
#include "NodeContainer.hpp"
#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>
#include <vector>

NS_IZENELIB_SF1R_BEGIN

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
class Sf1Topology : private boost::noncopyable {
public:
    
    /// Constructor.
    Sf1Topology();
    
    /// Destructor.
    ~Sf1Topology();
    
    /**
     * Add a new node to the current topology.
     * @param path The new node path.
     * @param data The new node data string.
     * @param emit If true the signal \ref changed is emitted.
     */
    void addNode(const std::string& path, const std::string& data, bool emit = true);

    /**
     * Update a node in the current topology.
     * @param path The path of the node to be updated.
     * @param data The new node data string.
     * @param emit If true the signal \ref changed is emitted.
     */
    void updateNode(const std::string& path, const std::string& data, bool emit = true);

    /**
     * Remove a node from the current topology.
     * @param path The path of the node to be removed.
     * @param emit If true the signal \ref changed is emitted.
     */
    void removeNode(const std::string& path, bool emit = true);
    
    void clearNodes();
    /**
     * Signal emitted on topology changes.
     */
    boost::signals2::signal<void ()> changed;
    
public:
    
    /**
     * Retrieve a node at a given path.
     * @param path The path of the node be retrieved
     * @return A reference to the requested node.
     */
    const Sf1Node& getNodeAt(const std::string& path);
    
    /**
     * Retrieve a node at a given position
     * @param position The position index within the list view of nodes.
     * @return A reference to the requested node.
     * @see NodeContainer
     */
    const Sf1Node& getNodeAt(const size_t& position);

    const NodeCollectionsIndex& getCollectionIndex() const { 
        return collectionsIndex; 
    }
    
    /**
     * Get a view on all the nodes in the actual topology 
     * hosting the specified collection.
     * @param collection The collection name.
     * @return An iterator range.
     */
    NodeCollectionsRange getNodesFor(const std::string& collection);
        
    /**
     * Get a view on all the nodes in the actual topology.
     * @return An iterator range.
     */
    NodeListRange getNodes();
    
    /**
     * @return The number of actual nodes.
     */
    size_t count() const {
        return nodes.size();
    }
    
    /**
     * @param collection The collection name
     * @return The number of actual nodes hosting the given collection.
     */
    size_t count(const std::string collection) const {
        return nodeCollections.count(collection);
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
    NodeContainer nodes;
    
    // collections by node
    NodeCollectionsContainer nodeCollections;
    // collections index
    NodeCollectionsIndex collectionsIndex;
    
    
#ifdef ENABLE_ZK_TEST
    
public: // for tests only 
    NodeContainer& _nodes() { return nodes; }
    NodeCollectionsContainer& _colls() { return nodeCollections; }
    
#endif
    
};


NS_IZENELIB_SF1R_END

#endif	/* SF1TOPOLOGY_HPP */
