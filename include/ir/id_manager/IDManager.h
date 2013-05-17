/**
 * @file	IDManager.h
 * @brief	Header file of ID Manager Class
 * @author	Do Hyun Yun
 * @date    2008-06-05
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
 * Use LuxIO instead of SDB in AutoFillIDManager
 * @author Hongliang Zhao
 * @date 2013-02-15
 */

#ifndef _ID_MANAGER_
#define _ID_MANAGER_

#include "WildcardQueryManager.h"
#include "DocIdManager.h"
#include "TermIdManager.h"
#include "IDGenerator.h"
#include "IDStorage.h"

#include <types.h>
#include <util/ustring/UString.h>
#include <3rdparty/am/luxio/array.h>

/**
 * @brief a class to manage all kinds of operations about ID.
 *
 * @details
 * IDManager controls many kinds of IDs: Document ID, term ID,
 * or any other types of ID which are requested from specific manager.
 * ID data or Key data can be easily taken out from IDManager when the
 * matched Key or ID is given. If new key is inserted into the IDManager,
 * IDManager generates new ID which doesn't have duplication in the
 * vocabulary (storage) of IDManager. Different ID set is managed by its
 * type information. It means different ID type is stored in different
 * vocabulary storage. IDManager use UString class for the Key value.
 *
 *  - TODO List
 *      - There's multiple definition linking error while compiling test files.
 *        Currently I temporary set the option -Xlinker -zmuldefs to fix them.
 */

NS_IZENELIB_IR_BEGIN

namespace idmanager {

class IDManagerBase {

public:

    virtual ~IDManagerBase(){};

    virtual bool getTermIdByTermString(const char* termStringBuffer,
        const size_t termStringLength, uint32_t & termId) {return false;}

    virtual bool getTermIdByTermString(const char* termStringBuffer,
        const size_t termStringLength, uint64_t & termId) {return false;}

};

#define MAJOR_VERSION "1"
#define MINOR_VERSION "0"
#define PATCH_VERSION "20091210"

/**
 * @brief Combination of both TermIdManager, DocIdManager and RegexpManager.
 */

template<typename TermType,
         typename DocType,
         typename IDType,
         typename LockType = izenelib::util::NullLock,
         typename WildcardQueryHandler = EmptyWildcardQueryHandler<TermType, IDType>,
         typename TermIDGenerator = HashIDGenerator<TermType, IDType>,
         typename TermIDStorage = HDBIDStorage<TermType, IDType, LockType>,
         typename DocIDGenerator = UniqueIDGenerator<DocType, IDType, LockType>,
         typename DocIDStorage = HDBIDStorage<DocType, IDType, LockType> >
class _IDManager : public IDManagerBase
{
public:
    _IDManager(const string& storageName = "idm")
    :
        termIdManager_(storageName + "_tid"),
        docIdManager_(storageName + "_did"),
        wildcardQueryManager_(storageName + "_regexp")
    {
        version_ = "ID Manager - ver. alpha ";
        version_ += MAJOR_VERSION;
        version_ += ".";
        version_ += MINOR_VERSION;
        version_ += ".";
        version_ += PATCH_VERSION;
    }

    ~_IDManager()
    {
    }

    /**
     * @brief a member function to get term ID from vocabulary which matches to the given term string.
     *
     * @param termString	a UString object which contains term string.
     * @param termId	    a term identifier which matches to the term string.
     * @return true  : 	Term exists in the dictionary.
     * @return false : 	Term does not exist in the dictionary.
     */
    bool getTermIdByTermString(const TermType& termString, IDType& termId)
    {
        return termIdManager_.getTermIdByTermString(termString, termId);
    }

    /**
     * @brief a member function to add a term to be candidate of wildcard queries
     */
    bool addWildcardCandidate(const TermType& termString)
    {
        wildcardQueryManager_.insert(termString);
        return true;
    }

    /**
     * @brief a member function to get a set of term ID list which matches to the given term strings respectively.
     *
     * @param termStringList	a list of term strings.
     * @param termIdList	    a list of term IDs.
     * @return true  :		One or more terms exist in the dictionary.
     * @return false :		No term exists in the dictionary.
     */
    bool getTermIdListByTermStringList(
            const std::vector<TermType>& termStringList,
            std::vector<IDType>& termIdList)
    {
        return termIdManager_.getTermIdListByTermStringList(termStringList, termIdList);
    }

    /**
     * @brief a member function to add a list of terms to be candidates of wildcard queries.
     */
    bool addWildcardCandidateList(const std::vector<TermType>& termStringList)
    {
        for(size_t i =0; i< termStringList.size(); i++ ) {
            wildcardQueryManager_.insert(termStringList[i]);
        }
        return true;
    }

