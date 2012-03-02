/* 
 * File:   CollectionsContainer.hpp
 * Author: paolo
 *
 * Created on February 29, 2012, 4:48 PM
 */

#ifndef COLLECTIONSCONTAINER_HPP
#define	COLLECTIONSCONTAINER_HPP

#include "../config.h"
#include <map>
#include <set>

NS_IZENELIB_SF1R_BEGIN


/** Container providing a view on collections. */
typedef std::multimap<std::string, Sf1NodePtr> CollectionsContainer;
/** Iterator on \ref CollectionsContainer. */
typedef CollectionsContainer::iterator CollectionsIterator;
/** Iterator range on \ref CollectionsContainer. */
typedef std::pair<CollectionsIterator, CollectionsIterator> CollectionsRange;
    
/** Index of available collections (keys of \ref CollectionsContainer). */
typedef std::set<std::string> CollectionsIndex;


NS_IZENELIB_SF1R_END

#endif	/* COLLECTIONSCONTAINER_HPP */
