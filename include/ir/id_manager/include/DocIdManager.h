/**
 * @file	DocIdManager.h
 * @brief	Header file of Document Id Manager Class 
 * @author	Do Hyun Yun
 * @details
 * 
 *================
 * Using SDB/hash
 *  
 * @Peisheng Wang
 * @date 2009-04-16
 */

#ifndef _DOC_ID_MANAGER_ 
#define _DOC_ID_MANAGER_

#include "IDManagerTypes.h"
/**
 * @brief a class to generate, serve, and manage all about of the document id. 
 */

namespace idmanager {

class DocIdManager {
	typedef unsigned int NameId;
	typedef wiselib::UString NameString;
	typedef izenelib::util::NullLock LockType;
	typedef izenelib::am::tc_hash<NameHook, NameId> SDB_HASH_SI;
	typedef izenelib::am::tc_hash<IDHook, NameString> SDB_HASH_IS;
	//typedef izenelib::sdb::SequentialDB<NameHook, NameId, LockType, SDB_HASH_SI> IdFinder;
	//typedef izenelib::sdb::SequentialDB<IDHook, NameString, LockType, SDB_HASH_IS> NameFinder;
	typedef izenelib::sdb::SequentialDB<NameHook, NameId, LockType> IdFinder;
	typedef izenelib::sdb::SequentialDB<IDHook, NameString, LockType> NameFinder;

public:

	/**
	 * @brief a constructor of DocIdManager.
	 *
	 * @details
	 *  - Create docIndexer_ and invertedDocIndexer_ which size is  the amount of 
	 *    DEFAULT_COLLECTION_SIZE. 
	 */
	DocIdManager(const string& sdbname="docid_manager", unsigned int initialDocIdValue =1,
			unsigned int maxDocIdValue = -2);

	~DocIdManager();

public:

	/**
	 * @brief a member function to offer a document ID which exists in the dictionary.
	 * If there isn't matched id value, termIdManager generate new term id and insert into dictionary.
	 *
	 * @param collectionId  a collection Id in which the document name is included.
	 * @param docName	    a document name string which is used to find the document ID.
	 * @param docId         a document identifier which is the result of this interface.
	 * @return true     :   The document ID is in dictionary. 
	 * @return false    :   There is no matched ID in dictionary.
	 */
	bool getDocIdByDocName(unsigned int collectionId,
			const wiselib::UString& docName, unsigned int& docId);

	/**
	 * @brief a member function to offer a document name according to the ID.
	 *
	 * @param collectionId  a collection Id in which the document name is included.
	 * @param docId	        a document identifier which is used to get document name.
	 * @param docName	    a document name for the output.
	 * @return true  :  Given docId exists in the dictionary.	
	 * @return false :	Given docId does not exist in the dictionary.	
	 */
	bool getDocNameByDocId(unsigned int collectionId, unsigned int docId,
			wiselib::UString& docName);

	/**
	 * @brief a member function to display all the contents of the sequential db. this function is used for debugging. 
	 */
	void displaySDBList();

private:

	/**
	 * @brief a member function to insert document string and ID into dictionary.
	 *
	 * @param collectionId  a collection Id in which the document name is included.
	 * @param docName       a document name which is inserted to the dictionary.
	 * @param docId         a document ID which is the output of intersion.ng.
	 * @return true  :  Insertion is completed.	
	 * @return false :  Inserted string already exists in the dictionary..
	 */
	bool insertDocName(unsigned int collectionId,
			const wiselib::UString& docName, unsigned int docId);

	/**
	 * @brief a member function to recover document string and ID of sequential db index file into dictionary.
	 *
	 * @param collectionId  a collection Id in which the document name is included.
	 * @param docName       a document name which is inserted to the dictionary.
	 * @param docId         a document ID which is the output of intersion.ng.
	 * @return true  :  Insertion is completed.	
	 * @return false :  Inserted string already exists in the dictionary..
	 */
	bool recoverDocName(unsigned int collectionId,
			const wiselib::UString& docName, unsigned int docId);

	/**
	 * @brief This function adds new pair of (UString, ID) to DocIdManager
	 * @param data the data in SerializedIDObjectType format. It contains 
	 * the string and id values
	 */
	/*void addData(unsigned int collectionId,
	 const wiselib::UString& docName, unsigned int& docId)*/

	friend class IDManager;
private:
	unsigned int minID_; /// <initial value of DocId
	unsigned int maxID_; /// << maximum value of DocId
	unsigned int newID_;

	IdFinder idFinder_; ///< an indexer which gives ids according to the name.
	NameFinder nameFinder_; ///< an inverted indexer which gives name according to the id.		

}; // end - class DocIdManager 

} // end - namespace sf1v5

#endif // _DOC_ID_MANAGER_
