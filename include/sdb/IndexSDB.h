/**
 * @file IndexSDB.h
 * @brief The header file of IndexSDB.
 * @author Peisheng Wang
 *
 * This file defines clas IndexSDB.
 * 
 * @history
 *  - 11.27 Peisheng Wnag
 *  - refactor to make it fater
 * 
 * */

#ifndef INDEXSDB_H_
#define INDEXSDB_H_

#include <algorithm>
#include <wiselib/ustring/UString.h>

#include "SequentialDB.h"

namespace izenelib {

namespace sdb {

template<typename KeyType> inline bool isPrefix1(const KeyType& sub,
		const KeyType& str) {
	return sub.isPrefix(str);
}

template<> inline bool isPrefix1<std::string>(const string& sub,
		const string& str) {
	return (str.substr(0, sub.length() ) == sub );
}

template<> inline bool isPrefix1<wiselib::UString>(const wiselib::UString& sub,
		const wiselib::UString& str) {
	return (str.substr(0, sub.length() ) == sub );
}

/**
 *  \brief wrapper KeyType for indexSDB, 
 * 
 *  An extra member offset is added, 
 *  A key of KeyType is presented by serveral sequential iKeyTypes 
 *  in IndexSDB.  
 *  
 * 
 */
template<class KeyType> struct iKeyType {
	friend class boost::serialization::access;
	KeyType key;
	unsigned int offset;

	iKeyType() {
		offset = -1;
	}

	iKeyType(KeyType key1, unsigned int offset1) {
		key = key1;
		offset = offset1;
	}

	template<class Archive> void serialize(Archive & ar,
			const unsigned int version) {
		ar & key;
		ar & offset;
	}

	int compare(const iKeyType& other) const {
		if (comp_(key, other.key) != 0) {
			return comp_(key, other.key);
		} else {
			return offset - other.offset;
		}
	}

	bool isPrefix(const iKeyType<KeyType>& other) const {
		return isPrefix1(key, other.key);
	}

	void display(std::ostream& os = std::cout) const {
		key.display(os);
		//os<<key;
		os<<" offset= "<<offset;
	}
	izenelib::am::CompareFunctor<KeyType> comp_;
};
/**
 * 
 *  \brief IndexSDB 
 * 
 *  IndexerManager stores T ermID/vector DocID as Key/V alue pair. Usually
 *  the size of Value can be very small or very large. And it will be of low efficiency
 *  and waste a lot of disk space if we use original SDB by setting very large
 *  page-size to avoid too much overflowing. Instead, we use several sub keys to
 *  represent one key and original value, vector ElementT ype , are scattered to
 *  serveral sequential nodes
 *  
 */
template<class KeyType, class ElementType, class LockType=NullLock> class IndexSDB {
	typedef iKeyType<KeyType> myKeyType;
	typedef std::vector<ElementType> myValueType;
	typedef DataType<myKeyType, myValueType> myDataType;
public:
	typedef SequentialDB<myKeyType, myValueType, LockType> SDB_TYPE;
	typedef typename SDB_TYPE::SDBCursor IndexSDBCursor;

public:
	IndexSDB(const string& fileName = "index_sdb.dat") :
		_sdb(fileName) {
	}
	~IndexSDB() {
		_sdb.close();
	}
	/**
	 *  \brief It must be initialized to run IndexSDB.
	 */
	void initialize(unsigned int vecSize = 100, int degree= 16,
			unsigned int pageSize = 1024, unsigned int cacheSize = 1024*100) {
		_vDataSize = vecSize;
		_sdb.setDegree(degree);
		_sdb.setPageSize(pageSize);
		_sdb.setCacheSize(cacheSize);
		_sdb.open();
	}

	/**
	 *  \brief update 
	 */
	bool update(const KeyType& key, vector<ElementType>& data);

	bool del(const KeyType& key);

	void display(std::ostream& os = std::cout) {
		_sdb.display(os);
	}
	void flush() {
		_sdb.flush();
	}

	void commit() {
		_sdb.commit();
	}

	KeyType getNext(const KeyType& key) {
		myKeyType ikey(key, 0);
		myKeyType temp;
		if (_sdb.hasKey(ikey)) {
			temp = ikey;
		} else {
			temp = _sdb.getNearest(ikey);
			return temp.key;
		}
		KeyType flagkey = temp.key;
		while (temp.key.compare(flagkey) == 0) {
			temp = _sdb.getNext(temp);
		}
		return temp.key;
	}

	KeyType getPrev(const KeyType& key) {
		myKeyType ikey(key, 0);
		myKeyType temp;
		if (_sdb.hasKey(ikey)) {
			temp = ikey;
		} else {
			temp = _sdb.getNearest(ikey);
		}
		temp = _sdb.getPrev(temp);
		return temp.key;
	}

	bool getValue(const KeyType& key, vector<ElementType>& result);

	void getValueBetween(const KeyType& lowKey, const KeyType& highKey,
			vector<ElementType>& result);
	void getValueIn(const vector<KeyType>& vKey, vector<ElementType>& result);

	void getValueGreat(const KeyType& key, vector<ElementType>& result) {
		myKeyType ikey(key, 0);
		myValueType ival;
		IndexSDBCursor locn;
		_sdb.search(ikey, locn);
		while (_sdb.get(locn, ikey, ival) ) {
			if (comp_(ikey.key, key)> 0) {
				for (size_t i=0; i<ival.size(); i++) {
					result.push_back(ival[i]);
				}
			}
			_sdb.seq(locn, ESD_FORWARD);
		}

	}
	void getValueGreatEqual(const KeyType& key, vector<ElementType>& result) {
		myKeyType ikey(key, 0);
		myValueType ival;
		IndexSDBCursor locn;
		_sdb.search(ikey, locn);
		while (_sdb.get(locn, ikey, ival) ) {
			if (comp_(ikey.key, key) >= 0) {
				for (size_t i=0; i<ival.size(); i++) {
					result.push_back(ival[i]);
				}
			}
			_sdb.seq(locn, ESD_FORWARD);
		}
	}

	void getValueLess(const KeyType& key, vector<ElementType>& result) {
		myKeyType ikey(key, 0);
		myValueType ival;
		IndexSDBCursor locn;
		_sdb.search(ikey, locn);
		while (_sdb.get(locn, ikey, ival) ) {
			if (comp_(ikey.key, key) < 0) {
				for (size_t i=0; i<ival.size(); i++) {
					result.push_back(ival[i]);
				}
			}
			_sdb.seq(locn, ESD_BACKWARD);
		}

	}
	void getValueLessEqual(const KeyType& key, vector<ElementType>& result) {
		myKeyType ikey(key, 0);
		myValueType ival;
		IndexSDBCursor locn;
		_sdb.search(ikey, locn);
		while (_sdb.get(locn, ikey, ival) ) {
			if (comp_(ikey.key, key) <= 0) {
				for (size_t i=0; i<ival.size(); i++) {
					result.push_back(ival[i]);
				}
			}
			_sdb.seq(locn, ESD_BACKWARD);
		}
	}

	void getValuePrefix(const KeyType& key, vector<ElementType>& result) {
		myKeyType ikey(key, 0);
		myValueType ival;
		IndexSDBCursor locn;
		_sdb.search(ikey, locn);
		while (_sdb.get(locn, ikey, ival) ) {
			ikey.display();
			if (isPrefix1(key, ikey.key) ) {
				for (size_t i=0; i<ival.size(); i++) {
					//if (find(result.begin(), result.end(), vdat[i]) == result.end() )				
					result.push_back(ival[i]);
				}
				_sdb.seq(locn, ESD_FORWARD);
			} else
				break;
		}
	}

	bool add(const KeyType& key, const ElementType& item) {
		vector<ElementType> vDat;
		getValue(key, vDat);

		if (find(vDat.begin(), vDat.end(), item) == vDat.end() ) {
			vDat.push_back(item);
			return update(key, vDat);
		} else {
			return false;
		}
	}
	
	bool remove(const KeyType& key, const ElementType& item) {
		vector<ElementType> vDat;
		getValue(key, vDat);

		if (find(vDat.begin(), vDat.end(), item) != vDat.end() ) {
			vDat.erase(std::remove(vDat.begin(),vDat.end(), item), vDat.end() ); 
			cout<<"after remove "<<vDat.size()<<endl;
			return update(key, vDat);
		} else {
			return false;
		}
	}

	//if we can make sure there are no duplicate inserting items, e.g, documentID
	//can't be the same as the existing items in IndexSDB, i.e nodup is true. 
	bool add_nodup(const KeyType& key, const ElementType& item) {
		myKeyType ikey(key, 0);
		if ( !_sdb.hasKey(ikey) )
			return add(key, item);
		else {
			myKeyType ukey = _getUpper(key);
			myValueType udat;
			_sdb.getValue(ukey, udat);
			if (udat.size() < _vDataSize) {
				udat.push_back(item);
				_sdb.update(ukey, udat);
			} else {
				vector<ElementType> nv;
				nv.push_back(item);
				myKeyType newkey(key, ukey.offset+1);
				_sdb.insertValue(newkey, nv);
			}
		}
		return true;
	}

protected:
	SDB_TYPE _sdb;
	izenelib::am::CompareFunctor<KeyType> comp_;

	//test;
	LockType lock_;
	unsigned int _vDataSize;

	myKeyType _getUpper(const KeyType& key) {
		myKeyType ikey(key, 0);
		myValueType ival;
		IndexSDBCursor locn;

		_sdb.search(ikey, locn);
		myKeyType rkey = ikey;
		while (_sdb.get(locn, ikey, ival) ) {
			if (comp_(ikey.key, key) == 0) {
				rkey = ikey;
				_sdb.seq(locn, ESD_FORWARD);
			} else
				break;
		}
		return rkey;

	}

};

template<class KeyType, class ElementType, class LockType> bool IndexSDB<
		KeyType, ElementType, LockType>::getValue(const KeyType& key,
		vector<ElementType>& result) {

	myKeyType ikey(key, 0);
	myValueType ival;
	IndexSDBCursor locn;
	_sdb.search(ikey, locn);
	while (_sdb.get(locn, ikey, ival) ) {
		if (comp_(ikey.key, key) == 0) {
			for (size_t i=0; i<ival.size(); i++) {
				result.push_back(ival[i]);
			}
			_sdb.seq(locn, ESD_FORWARD);
		} else
			break;
	}

	return result.size() != 0;
	
}

template<class KeyType, class ElementType, class LockType> void IndexSDB<
		KeyType, ElementType, LockType>::getValueIn(const vector<KeyType>& vKey,
		vector<ElementType>& result) {
	for (size_t i=0; i<vKey.size(); i++) {
		getValue(vKey[i], result);
	}
}

template<class KeyType, class ElementType, class LockType> void IndexSDB<
		KeyType, ElementType, LockType>::getValueBetween(const KeyType& lowKey,
		const KeyType& highKey, vector<ElementType>& result) {

	if (comp_(lowKey, highKey) > 0) {
		return;
	}

	myKeyType ikey(lowKey, 0);
	myValueType ival;
	IndexSDBCursor locn;
	_sdb.search(ikey, locn);
	while (_sdb.get(locn, ikey, ival) ) {
		if (comp_(ikey.key, highKey) <= 0) {
			for (size_t i=0; i<ival.size(); i++) {
				result.push_back(ival[i]);
			}
			_sdb.seq(locn, ESD_FORWARD);
		} else
			break;
	}

}

template<class KeyType, class ElementType, class LockType> bool IndexSDB<
		KeyType, ElementType, LockType>::del(const KeyType& key) {

	unsigned int offset = 0;
	do {
		myKeyType ikey(key, offset++);
		if (_sdb.del(ikey)){}
		else {
			break;
		}
	} while (true);
	return true;

}
template<class KeyType, class ElementType, class LockType> bool IndexSDB<
		KeyType, ElementType, LockType>::update(const KeyType& key,
		vector<ElementType>& data) {

	if (data.size() == 0)//if data is empty, do the deletion.
	{
		cout<<"delete ?"<<endl;
		del(key);
		return false;
	}

	vector<ElementType> origin;
	getValue(key, origin);

	unsigned int sz1 = origin.size();
	unsigned int sz2 = data.size();

	size_t n1 = (sz1 ==0 ) ? 0 : (sz1-1)/_vDataSize+1;
	size_t n2 = (sz2 ==0 ) ? 0 : (sz2-1)/_vDataSize+1;

	if (n1> n2) {
		for (size_t i=n2; i<n1; i++) {
			myKeyType ikey(key, i);
			_sdb.del(ikey);
		}
	}

	vector<ElementType> vDat;
	unsigned int j = 1;
	unsigned int offset = 0;
	for (unsigned int i=0; i<sz2; i++, j++) {
		vDat.push_back(data[i]);
		if (j == _vDataSize || i == sz2-1) {
			myKeyType ikey(key, offset++);
			myDataType idat(ikey, vDat);
			_sdb.update(idat);

			vDat.clear();
			j=0;
		}

	}
	return true;
}

}

}
#endif /*IndexSDB_H_*/
