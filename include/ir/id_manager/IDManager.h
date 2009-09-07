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
 */

#ifndef _ID_MANAGER_
#define _ID_MANAGER_

#include <types.h>

#include <wiselib/ustring/UString.h>

#include "RegexpManager.h"
#include "DocIdManager.h"
#include "TermIdManager.h"
#include "IDGenerator.h"
#include "IDStorage.h"

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

#define MAJOR_VERSION "1"
#define MINOR_VERSION "0"
#define PATCH_VERSION "20081203"

/**
 * @brief Combination of both TermIdManager, DocIdManager and RegexpManager.
 */

template<typename NameString,
         typename NameID,
         typename LockType = izenelib::util::NullLock,
         typename RegExpHandler = DiskRegExpHandler<NameString, NameID>,
         typename TermIDGenerator = HashIDGenerator<NameString, NameID>,
         typename TermIDStorage = SDBIDStorage<NameString, NameID, LockType>,
         typename DocIDGenerator = UniqueIDGenerator<NameString, NameID, LockType>,
         typename DocIDStorage = SDBIDStorage<NameString, NameID, LockType> >
class _IDManager
{

public:
	_IDManager(const string& storageName = "idm")
	:
		termIdManager_(storageName + "_tid"),
		docIdManager_(storageName + "_did"),
		regexpManager_(storageName + "_regexp")
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
	bool getTermIdByTermString(const NameString& termString, NameID& termId)
    {
        return termIdManager_.getTermIdByTermString(termString, termId, regexpManager_);
    }

	/**
	 * @brief a member function to get a set of term ID list which matches to the given term strings respectively.
	 *
	 * @param termStringList	a list of term strings.
	 * @param termIdList	    a list of term IDs.
	 * @return true  :		One or more terms exist in the dictionary.
	 * @return false :		No term exists in the dictionary.
	 */
	bool getTermIdListByTermStringList( const std::vector<NameString>& termStringList,
			std::vector<NameID>& termIdList)
    {
        return termIdManager_.getTermIdListByTermStringList(termStringList, termIdList, regexpManager_);
    }

	/**
	 * @brief a member function to offer a result set of wildcard search with WildcardSearchManager.
	 *
	 * @param wildcardPattern   a UString of wildcard pattern which contains '*';
	 * @param termIdList        a list of term IDs which is the result of WildcardSearchManager.
	 * @return true  :          Given wildcard pattern is matched at least once in the dictionary.
	 * @return false :          Given wildcard pattern is not matched in the dictionary.
	 */
	bool getTermIdListByWildcardPattern(const NameString& wildcardPattern,
			std::vector<NameID>& termIdList)
    {
        return regexpManager_.findRegExp(wildcardPattern, termIdList);
    }

	/**
	 * @brief a member function to get term string by its ID.
	 *
	 * @param termId	    a term identifier for the input.
	 * @param termString	a UString object which contains term string.
	 * @return true  :  Given term string exists in the dictionary.
	 * @return false :  Given term string does not exist in the dictionary.
	 */
	bool getTermStringByTermId(const NameID& termId, NameString& termString)
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
	bool getTermStringListByTermIdList(const std::vector<NameID>& termIdList,
			std::vector<NameString>& termStringList)
    {
        return termIdManager_.getTermStringListByTermIdList(termIdList, termStringList);
    }

	/**
	 * @brief a member function to get document ID from the vocabulary which matches to the given document name.
	 *
	 * @param docName		a unique string of the document which is used to distinguish between documents.
	 * @param docId    	    a document identifier which matches to the document name.
	 * @return true  : 	    Document name exists in the dictionary.
	 * @return false : 	    Document name does not exist in the dictionary.
	 */
	bool getDocIdByDocName(const NameString& docName, NameID& docId)
    {
        return docIdManager_.getDocIdByDocName(docName, docId);
    }

	/**
	 * @brief a member function to get a name of the document by its ID.
	 *
	 * @param docId		    a document identifier.
	 * @param docName	   	a unique string of the document which matches to the document ID.
	 * @return true  : 	    Given document name exists in the dictionary.
	 * @return false : 	    Given document name does not exist in the dictionary.
	 */
	bool getDocNameByDocId(const NameID& docId, NameString& docName)
	{
        return docIdManager_.getDocNameByDocId(docId, docName);
    }

