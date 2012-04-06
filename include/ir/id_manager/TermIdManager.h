/**
 * @file	TermIdManager.h
 * @brief	Header file of Term ID Manager Class
 * @author	Do Hyun Yun
 * @date    2008-11-18
 * @details
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

#ifndef _TERM_ID_MANAGER_
#define _TERM_ID_MANAGER_


#include "IDGenerator.h"
#include "IDStorage.h"
#include "IDFactory.h"
#include "IDFactoryErrorString.h"
#include "IDFactoryException.h"

/**
 * @brief a class to generate, serve, and manage all about of the term id.
 */
NS_IZENELIB_IR_BEGIN

namespace idmanager
{

template<typename NameString,
         typename NameID,
         typename IDGenerator   = HashIDGenerator<NameString, NameID>,
         typename IDStorage     = SDBIDStorage<NameString, NameID> >
class TermIdManager
{
    typedef IDFactory<NameString, NameID, IDGenerator, IDStorage> TermIDFactory;

public:

    /**
     * @brief Constructor of TermIdManager having term ids within a given range
     */
    TermIdManager(const string& storageName = "termid_manager")
    :   idFactory_(storageName) {}

    /**
     * @brief A Destructor.
     */
    ~TermIdManager() {}

public:
    /**
     * @brief a member function to offer a termID which exists in the dictionary.
     * If it is not in the dictionary, termIdManager will generate new term id and automatically inserts into dictionary.
     *
     * @param termString	a term string which is used to find the term ID.
     * @param termId        a term identifier which is the matched id value of termString.
     * @return true     : The term ID is in dictionary.
     * @return false    : There is no matched term ID in dictionary. New term Id generation and Insertion processes are done.
     */
    bool getTermIdByTermString(const NameString& termString, NameID& termId)
    {
        return idFactory_.getNameIDByNameString(termString, termId);
    }


    /**
     * @brief a member function to offer a term string according to the ID.
     *
     * @param termId	    a term identifier which is used to get term string.
     * @param termString	a term string for the output.
     * @return true  :  Given id exists in the dictionary.
     * @return false :	Given id does not exist in the dictionary.
     */
    bool getTermStringByTermId(NameID termId, NameString& termString)
    {
        return idFactory_.getNameStringByNameID(termId, termString);
    }

    /**
     * @brief a member function to offer a set of search result of term id list.
     * If one or more term strings are not matched in the dictionary,
     * 0 will be contained for each unmatched termIdList.
     *
     * @param termStringList    a string list
     * @param termIdList        a list of term IDs which is the result of searching
     * @return true  :          all the term strings in given list are matched in the dictionary.
     * @return false :          one or more term strings are not matched in the dictionary.
     */
    bool getTermIdListByTermStringList(
            const std::vector<NameString>& termStringList,
            std::vector<NameID>& termIdList)
    {
        bool ret;
        bool isAllIDFound = true;
        size_t sizeOfTermStringList = termStringList.size();

        // clear termIDList
        termIdList.clear();
        termIdList.resize(sizeOfTermStringList);

        for (size_t i = 0; i < sizeOfTermStringList; i++) {
            ret = getTermIdByTermString(termStringList[i], termIdList[i]);
            if (ret == false)
                isAllIDFound = false;
        } // end - for

        return isAllIDFound;
    }
    /**
     * @brief a memeber function to offer a list of term string by term id list. If one or more ids are not matched in the dictionary, 0 will be contained for each unmatched termIdList.
     *
     * @param termIdList        a list of term Ids which indicates the identifier.
     * @param termStringList    a list of term string which is the result of searching
     *
     * @return true     :       all the term ids are matched in the dictionary.
     * @return false    :       one or more term ids are not matched in the dictionary.
     */
    bool getTermStringListByTermIdList(const std::vector<NameID>& termIdList,
            std::vector<NameString>& termStringList)
    {
        bool ret;
        bool isAllTermFound = true;
        size_t sizeOfTermIdList = termIdList.size();

        // clear nameStringList
        termStringList.clear();
        termStringList.resize(sizeOfTermIdList);

        for (size_t i = 0; i < sizeOfTermIdList; i++) {
            ret = idFactory_.getNameStringByNameID(termIdList[i], termStringList[i]);
            if (ret == false)
                isAllTermFound = false;
        } // end - for
        return isAllTermFound;
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
     *        this function is used for debugging.
     */
    void display()
    {
        idFactory_.display();
    }


private:

    /**
     * @brief ID generator
     */
    TermIDFactory idFactory_;

}; // end - class TermIdManager

}
// end - namespace idmanager

NS_IZENELIB_IR_END

#endif // _TERM_ID_MANAGER_
