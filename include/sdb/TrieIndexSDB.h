#ifndef TRIEINDEXSDB_H_
#define TRIEINDEXSDB_H_

#include "IndexSDB.h"
#include <am/hdb_trie/hdb_trie.hpp>
#include <util/hashFunction.h>
using namespace std;

namespace izenelib {
namespace sdb {

template <typename StringType, typename ElementType,
		typename LockType =izenelib::util::NullLock> class TrieIndexSDB {
public:
	typedef uint32_t KeyType;
	typedef typename StringType::value_type CharType;
	typedef iKeyType<KeyType> myKeyType;
	typedef vector<ElementType> myValueType;
	typedef DataType<myKeyType, myValueType> myDataType;

	typedef IndexSDB<uint32_t, ElementType, LockType> IndexSDBType;

	typedef izenelib::am::HDBTrie2<StringType, uint32_t, uint64_t, LockType>
			TrieType;
public:
	TrieIndexSDB(const string& fileName = "trie_index") :
		indexsdb_(fileName+".isdb"), triedb_(fileName + ".triedb") {
	}
	bool open() {
		indexsdb_.initialize(100, 12, 1024, 200*1024);
		triedb_.open();
		return true;
	}

	void getValuePrefix(const StringType& key, vector<ElementType>& result) {
		vector<uint32_t> hooks;
		triedb_.findPrefix(key, hooks);
		indexsdb_.getValueIn(hooks, result);
	}

	void getValueSuffix(const StringType& key, vector<ElementType>& result) {
		uint32_t hashval =
				(int32_t)HashFunction<StringType>::generateHash32(key);
		indexsdb_.getValue(hashval, result);
	}

	bool add_suffix(const StringType& key, const ElementType& item) {
		uint32_t t;
		if ( !triedb_.get(key, t) ) {
			unsigned int pos = 0;
			for (; pos<key.size(); pos++) {
				StringType suf = key.substr(pos);
				add(suf, item);
			}
		}
		return false;
	}

	bool add(const StringType& key, const ElementType& item) {
		uint32_t hashval =
				(int32_t)HashFunction<StringType>::generateHash32(key);
		triedb_.insert(key, hashval);
		indexsdb_.add_nodup(hashval, item);
		return false;
	}
	void display() {
		indexsdb_.display();
	}

	void commit() {

	}

	void flush() {
		indexsdb_.flush();
		triedb_.flush();
	}
private:
	IndexSDBType indexsdb_;
	TrieType triedb_;
};

template <typename StringType, typename ElementType,
		typename LockType =izenelib::util::NullLock> class TrieIndexSDB2 {
	typedef SequentialDB<std::pair<StringType, ElementType>, NullType> SDBTYPE;
	typedef typename SDBTYPE::SDBCursor SDBCursor;
public:
	TrieIndexSDB2(const string& fileName = "trie_index2.dat") :
		sdb_(fileName) {

	}
	bool open() {
		return sdb_.open();
	}

	void getValuePrefix(const StringType& key, vector<ElementType>& result) {
		SDBCursor locn = sdb_.search(make_pair(key, ElementType() ) );
		std::pair<StringType, ElementType> skey;
		StringType lstr;
		NullType sval;
		while (sdb_.get(locn, skey, sval) ) {
			if (isPrefix1(key, skey.first) ) {
				//cout<<skey.first<<"+"<<skey.second<<endl;
				result.push_back(skey.second);
				sdb_.seq(locn);
			} else
				break;
		}

	}

	void getValueSuffix(const StringType& key, vector<ElementType>& result) {
		SDBCursor locn = sdb_.search(make_pair(key, ElementType() ) );
		std::pair<StringType, ElementType> skey;
		StringType lstr;
		NullType sval;
		while (sdb_.get(locn, skey, sval) ) {
			if (key == skey.first) {
				//cout<<skey.first<<"+"<<skey.second<<endl;
				result.push_back(skey.second);
				sdb_.seq(locn);
			} else
				break;
		}

	}

	bool add_suffix(const StringType& key, const ElementType& item) {
		if (sdb_.hasKey(make_pair(key, key) ) )
			return false;
		unsigned int pos = 0;
		for (; pos<key.length(); pos++) {
			StringType suf = key.substr(pos);
			add(suf, item);
		}
		return false;
	}

	bool add(const StringType& key, const ElementType& item) {
		sdb_.insertValue(make_pair(key, item) );
		return false;
	}

	void display() {
		sdb_.display();
	}

	void commit() {
		sdb_.commit();
	}

	void flush() {
		sdb_.flush();
	}
private:
	SDBTYPE sdb_;

};

}
}
#endif /*TRIEINDEXSDB_H_*/
