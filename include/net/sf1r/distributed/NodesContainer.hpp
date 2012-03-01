/* 
 * File:   NodesContainer.hpp
 * Author: paolo
 *
 * Created on February 29, 2012, 4:42 PM
 */

#ifndef NODESCONTAINER_HPP
#define	NODESCONTAINER_HPP

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
> NodesContainer;

/** Index on tag path. */
typedef NodesContainer::index<path>::type NodesPathIndex;
/** Iterator on tag path. */
typedef NodesPathIndex::iterator NodesPathIterator;

/** Index on tag list.*/
typedef NodesContainer::index<list>::type NodesListIndex;
/** Iterator on tag list. */
typedef NodesListIndex::iterator NodesListIterator;


NS_IZENELIB_SF1R_END

#endif	/* NODESCONTAINER_HPP */
