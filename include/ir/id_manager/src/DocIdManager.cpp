/**
 * @file	DocIdManager.cpp
 * @brief	Source file of Document Id Manager Class 
 * @author	Do Hyun Yun
 * @date    2008-07-05 : Created DocIdManager.cpp 
 * @details
 *  - Log
 *      - 2008-07-05 : Created DocIdManager.cpp 
 *      - 2008-08-07 : Change string -> UString
 *      - 2008-09-07 : Disable Sequential DB because of its unstableness 
 *      - 2008-10-06 : Added index file writing check mode.
 *  - TODO
 *      - Request new id from document manager
 * 
 * 
 */

#include <DocIdManager.h> 
#include <IDFactoryException.h>
#include <IDFactoryErrorString.h>

/**********************************************************
 *                                 Constructor & Destructor
 **********************************************************/

namespace idmanager {

DocIdManager::DocIdManager(const string& sdbname, unsigned int initialDocIdValue,
		unsigned int maxDocIdValue) :
	minID_(initialDocIdValue), maxID_(maxDocIdValue),
			newID_(initialDocIdValue), idFinder_(sdbname+ "_name.sdb"),
			nameFinder_(sdbname+"_id.sdb") {
	idFinder_.open();
	nameFinder_.open();
} // end - IDFactory()


DocIdManager::~DocIdManager() {
} // end - ~DocIdManager()


bool DocIdManager::getDocIdByDocName(unsigned int collectionId,
		const wiselib::UString& docName, unsigned int& docId) {
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

bool DocIdManager::getDocNameByDocId(unsigned int collectionId,
		unsigned int docId, wiselib::UString& docName) {
	IDHook idHook;
	idHook.collId = collectionId;
	idHook.docId = docId;
	return nameFinder_.getValue(idHook, docName);

} // end - getDocNameByDocId()


/**********************************************************
 *                                Display SequentialDB List 
 **********************************************************/

void DocIdManager::displaySDBList() {
	idFinder_.display();
	nameFinder_.display();
} // end - displaySDBList()

/*
 void DocIdManager::addData(unsigned int collectionId,
 const wiselib::UString& docName, unsigned int& docId) {
 NameHook nameHook;
 nameHook.colId = collectionId;
 nameHook.docName = docName;
 IDHook idHook;
 idHook.collId = collectionId;
 idHook.docId = docId;

 idFinder_.insert(nameHook, docId);
 nameFinder_.insert(idHook, docName);
 }*/

} // end - namespace sf1v5


