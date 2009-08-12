/**
 * @file	DocIdManager.h
 * @brief	Header file of Document Id Manager Class
 * @author	Do Hyun Yun
 * @details
 *
 * ==============
 *
 * Using SDB/hash
 * @author Peisheng Wang
 * @date 2009-04-16
 *
 * ==============
 *
 * Refactor
 * @author Wei Cao
 * @date 2009-08-07
 *
 * ==============
 */

#ifndef _DOC_ID_MANAGER_
#define _DOC_ID_MANAGER_

#include "IDManagerTypes.h"
#include "IDFactoryException.h"
#include "IDFactoryErrorString.h"

/**
 * @brief a class to generate, serve, and manage all about of the document id.
 */
NS_IZENELIB_IR_BEGIN

namespace idmanager {

    template<typename NameString,
             typename NameID,
             typename IDFactory>
    class DocIdManager {

    public:

		/**
		 * @brief a constructor of DocIdManager.
		 *
		 * @details
		 *  - Initialize IDFactory
		 */
		DocIdManager(const string& sdbname="docid_manager");

		~DocIdManager();

	public:

		/**
		 * @brief a member function to offer a document ID which exists in the dictionary.
		 *
		 * @param docName	    a document name string which is used to find the document ID.
		 * @param docId         a document identifier which is the result of this interface.
		 * @return true     :   The document ID is in dictionary.
		 * @return false    :   There is no matched ID in dictionary.
		 */
		bool getDocIdByDocName(const NameString& docName, NameID& docId);

		/**
		 * @brief a member function to offer a document name according to the ID.
		 *
		 * @param docId	        a document identifier which is used to get document name.
		 * @param docName	    a document name for the output.
		 * @return true  :  Given docId exists in the dictionary.
		 * @return false :	Given docId does not exist in the dictionary.
		 */
		bool getDocNameByDocId(NameID docId, NameString& docName);

		/**
		 * @brief a member function to display all the contents of the sequential db.
		 *          this function is provided for debugging purpose.
		 */
		void display();

	private:

        IDFactory idFactory_;

	}; // end - class DocIdManager


	template<typename NameString, typename NameID, typename IDFactory>
	DocIdManager<NameString, NameID, IDFactory>::DocIdManager(
        const string& sdbname)
    :
        idFactory_(sdbname)
    {
    } // end - IDFactory()


	template<typename NameString, typename NameID, typename IDFactory>
	DocIdManager<NameString, NameID, IDFactory>::~DocIdManager()
	{
    } // end - ~DocIdManager()

	template<typename NameString, typename NameID, typename IDFactory>
	bool DocIdManager<NameString, NameID, IDFactory>::getDocIdByDocName(
        const NameString& docName,
        NameID& docId)
    {
        return idFactory_.getNameIDByNameString(docName, docId);
    } // end - getDocIdByDocName()

	template<typename NameString, typename NameID, typename IDFactory>
	bool DocIdManager<NameString, NameID, IDFactory>::getDocNameByDocId(
        NameID docId,
        NameString& docName)
    {
        return idFactory_.getNameStringByNameID(docId, docName);
    } // end - getDocNameByDocId()

	template<typename NameString, typename NameID, typename IDFactory>
	void DocIdManager<NameString, NameID, IDFactory>::display()
	{
		idFactory_.display();
    } // end - display()

} // end - namespace idmanager

NS_IZENELIB_IR_END

#endif // _DOC_ID_MANAGER_
