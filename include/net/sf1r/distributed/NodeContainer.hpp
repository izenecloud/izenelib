/* 
 * File:   NodeContainer.hpp
 * Author: paolo
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

/** List of pointers to nodes. */
typedef std::vector<Sf1NodePtr> NodeList;


NS_IZENELIB_SF1R_END

#endif	/* NODECONTAINER_HPP */