    /**
     * @brief a member function to offer a result set of wildcard search with WildcardSearchManager.
     *
     * @param wildcardPattern   a UString of wildcard pattern which contains '*';
     * @param termIdList        a list of term IDs which is the result of WildcardSearchManager.
     * @return true  :          Given wildcard pattern is matched at least once in the dictionary.
     * @return false :          Given wildcard pattern is not matched in the dictionary.
     */
    bool getTermIdListByWildcardPattern(
            const TermType& wildcardPattern,
            std::vector<IDType>& termIdList,
            int maximumResultNumber = 5)
    {
        return wildcardQueryManager_.findRegExp(wildcardPattern, termIdList, maximumResultNumber);
    }

    /**
     * @brief a member function to offer a result set of wildcard search with WildcardSearchManager.
     *
     * @param wildcardPattern   a UString of wildcard pattern which contains '*';
     * @param termList        a list of terms which is the result of WildcardSearchManager.
     * @return true  :          Given wildcard pattern is matched at least once in the dictionary.
     * @return false :          Given wildcard pattern is not matched in the dictionary.
     */
    bool getTermListByWildcardPattern(
            const TermType& wildcardPattern,
            std::vector<TermType>& termList,
            int maximumResultNumber = 5)
    {
        return wildcardQueryManager_.findRegExp(wildcardPattern, termList, maximumResultNumber);
    }


    /**
     * @brief a member function to get term string by its ID.
     *
     * @param termId	    a term identifier for the input.
     * @param termString	a UString object which contains term string.
     * @return true  :  Given term string exists in the dictionary.
     * @return false :  Given term string does not exist in the dictionary.
     */
    bool getTermStringByTermId(const IDType& termId, TermType& termString)
    {
        return termIdManager_.getTermStringByTermId(termId, termString);
    }

    /**
     * @brief a member function to term string list by a set of term IDs.
     *
     * @param termIdList	    a list of term IDs.
     * @param termStringList	a list of term strings.
     * @return true  :      At least one term in the given list is matched in the dictionary.
     * @return false :      No term is matched in the dictionary.
     */
    bool getTermStringListByTermIdList(
            const std::vector<IDType>& termIdList,
            std::vector<TermType>& termStringList)
    {
        return termIdManager_.getTermStringListByTermIdList(termIdList, termStringList);
    }

    /**
     * @brief a member function to get document ID from the vocabulary which matches to the given document name.
     *
     * @param docName		a unique string of the document which is used to distinguish between documents.
     * @param docId    	    a document identifier which matches to the document name.
     * @param insert         whether insert docName to IDManager;
     * @return true  : 	    Document name exists in the dictionary.
     * @return false : 	    Document name does not exist in the dictionary.
     */
    bool getDocIdByDocName(const DocType& docName, IDType& docId, bool insert = true)
    {
        return docIdManager_.getDocIdByDocName(docName, docId, insert);
    }

    /**
     * @brief a member function to set the document ID to the new value so that it can satisfy the incremental document ID semantic.
     *
     * @param docName		a unique string of the document which is used to distinguish between documents.
     * @param updatedId  the new old old document identifier which matches to the document name.
     */
    void updateDocIdByDocName(const DocType& docName, IDType& updatedId)
    {
        docIdManager_.updateDocIdByDocName(docName, updatedId);
    }

    /**
     * @brief a member function to get a name of the document by its ID.
     *
     * @param docId		    a document identifier.
     * @param docName	   	a unique string of the document which matches to the document ID.
     * @return true  : 	    Given document name exists in the dictionary.
     * @return false : 	    Given document name does not exist in the dictionary.
     */
    bool getDocNameByDocId(const IDType& docId, DocType& docName)
    {
        return docIdManager_.getDocNameByDocId(docId, docName);
    }

    /**
    * @brief Get the maximum doc id.
    * @return max doc id, 0 for no doc id exists or this function is not supported
    */
    IDType getMaxDocId() const
    {
        return docIdManager_.getMaxDocId();
    }

    /**
     * @brief Start a standalone thread to insert all cached terms and ids into RegexpManager.
     */
    void startWildcardProcess(const int threadNumber = 1)
    {
        wildcardQueryManager_.startThread(threadNumber);
    }

    /**
     * @brief Wait for thread initialized in startWildcardProcess() stop.
     */
    void joinWildcardProcess()
    {
        wildcardQueryManager_.joinThread();
    }

    /**
     * @brief Flush all cached data to disk
     */
    void flush()
    {
        termIdManager_.flush();
        docIdManager_.flush();
        wildcardQueryManager_.flush();
    }