    /**
     * @brief Start a standalone thread to insert all cached terms and ids into RegexpManager.
     */
    void startWildcardProcess()
    {
        regexpManager_.startThread(0);
    }

    /**
     * @brief Wait for thread initialized in startWildcardProcess() stop.
     */
    void joinWildcardProcess()
    {
        regexpManager_.joinThread();
    }

    /**
     * @brief Close all resources.
     */
    void close()
    {
        termIdManager_.close();
        docIdManager_.close();
        regexpManager_.close();
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

	TermIdManager<NameString, NameID, TermIDGenerator, TermIDStorage> termIdManager_;

	DocIdManager<NameString, NameID, DocIDGenerator, DocIDStorage> docIdManager_;

    RegexpManager<NameString, NameID, RegExpHandler, LockType> regexpManager_;

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
typedef _IDManager<wiselib::UString, uint32_t> IDManagerDebug32;

/**
 * This version of IDManager doesn't support getTermStringByTermId() and
 * getTermStringListByTermIdList() interface, generated ID are 32bits unsigned integer.
 */
typedef _IDManager<wiselib::UString, uint32_t,
                   izenelib::util::NullLock,
                   DiskRegExpHandler<wiselib::UString, uint32_t>,
                   HashIDGenerator<wiselib::UString, uint32_t>,
                   EmptyIDStorage<wiselib::UString, uint32_t>,
                   UniqueIDGenerator<wiselib::UString, uint32_t>,
                   SDBIDStorage<wiselib::UString, uint32_t> > IDManagerRelease32;

/**
 * This version of IDManager supports getTermStringByTermId() and
 * getTermStringListByTermIdList() interface, generated ID are 64bits unsigned integer.
 */
typedef _IDManager<wiselib::UString, uint64_t> IDManagerDebug64;

/**
 * This version of IDManager doesn't support getTermStringByTermId() and
 * getTermStringListByTermIdList() interface, generated ID are 64bits unsigned integer.
 */
typedef _IDManager<wiselib::UString, uint64_t,
                   izenelib::util::NullLock,
                   DiskRegExpHandler<wiselib::UString, uint64_t>,
                   HashIDGenerator<wiselib::UString, uint64_t>,
                   EmptyIDStorage<wiselib::UString, uint64_t>,
                   UniqueIDGenerator<wiselib::UString, uint64_t>,
                   SDBIDStorage<wiselib::UString, uint64_t> > IDManagerRelease64;

/**
 * This version of IDManager is provided for I-classifier, which requires TermID to be
 * uique for different terms, besides it doesn't need generate doc id.
 * Besides, IClassifier requires multi-threads protection.
 */
typedef _IDManager<wiselib::UString, uint32_t,
                   izenelib::util::NullLock,
                   EmptyRegExpHandler<wiselib::UString, uint32_t>,
                   UniqueIDGenerator<wiselib::UString, uint32_t, izenelib::util::ReadWriteLock>,
                   SDBIDStorage<wiselib::UString, uint32_t, izenelib::util::ReadWriteLock>,
                   EmptyIDGenerator<wiselib::UString, uint32_t>,
                   EmptyIDStorage<wiselib::UString, uint32_t> > IDManagerIClassifier;

/**
 * This version of IDManager is provided for MIA, which only wants TermID genrated by
 * hash and doesn't permit generating any file.
 */
typedef _IDManager<wiselib::UString, uint32_t,
                   izenelib::util::NullLock,
                   EmptyRegExpHandler<wiselib::UString, uint32_t>,
                   HashIDGenerator<wiselib::UString, uint32_t>,
                   EmptyIDStorage<wiselib::UString, uint32_t>,
                   EmptyIDGenerator<wiselib::UString, uint32_t>,
                   EmptyIDStorage<wiselib::UString, uint32_t> > IDManagerMIA;
/**
 * The default IDManager is IDManagerDebug32, If you want to use a different version,
 * write code like following:

    #include <ir/id_manager/IDManager.h>

    #define REPLACE_DEFAULT_IDMANAGER
    typedef IDManagerRelease64 IDManager;

 */
#ifndef REPLACE_DEFAULT_IDMANAGER
typedef IDManagerDebug32 IDManager;
#endif

} // end - namespace idmanager

NS_IZENELIB_IR_END

#endif // _ID_MANAGER_
