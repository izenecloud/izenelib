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
 * Refactor to a policy-based design to make IDManager as flexible as possible
 * @author Wei Cao
 * @date 2009-08-07
 *
 * ==============
 */

#ifndef _DOC_ID_MANAGER_
#define _DOC_ID_MANAGER_

#include <types.h>

#include "IDGenerator.h"
#include "IDStorage.h"
#include "IDFactory.h"
#include "IDFactoryErrorString.h"
#include "IDFactoryException.h"

/**
 * @brief a class to generate, serve, and manage all about of the document id.
 */
NS_IZENELIB_IR_BEGIN

namespace idmanager {

template<typename NameString,
         typename NameID,
         typename IDGenerator   = OldUniqueIDGenerator<NameString, NameID>,
         typename IDStorage     = TCIDStorage<NameString, NameID> >
class DocIdManager {

    typedef IDFactory<NameString, NameID, IDGenerator, IDStorage> DocIDFactory;

public:

    /**
     * @brief a constructor of DocIdManager.
     *
     * @details
     *  - Initialize IDFactory
     */
    DocIdManager(const string& storageName="docid_manager");

    ~DocIdManager();

public:

    /**
     * @brief a member function to offer a document ID which exists in the dictionary.
     *
     * @param docName	    a document name string which is used to find the document ID.
     * @param docId         a document identifier which is the result of this interface.
     * @param insert	   whether insert docName to IDManager;
     * @return true     :   The document ID is in dictionary.
     * @return false    :   There is no matched ID in dictionary.
     */
    bool getDocIdByDocName(const NameString& docName, NameID& docId, bool insert = true);

    /**
     * @brief a member function to set the document ID to the new value so that it can satisfy the incremental document ID semantic.
     *
     * @param docName		a unique string of the document which is used to distinguish between documents.
     * @param updatedId  the new old old document identifier which matches to the document name.
     */
    void updateDocIdByDocName(const NameString& docName, NameID& updatedId);

    /**
     * @brief a member function to offer a document name according to the ID.
     *
     * @param docId	        a document identifier which is used to get document name.
     * @param docName	    a document name for the output.
     * @return true  :  Given docId exists in the dictionary.
     * @return false :	Given docId does not exist in the dictionary.
     */
    bool getDocNameByDocId(const NameID& docId, NameString& docName);

    /**
    * @brief Get the maximum doc id.
    * @return max doc id, 0 for no doc id exists or this function is not supported
    */
    NameID getMaxDocId() const
    {
        return idFactory_.getMaxNameID();
    }

    void flush()
    {
        idFactory_.flush();
    }

    void close()
    {
        idFactory_.close();
    }

    /**
     * @brief a member function to display all the contents of the sequential db.
     *          this function is provided for debugging purpose.
     */
    void display();

private:

    DocIDFactory idFactory_;
}; // end - class DocIdManager


template<typename NameString, typename NameID, typename IDGenerator, typename IDStorage>
DocIdManager<NameString, NameID, IDGenerator, IDStorage>::DocIdManager(
    const string& storageName)
:
    idFactory_(storageName)
{
} // end - IDFactory()


template<typename NameString, typename NameID, typename IDGenerator, typename IDStorage>
DocIdManager<NameString, NameID, IDGenerator, IDStorage>::~DocIdManager()
{
} // end - ~DocIdManager()

template<typename NameString, typename NameID, typename IDGenerator, typename IDStorage>
bool DocIdManager<NameString, NameID, IDGenerator, IDStorage>::getDocIdByDocName(
    const NameString& docName,
    NameID& docId,
    bool insert)
{
    return idFactory_.getNameIDByNameString(docName, docId, insert);
} // end - getDocIdByDocName()

template<typename NameString, typename NameID, typename IDGenerator, typename IDStorage>
void DocIdManager<NameString, NameID, IDGenerator, IDStorage>::updateDocIdByDocName(
    const NameString& docName,
    NameID& updatedId)
{
    idFactory_.updateNameIDByNameString(docName, updatedId);
} // end - updateDocIdByDocName()


template<typename NameString, typename NameID, typename IDGenerator, typename IDStorage>
bool DocIdManager<NameString, NameID, IDGenerator, IDStorage>::getDocNameByDocId(
    const NameID& docId,
    NameString& docName)
{
    return idFactory_.getNameStringByNameID(docId, docName);
} // end - getDocNameByDocId()

template<typename NameString, typename NameID, typename IDGenerator, typename IDStorage>
void DocIdManager<NameString, NameID, IDGenerator, IDStorage>::display()
{
    idFactory_.display();
} // end - display()

} // end - namespace idmanager

NS_IZENELIB_IR_END

#endif // _DOC_ID_MANAGER_
