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
  * There are two kinds of regexp handlers selectable:
  * - Empty : if selected, nothing will RegexpManager do.
  * - Disk version: based on SDBTrie, data is swapped between memory and disk.
  */
template<typename NameString, typename NameID>
class BasicRegExpHandler
{
public:

	void open() {}

	void optimize(){}

	void flush(){}

	void close(){}

	void executeTask(int threadNum) {}

	void insert(const NameString& str) {}

	bool findRegExp(const NameString& exp, std::vector<NameID> & results){ return false;}

	int num_items(){return 0;}

	void display(){}
};

/**
 *@brief nothing will be done for any operation.
 */
template<typename NameString, typename NameID>
class EmptyRegExpHandler : public BasicRegExpHandler<NameString, NameID>
{
public:

	EmptyRegExpHandler(const std::string&){}

	void display(){std::cout << "This is a EmptyRegExpHandler instance" << std::endl; }

}; // end - class EmptyRegExpHandler

/**
 *@brief based on SDBTrie, data is swapped between memory and disk.
 */
template<typename NameString, typename NameID>
class DiskRegExpHandler : public BasicRegExpHandler<NameString, NameID>
{
public:
	DiskRegExpHandler(const std::string& name, const int partitionNum = 64)
	:   trie_(name, partitionNum) {}

	void open() { trie_.open(); }

	void optimize(){ trie_.optimize(); }

	void flush(){ trie_.flush(); }

	void close(){ trie_.close(); }

	void insert(const NameString& str) { trie_.insert(str); }

    void executeTask(int threadNum) {
        trie_.executeTask(threadNum);
    }

	bool findRegExp(const NameString& exp, std::vector<NameID> & results){
	    std::vector<NameString> rlist;
	    if(trie_.findRegExp(exp, rlist) == false)
            return false;
        for(size_t i =0; i< rlist.size(); i++)
            results.push_back( NameIDTraits<NameID>::hash(rlist[i]) );
        return true;
    }

	int num_items(){return trie_.num_items();}

	void display(){std::cout << "This is a DiskRegExpHandler instance" << std::endl; trie_.display(); }

private:

    MtTrie<NameString> trie_;
}; // end - class DiskRegExpHandler


/**
 * @brief A meta function to test if given class is an instantiation of template EmptyRegExpHandler.
 * @return true it's a EmptyRegExpHandler
 *         false otherwise
 */
template <typename RegExpHandler>
class IsEmpty {
public:
    static const bool value = false;
};
template <typename N,typename I>
class IsEmpty<EmptyRegExpHandler<N,I> > {
public:
    static const bool value = true;
};

/**
 * @brief Manager to handler regexp searches.
 */
template<typename NameString,
         typename NameID,
         typename RegExpHandler,
         typename LockType>
class RegexpManager
{
    typedef typename NameString::value_type CharType;

public:

    RegexpManager(const std::string& storageName)
        : storageName_(storageName),
          handler_(storageName), worker_(NULL)
    {
        handler_.open();
    }

    ~RegexpManager()
    {
        close();
    }

	void insert(const NameString & word, const NameID & id)
	{
        handler_.insert(word);
    }

	bool findRegExp(const NameString& exp, std::vector<NameID> & results)
	{
        return handler_.findRegExp(exp, results);
    }

    void startThread(const unsigned int cacheSize)
    {
        // here if statement could be optimized at compile time,
        // so it doesn't effect the perforamnce.
        if( ! IsEmpty<RegExpHandler>::value )
        {
            if(worker_) return;
            worker_ = new boost::thread(boost::bind(&RegExpHandler::executeTask, &handler_, 2));
        }
    }

    void joinThread()
    {
        // here if statement could be optimized at compile time,
        // so it doesn't effect the perforamnce.
        if( ! IsEmpty<RegExpHandler>::value )
        {
            if(worker_==NULL) return;

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

        // here if statement could be optimized at compile time,
        // so it doesn't effect the perforamnce.
        if( ! IsEmpty<RegExpHandler>::value )
        {
            if(worker_) {
                worker_->join();
                delete worker_;
                worker_ = NULL;
            }
        }
    }

private:

    std::string storageName_;

    LockType filelock_;

    RegExpHandler handler_;

    boost::thread* worker_;
};

}

NS_IZENELIB_IR_END

#endif