    /**
     * @brief Close all resources.
     */
    void close()
    {
        termIdManager_.close();
        docIdManager_.close();
        wildcardQueryManager_.close();
    }

    /**
     * @brief retrieve version string of id-manager
     * @return version string of id-manager
     */
    const std::string& getVersionString() const {
        return version_;
    }

    void display(){
        termIdManager_.display();
        docIdManager_.display();
    }

private:
    TermIdManager<TermType, IDType, TermIDGenerator, TermIDStorage> termIdManager_;

    DocIdManager<DocType, IDType, DocIDGenerator, DocIDStorage> docIdManager_;

    WildcardQueryManager<TermType, IDType, WildcardQueryHandler, LockType> wildcardQueryManager_;

    std::string version_;
};

/*****************************************************************************
 *                      Public Interfaces For Users With Different Requirements
 *****************************************************************************/

/**
 * VERY IMPORTANT NOTE!!!
 *
 * IDManagerDebug means, getTermStringByTermId() and getTermStringListByTermIdList()
 * interface are available, this is especially helpful for those poor guys to convert
 * the meaningless IDs back to strings which is easy to understand for human beings
 * when debug their codes. However, the cost is, an additional SDB should be used
 * for saving all TermID->Term mappings, introducing both computation and storage cost.
 *
 * Some one may be not comfortable to the cost of the above feature, so I also prepare
 * the IDManagerRelease interfaces for these brave guys, in which getTermStringByTermId()
 * and getTermStringListByTermIdList() are disabled, and TermID->Term mappings are simply
 * neglected and lost.
 *
 * You may also want another intermediate solution, e.g. just record TermID->Term pairs
 * in a disk file, process it elsewhere later. It's easy to implement so in current
 * template-based code, have a look at SDBIDStorage and EmptyIDStorage first, write
 * some IDStorage policy, say, LogIDStorage and pass it into _IDManager, then typedef your
 * IDManagerLog here, enjoy it!
 */

/**
 * This version of IDManager supports getTermStringByTermId() and
 * getTermStringListByTermIdList() interface, generated ID are 32bits unsigned integer.
 */
typedef _IDManager<izenelib::util::UString, izenelib::util::UString, uint32_t> IDManagerDebug32;

/**
 * This version of IDManager doesn't support getTermStringByTermId() and
 * getTermStringListByTermIdList() interface, generated ID are 32bits unsigned integer.
 */
/*
typedef _IDManager<izenelib::util::UString, uint32_t,
                   izenelib::util::NullLock,
                   EmptyWildcardQueryHandler<izenelib::util::UString, uint32_t>,
                   HashIDGenerator<izenelib::util::UString, uint32_t>,
                   EmptyIDStorage<izenelib::util::UString, uint32_t>,
                   UniqueIDGenerator<izenelib::util::UString, uint32_t>,
                   SDBIDStorage<izenelib::util::UString, uint32_t> > IDManagerRelease32;*/

typedef _IDManager<izenelib::util::UString, izenelib::util::UString, uint32_t,
                   izenelib::util::ReadWriteLock,
                   EmptyWildcardQueryHandler<izenelib::util::UString, uint32_t>,
                   HashIDGenerator<izenelib::util::UString, uint32_t>,
                   EmptyIDStorage<izenelib::util::UString, uint32_t>,
                   UniqueIDGenerator<izenelib::util::UString, uint32_t>,
                   EmptyIDStorage<izenelib::util::UString, uint32_t> > IDManagerRelease32;

/**
 * This version of IDManager supports getTermStringByTermId() and
 * getTermStringListByTermIdList() interface, generated ID are 64bits unsigned integer.
 */
typedef _IDManager<izenelib::util::UString, izenelib::util::UString, uint64_t> IDManagerDebug64;

/**
 * This version of IDManager doesn't support getTermStringByTermId() and
 * getTermStringListByTermIdList() interface, generated ID are 64bits unsigned integer.
 */
typedef _IDManager<izenelib::util::UString, izenelib::util::UString, uint64_t,
                   izenelib::util::NullLock,
                   EmptyWildcardQueryHandler<izenelib::util::UString, uint64_t>,
                   HashIDGenerator<izenelib::util::UString, uint64_t>,
                   EmptyIDStorage<izenelib::util::UString, uint64_t>,
                   UniqueIDGenerator<izenelib::util::UString, uint64_t>,
                   HDBIDStorage<izenelib::util::UString, uint64_t> > IDManagerRelease64;

/**
 * This version of IDManager is provided for I-classifier, which requires TermID to be
 * uique for different terms, besides it doesn't need generate doc id.
 * Besides, IClassifier requires multi-threads protection.
 */
//typedef _IDManager<izenelib::util::UString, uint32_t,
//                   izenelib::util::NullLock,
//                   EmptyWildcardQueryHandler<izenelib::util::UString, uint32_t>,
//                   UniqueIDGenerator<izenelib::util::UString, uint32_t, izenelib::util::ReadWriteLock>,
//                   HDBIDStorage<izenelib::util::UString, uint32_t, izenelib::util::ReadWriteLock>,
//                   EmptyIDGenerator<izenelib::util::UString, uint32_t>,
//                   EmptyIDStorage<izenelib::util::UString, uint32_t> > IDManagerIClassifier;

typedef _IDManager<izenelib::util::UString, izenelib::util::UString, uint32_t,
                   izenelib::util::ReadWriteLock,
                   EmptyWildcardQueryHandler<izenelib::util::UString, uint32_t>,
                   HashIDGenerator<izenelib::util::UString, uint32_t>,
                   EmptyIDStorage<izenelib::util::UString, uint32_t>,
                   EmptyIDGenerator<izenelib::util::UString, uint32_t>,
                   EmptyIDStorage<izenelib::util::UString, uint32_t> > IDManagerIClassifier;

/**
 * This version of IDManager is provided for ESA, which requires TermId and DocId,
 * it dose not generate any file.
 */
typedef _IDManager<izenelib::util::UString, izenelib::util::UString, uint32_t,
                   izenelib::util::NullLock,
                   EmptyWildcardQueryHandler<izenelib::util::UString, uint32_t>,
                   HashIDGenerator<izenelib::util::UString, uint32_t>,
                   EmptyIDStorage<izenelib::util::UString, uint32_t>,
                   UniqueIDGenerator<izenelib::util::UString, uint32_t>,
                   EmptyIDStorage<izenelib::util::UString, uint32_t> > IDManagerESA;

/**
 * This version of IDManager is provided for MIA, which only wants TermID genrated by
 * hash and doesn't permit generating any file.
 */
typedef _IDManager<izenelib::util::UString, izenelib::util::UString, uint32_t,
                   izenelib::util::NullLock,
                   EmptyWildcardQueryHandler<izenelib::util::UString, uint32_t>,
                   HashIDGenerator<izenelib::util::UString, uint32_t>,
                   HDBIDStorage<izenelib::util::UString, uint32_t>,
                   EmptyIDGenerator<izenelib::util::UString, uint32_t>,
                   EmptyIDStorage<izenelib::util::UString, uint32_t> > IDManagerMIA;

typedef _IDManager<izenelib::util::UString, uint128_t, uint32_t,
                   izenelib::util::ReadWriteLock,
                   EmptyWildcardQueryHandler<izenelib::util::UString, uint32_t>,
                   HashIDGenerator<izenelib::util::UString, uint32_t>,
                   EmptyIDStorage<izenelib::util::UString, uint32_t>,
                   UniqueIDGenerator<uint128_t, uint32_t>,
                   EmptyIDStorage<uint128_t, uint32_t> > IDManager;

typedef _IDManager<izenelib::util::UString, uint128_t, uint32_t,
                   izenelib::util::ReadWriteLock,
                   EmptyWildcardQueryHandler<izenelib::util::UString, uint32_t>,
                   HashIDGenerator<izenelib::util::UString, uint32_t>,
                   EmptyIDStorage<izenelib::util::UString, uint32_t>,
                   OldUniqueIDGenerator<uint128_t, uint32_t>,
                   EmptyIDStorage<uint128_t, uint32_t> > OldIDManager;

typedef _IDManager<izenelib::util::UString, izenelib::util::UString, uint64_t,
                   izenelib::util::NullLock,
                   EmptyWildcardQueryHandler<izenelib::util::UString, uint64_t>,
                   HashIDGenerator<izenelib::util::UString, uint64_t>,
                   EmptyIDStorage<izenelib::util::UString, uint64_t>,
                   UniqueIDGenerator<izenelib::util::UString, uint64_t>,
                   HDBIDStorage<izenelib::util::UString, uint64_t> > AutoFillIDManager_old;

typedef _IDManager<std::string, std::string, uint32_t,
                   izenelib::util::NullLock,
                   EmptyWildcardQueryHandler<std::string, uint32_t>,
                   HashIDGenerator<std::string, uint32_t>,
                   EmptyIDStorage<std::string, uint32_t>,
                   UniqueIDGenerator<std::string, uint32_t>,
                   Lux::IO::Array> AutoFillIDManager;

} // end - namespace idmanager

NS_IZENELIB_IR_END

#endif // _ID_MANAGER_
