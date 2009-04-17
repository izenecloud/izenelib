/**
 * @file	CollectionIdManager.cpp
 * @brief	Source file of Collection Id Manager Class 
 * @author	Do Hyun Yun
 * @date    2008-07-05 : Created CollectionIdManager.cpp 
 * @details
 *
 *  -Log
 *      - 2008-07-05 : Created CollectionIdManager.cpp 
 *      - 2008-08-05 : Change string -> UString
 *      - 2008-09-07 : Disable Sequential DB because of its unstableness 
 *      - 2008-10-06 : Added index file writing check mode.
 *      - 2009-02-10 : Change sf1lib -> wiselib
 *\n
 *  - TODO
 *      - Request new id from document manager
 */

#include <CollectionIdManager.h>

/**********************************************************
 *                                 Constructor & Destructor
 **********************************************************/

namespace idmanager {

CollectionIdManager::CollectionIdManager(const string& sdbname, unsigned int initialValue, unsigned int maxValue):
			IDFactory<wiselib::UString, unsigned int>(sdbname, initialValue, maxValue)
{
	
} // end - CollectionIdManager()


CollectionIdManager::~CollectionIdManager()
{
} // end - ~CollectionIdManager()


bool CollectionIdManager::getCollectionIdByCollectionName(const wiselib::UString& collectionName, unsigned int& collectionId)
{
	return getNameIDByNameString(collectionName, collectionId);	
} // end - get collection id by collection name ()


bool CollectionIdManager::getCollectionNameByCollectionId(unsigned int collectionId, wiselib::UString& collectionName)
{
	return getNameStringByNameID( collectionId, collectionName);
} // end - get collection name by collection id ()

/**********************************************************
 *                                Display SequentialDB List 
 **********************************************************/

void CollectionIdManager::displaySDBList()
{
} // end - displaySDBList()

} // end - namespace sf1v5  


