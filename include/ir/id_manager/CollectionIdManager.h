/**
 * @file	CollectionIdManager.h
 * @brief	Header file of Collection Id Manager Class 
 * @author	Do Hyun Yun
 * @date    2008-07-05 : Created CollectionIdManager.cpp 
 * @details
 *  -Log
 */

#ifndef _COLLECTION_ID_MANAGER_ 
#define _COLLECTION_ID_MANAGER_

#include "IDFactory.h"
/**
 * @brief a class to generate, serve, and manage all about of the collection id. 
 */
NS_IZENELIB_IR_BEGIN

namespace idmanager {

template<typename NameString, typename NameID> class CollectionIdManager :
	protected IDFactory<NameString, NameID> {
public:

	CollectionIdManager(const string& sdbname="col_manager",
			NameID initialValue = 1, NameID maxValue = -2);

	~CollectionIdManager();

public:

	/**
	 * @brief a member function to offer a collection ID which exists in the dictionary.
	 * If there is no matched id of given collection name, collection id manager generates new collection id and insert into dictionary.
	 *
	 * @param collectionName	a collection name string which is used to find the collection ID.
	 * @param collectionId      a collection identifier which is the matched id of given collection name.
	 *
	 * @return true     :       There is matched collection ID in dictionary. 
	 * @return false    :       There is no matched ID in dictionary. New id generation and insertion process is done.
	 */
	bool getCollectionIdByCollectionName(const NameString& collectionName,
			NameID& collectionId);

	/**
	 * @brief a member function to offer a collection name according to the ID.
	 *
	 * @param collectionId      a collection identifier which is used to get collection name.
	 * @param collectionName	a collection name for the output.
	 *
	 * @return true     :       Given collectionId exists in the dictionary.	
	 * @return false    :	    Given collectionId does not exist in the dictionary.	
	 */
	bool getCollectionNameByCollectionId(NameID collectionId,
			NameString& collectionName);

	/**
	 * @brief a member function to get the total name count in the collection Indexer.
	 *
	 * @return The number of names in the collection Indexer.
	 */
	unsigned int getTotalNameCount() {
		return static_cast<unsigned int>( IDFactory<NameString, NameID>::idFinder_.numItems() );
	}

	/**
	 * @brief a member function to display all the contents of the sequential db. this function is used for debugging. 
	 */
	
	void displaySDBList();
	
	void display(){
		IDFactory<NameString, NameID>::display();
	}
	
}; // end - class CollectionIdManager 

template<typename NameString, typename NameID> CollectionIdManager<NameString,
		NameID>::CollectionIdManager(const string& sdbname,
		NameID initialValue, NameID maxValue) :
	IDFactory<NameString, NameID>(sdbname, initialValue, maxValue) {

} // end - CollectionIdManager()

template<typename NameString, typename NameID> CollectionIdManager<NameString,
		NameID>::~CollectionIdManager() {
} // end - ~CollectionIdManager()


template<typename NameString, typename NameID> bool CollectionIdManager<
		NameString, NameID>::getCollectionIdByCollectionName(
		const NameString& collectionName, NameID& collectionId) {
	return getNameIDByNameString(collectionName, collectionId);
} // end - get collection id by collection name ()

template<typename NameString, typename NameID> bool CollectionIdManager<
		NameString, NameID>::getCollectionNameByCollectionId(
		NameID collectionId, NameString& collectionName) {
	return getNameStringByNameID(collectionId, collectionName);
} // end - get collection name by collection id ()

/**********************************************************
 *                                Display SequentialDB List 
 **********************************************************/

template<typename NameString, typename NameID> void CollectionIdManager<
		NameString, NameID>::displaySDBList() {
} // end - displaySDBList()


} // end - namespace idmanager 

NS_IZENELIB_IR_END

#endif // _COLLECTION_ID_MANAGER_
