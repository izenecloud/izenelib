/**
 * @author Wei Cao
 * @date 2009-09-05
 */

#ifndef _REGEXP_MANAGER_
#define _REGEXP_MANAGER_

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <types.h>

#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "NameIDTraits.h"
#include <am/mt_trie/mt_trie.hpp>

NS_IZENELIB_IR_BEGIN

namespace idmanager {

/**
  * There are two kinds of wildcard query handlers selectable:
  * - Empty : if selected, nothing will RegexpManager do.
  * - Disk version: based on SDBTrie, data is swapped between memory and disk.
  */
template<typename NameString, typename NameID>
class BasicWildcardQueryHandler
{
public:

	void open() {}

	void optimize(){}

	void flush(){}

	void close(){}

	void executeTask(int threadNum) {}

	void insert(const NameString& str) {}

	bool findRegExp(const NameString& exp, std::vector<NameString> & results, int maximumResultNumber){ return false;}

	int num_items(){return 0;}

	void display(){}
};

/**
 *@brief nothing will be done for any operation.
 */
template<typename NameString, typename NameID>
class EmptyWildcardQueryHandler : public BasicWildcardQueryHandler<NameString, NameID>
{
public:

	EmptyWildcardQueryHandler(const std::string&){}

	void display(){std::cout << "This is a EmptyWildcardQueryHandler instance" << std::endl; }

};

/**
 *@brief based on MtTrie, data is swapped between memory and disk.
 */
template<typename NameString, typename NameID>
class DiskWildcardQueryHandler : public BasicWildcardQueryHandler<NameString, NameID>
{
public:
	DiskWildcardQueryHandler(const std::string& name, const int partitionNum = 64)
	:   trie_(name, partitionNum) {}

	void open() { trie_.open(); }

	void optimize(){ trie_.optimize(); }

	void flush(){ trie_.flush(); }

	void close(){ trie_.close(); }

	void insert(const NameString& str) { trie_.insert(str); }

    void executeTask(int threadNum) {
        trie_.executeTask(threadNum);
    }

	bool findRegExp(const NameString& exp, std::vector<NameString> & results, int maximumResultNumber){
        return trie_.findRegExp(exp, results, maximumResultNumber);
    }

	int num_items(){return trie_.num_items();}

	void display(){std::cout << "This is a DiskWildcardQueryHandler instance" << std::endl; trie_.display(); }

private:

    MtTrie<NameString> trie_;
};


/**
 * @brief Manager to handler regexp searches.
 */
template<typename NameString,
         typename NameID,
         typename WildcardQueryHandler,
         typename LockType>
class WildcardQueryManager
{
    typedef typename NameString::value_type CharType;

public:

    WildcardQueryManager(const std::string& storageName)
        : storageName_(storageName),
          handler_(storageName), worker_(NULL)
    {
        handler_.open();
    }

    ~WildcardQueryManager()
    {
        close();
    }

	void insert(const NameString & word)
	{
        handler_.insert(word);
    }

	bool findRegExp(const NameString& exp, std::vector<NameString> & results, int maximumResultNumber)
	{
	    return handler_.findRegExp(exp, results, maximumResultNumber);
    }

	bool findRegExp(const NameString& exp, std::vector<NameID> & results, int maximumResultNumber)
	{
	    std::vector<NameString> rlist;
	    if(handler_.findRegExp(exp, rlist, maximumResultNumber) == false)
            return false;

        for(size_t i =0; i< rlist.size(); i++) {
            results.push_back( NameIDTraits<NameID>::hash(rlist[i]) );
        }
        return true;
    }

    void startThread(const int threadNumber = 1)
    {
        if( !worker_ ) {
            worker_ = new boost::thread(boost::bind(&WildcardQueryHandler::executeTask,
                &handler_, threadNumber));
        }
    }

    void joinThread()
    {
        if(worker_) {
            worker_->join();
            delete worker_;
            worker_ = NULL;
        }
    }

    void flush()
    {
        handler_.flush();
    }

    void close()
    {
        handler_.close();

        if(worker_) {
            worker_->join();
            delete worker_;
            worker_ = NULL;
        }
    }

private:

    std::string storageName_;

    WildcardQueryHandler handler_;

    boost::thread* worker_;
};

}

NS_IZENELIB_IR_END

#endif
