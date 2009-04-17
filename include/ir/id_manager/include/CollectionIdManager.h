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

#include <IDFactory.h>
/**
 * @brief a class to generate, serve, and manage all about of the collection id. 
 */

namespace idmanager 
{

    class CollectionIdManager : protected IDFactory<wiselib::UString, unsigned int>
    {
    public:

        CollectionIdManager(const string& sdbname="col_manager", unsigned int initialValue = 1, unsigned int maxValue = -2); 

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
        bool getCollectionIdByCollectionName(const wiselib::UString& collectionName, unsigned int& collectionId );

        /**
         * @brief a member function to offer a collection name according to the ID.
         *
         * @param collectionId      a collection identifier which is used to get collection name.
         * @param collectionName	a collection name for the output.
         *
         * @return true     :       Given collectionId exists in the dictionary.	
         * @return false    :	    Given collectionId does not exist in the dictionary.	
         */
        bool getCollectionNameByCollectionId(unsigned int collectionId, wiselib::UString& collectionName);

        /**
         * @brief a member function to get the total name count in the collection Indexer.
         *
         * @return The number of names in the collection Indexer.
         */
        unsigned int getTotalNameCount() { return static_cast<unsigned int>(idFinder_.numItems()); }

        /**
         * @brief a member function to display all the contents of the sequential db. this function is used for debugging. 
         */
        void displaySDBList();

    private:    	

        friend class IDManager;
    }; // end - class CollectionIdManager 

} // end - namespace sf1v5 

#endif // _COLLECTION_ID_MANAGER_
