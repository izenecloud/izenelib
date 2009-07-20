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
NS_IZENELIB_IR_BEGIN

namespace idmanager {

	template<typename NameString, typename NameID> class DocIdManager {
		typedef NameHook<NameString> NameHook;
		typedef IDHook<NameID> IDHook;
		typedef izenelib::sdb::unordered_sdb_1<NameHook, NameID> IdFinder;
		typedef izenelib::sdb::unordered_sdb_1<IDHook, NameString> NameFinder;

	public:

		/**
		 * @brief a constructor of DocIdManager.
		 *
		 * @details
		 *  - Create docIndexer_ and invertedDocIndexer_ which size is  the amount of 
		 *    DEFAULT_COLLECTION_SIZE. 
		 */
		DocIdManager(const string& sdbname="docid_manager",
				NameID initialDocIdValue =1, NameID maxDocIdValue = -2);

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
		bool getDocIdByDocName(NameID collectionId, const NameString& docName,
				NameID& docId);

		/**
		 * @brief a member function to offer a document name according to the ID.
		 *
		 * @param collectionId  a collection Id in which the document name is included.
		 * @param docId	        a document identifier which is used to get document name.
		 * @param docName	    a document name for the output.
		 * @return true  :  Given docId exists in the dictionary.	
		 * @return false :	Given docId does not exist in the dictionary.	
		 */
		bool getDocNameByDocId(NameID collectionId, NameID docId,
				NameString& docName);

		/**
		 * @brief a member function to display all the contents of the sequential db. this function is used for debugging. 
		 */
		void displaySDBList();
		
		void display() {
				idFinder_.display();
				nameFinder_.display();
		}

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
		bool insertDocName(NameID collectionId, const NameString& docName,
				NameID docId);

		/**
		 * @brief a member function to recover document string and ID of sequential db index file into dictionary.
		 *
		 * @param collectionId  a collection Id in which the document name is included.
		 * @param docName       a document name which is inserted to the dictionary.
		 * @param docId         a document ID which is the output of intersion.ng.
		 * @return true  :  Insertion is completed.	
		 * @return false :  Inserted string already exists in the dictionary..
		 */
		bool recoverDocName(NameID collectionId, const NameString& docName,
				NameID docId);

		/**
		 * @brief This function adds new pair of (UString, ID) to DocIdManager
		 * @param data the data in SerializedIDObjectType format. It contains 
		 * the string and id values
		 */
		/*void addData(unsigned int collectionId,
		 const NameString& docName, unsigned int& docId)*/

	

	private:
		NameID minID_; /// <initial value of DocId
		NameID maxID_; /// << maximum value of DocId
		NameID newID_;

		IdFinder idFinder_; ///< an indexer which gives ids according to the name.
		NameFinder nameFinder_; ///< an inverted indexer which gives name according to the id.		

	}; // end - class DocIdManager 


	template<typename NameString, typename NameID> DocIdManager<NameString, NameID>::DocIdManager(
			const string& sdbname, NameID initialDocIdValue, NameID maxDocIdValue) :
	minID_(initialDocIdValue), maxID_(maxDocIdValue),
	newID_(initialDocIdValue), idFinder_(sdbname+ "_name.sdb"),
	nameFinder_(sdbname+"_id.sdb") {		
		idFinder_.open();
		nameFinder_.open();
	} // end - IDFactory()


	template<typename NameString, typename NameID> DocIdManager<NameString, NameID>::~DocIdManager() {
	} // end - ~DocIdManager()

	template<typename NameString, typename NameID> bool DocIdManager<NameString,
	NameID>::getDocIdByDocName(NameID collectionId,
			const NameString& docName, NameID& docId) {
		NameHook nameHook;
		nameHook.collId = collectionId;
		nameHook.docName = docName;

		// If name string is found, return the id.
		if (idFinder_.getValue(nameHook, docId) ) {
			return true;
		} // end - if

		// Because there's no name string in idFinder, create new id according to the string. 
		docId = newID_;
		newID_++;

		IDHook idHook;
		idHook.collId = collectionId;
		idHook.docId = docId;

		// check correctness of input nameID
		if (newID_> maxID_)
		throw IDFactoryException(SF1_ID_FACTORY_OUT_OF_BOUND, __LINE__, __FILE__);

		// insert (nameString, nameID) to sdb
		idFinder_.insertValue(nameHook, docId);
		nameFinder_.insertValue(idHook, docName);
		return false;
	}

	template<typename NameString, typename NameID> bool DocIdManager<NameString,
	NameID>::getDocNameByDocId(NameID collectionId, NameID docId,
			NameString& docName) {
		IDHook idHook;
		idHook.collId = collectionId;
		idHook.docId = docId;
		return nameFinder_.getValue(idHook, docName);

	} // end - getDocNameByDocId()


	/**********************************************************
	 *                                Display SequentialDB List 
	 **********************************************************/

	template<typename NameString, typename NameID> void DocIdManager<NameString,
	NameID>::displaySDBList() {
		idFinder_.display();
		nameFinder_.display();
	} // end - displaySDBList()

	/*
	 void DocIdManager::addData(NameID collectionId,
	 const NameString& docName, NameID& docId) {
	 NameHook nameHook;
	 nameHook.colId = collectionId;
	 nameHook.docName = docName;
	 IDHook idHook;
	 idHook.collId = collectionId;
	 idHook.docId = docId;

	 idFinder_.insert(nameHook, docId);
	 nameFinder_.insert(idHook, docName);
	 }*/

} // end - namespace idmanager

NS_IZENELIB_IR_END

#endif // _DOC_ID_MANAGER_
