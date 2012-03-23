/* 
 * File:   NodeContainer.hpp
 * Author: Paolo D'Apice
 *
 * Created on February 29, 2012, 4:42 PM
 */

#ifndef NODECONTAINER_HPP
#define	NODECONTAINER_HPP

#include "../config.h"
#include "Sf1Node.hpp"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <map>
#include <set>


NS_IZENELIB_SF1R_BEGIN
        
namespace bm = boost::multi_index;


/** Index tag for node path. */
struct path {};

/** Index tag for node list. */
struct list {};

/**
 * Multi indexed container for the actual SF1 topology.
 * It is possible to access a node by its path (as in a map)
 * or by its position (as in a list).
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
> NodeContainer;

/** Index on tag path. */
typedef NodeContainer::index<path>::type NodePathIndex;
/** Iterator on tag path. */
typedef NodePathIndex::iterator NodePathIterator;

/** Index on tag list.*/
typedef NodeContainer::index<list>::type NodeListIndex;
/** Iterator on tag list. */
typedef NodeListIndex::iterator NodeListIterator;
/** Iterator range on \ref NodeListIndex. */
typedef std::pair<NodeListIterator, NodeListIterator> NodeListRange;


/** Container providing a view on collections. */
typedef std::multimap<std::string, Sf1Node> NodeCollectionsContainer;
/** Iterator on \ref CollectionsContainer. */
typedef NodeCollectionsContainer::iterator NodeCollectionsIterator;
/** Iterator range on \ref CollectionsContainer. */
typedef std::pair<NodeCollectionsIterator, NodeCollectionsIterator> NodeCollectionsRange;
    
/** Index of available collections (keys of \ref CollectionsContainer). */
typedef std::set<std::string> NodeCollectionsIndex;

/** List of pointers to NodeCollectionsContainer::value_type. */
typedef std::vector<NodeCollectionsContainer::value_type> NodeCollectionsList;


NS_IZENELIB_SF1R_END

#endif	/* NODECONTAINER_HPP */
