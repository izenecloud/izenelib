/**
 * @file IndexSDB.h
 * @brief The header file of IndexSDB.
 * @author Peisheng Wang
 *
 * This file defines clas IndexSDB.
 */

#ifndef INDEXSDB_H_
#define INDEXSDB_H_

#include <algorithm>
#include "SequentialDB.h"

namespace izenelib {

namespace sdb {

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
		if (key.compare(other.key) != 0) {
			return key.compare(other.key);
		} else {
			return offset - other.offset;
		}
	}

	bool isPrefix(const iKeyType<KeyType>& other) {
		return key.isPrefix(other.key);
	}

	void display(std::ostream& os = std::cout) const {
		key.display(os);
		os<<" offset= "<<offset;
	}
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
	typedef typename SequentialDB<myKeyType, myValueType, LockType>::SDBCursor
			IndexSDBCursor;

public:
	IndexSDB(const string& fileName = "index_sdb.dat") :
		_sdb(fileName) {
	}
	~IndexSDB() {
		_sdb.close();
	}
	/**
	 *  It must be initialized to run IndexSDB.
	 */
	void initialize(unsigned int vecSize = 100, int degree= 16,
			unsigned int pageSize = 1024, unsigned int cacheSize = 1000000) {
		_vDataSize = vecSize;
		_sdb.setDegree(degree);
		_sdb.setPageSize(pageSize);
		_sdb.setCacheSize(cacheSize);
		_sdb.open();
	}

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
		//temp.display();
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

	bool getValueBetween(const KeyType& lowKey, const KeyType& highKey,
			vector<ElementType>& result);
	void getValueIn(const vector<KeyType>& vKey, vector<ElementType>& result);

	void getValueGreat(const KeyType& key, vector<ElementType>& result) {
		KeyType temp = getNext(key);
		myKeyType ikey(temp, 0);
		IndexSDBCursor locn;
		_sdb.search(ikey, locn);
		myDataType rec;
		while (_sdb.seq(locn, ESD_FORWARD) ) {
			if (_sdb.get(locn, rec)) {
				vector<ElementType> vdat = rec.get_value();
				for (size_t i=0; i<vdat.size(); i++)
					result.push_back(vdat[i]);
			}
		}
	}
	void getValueGreatEqual(const KeyType& key, vector<ElementType>& result) {
		getValue(key, result);
		getValueGreat(key, result);
	}

	void getValueLess(const KeyType& key, vector<ElementType>& result) {
		KeyType temp = getPrev(key);
		myKeyType ikey(temp, 0);
		IndexSDBCursor locn;
		_sdb.search(ikey, locn);
		myDataType rec;
		while (_sdb.seq(locn, ESD_BACKWARD) ) {
			if (_sdb.get(locn, rec) ) {
				vector<ElementType> vdat = rec.get_value();
				//rec.key.display();
				for (size_t i=0; i<vdat.size(); i++)
					result.push_back(vdat[i]);
			}
		}

	}
	void getValueLessEqual(const KeyType& key, vector<ElementType>& result) {
		getValue(key, result);
		getValueLess(key, result);
	}

	void getValuePrefix(const KeyType& key, vector<ElementType>& result) {
		myKeyType ikey(key, 0);
		myKeyType temp;
		myDataType idat;
		temp = _sdb.getNearest(ikey);
		IndexSDBCursor locn;
		_sdb.search(temp, locn);
		while (ikey.isPrefix(temp) ) {
			//if (_sdb.getValue(temp, idat)) 
			//{
			vector<ElementType> vdat = idat.get_value();
			for (size_t i=0; i<vdat.size(); i++) {
				//no duplicate
				if (find(result.begin(), result.end(), vdat[i]) == result.end() ) {
					result.push_back(vdat[i]);
				}
			}
			//}
			if (_sdb.seq(locn, ESD_FORWARD) )
				if (_sdb.get(locn, idat))
					temp = idat.get_key();
				else
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

	//if we can make sure there are no duplicate inserting items, e.g, documentID
	//can't be the same as the existing items in IndexSDB, i.e nodup is true. 
	bool add_nodup(const KeyType& key, const ElementType& item) {
		myKeyType ikey(key, 0);
		if ( !_sdb.hasKey(ikey) )
			return add(key, item);
		else {
			myKeyType ukey = _getUpper(key);
			myDataType udat;
			_sdb.getValue(ukey, udat);
			vector<ElementType> vdat = udat.get_value();
			if (vdat.size() < _vDataSize) {
				vdat.push_back(item);
				myDataType udat(ukey, vdat);
				_sdb.update(ukey, udat);
			} else {
				vector<ElementType> nv;
				nv.push_back(item);
				myKeyType newkey(key, ukey.offset+1);
				myDataType newData(newkey, nv);
				_sdb.insert(newData);
			}
		}
	}

protected:
	SequentialDB<myKeyType, myValueType, LockType> _sdb;

	//test;
	LockType lock_;
	unsigned int _vDataSize;

	myKeyType _getUpper(const KeyType& key) {
		unsigned int offset = 0;
		do {
			myDataType dat;
			myKeyType mykey(key, offset++);
			if (!_sdb.getValue(mykey, dat)) {
				return myKeyType(key, offset-2);//it should be offset-2,not offset-1.
			}
		} while (true);
	}
	void _dump(const vector<myDataType>& vDat, vector<ElementType>& result) {

		for (size_t i=0; i<vDat.size(); i++) {

			//#ifdef DEBUG
#if 0
			cout<<vDat[i].key.key.tid<<endl;
			cout<<vDat[i].key.offset<<endl;
#endif

			std::vector<ElementType> vdat;
			vdat = vDat[i].get_value();
			for (size_t j=0; j<vdat.size(); j++) {
				result.push_back(vdat[j]);
				//cout<<vDat[i].data[j]<<endl;
			}
		}
	}

};

template<class KeyType, class ElementType, class LockType> bool IndexSDB<
		KeyType, ElementType, LockType>::getValue(const KeyType& key,
		vector<ElementType>& result) {

	myDataType idat;
	unsigned int offset = 0;
	myKeyType firstKey(key, 0);
	if ( !_sdb.hasKey(firstKey) )
		return false;
	do {
		myKeyType ikey(key, offset++);
		if (_sdb.getValue(ikey, idat)) {
			vector<ElementType> vdat = idat.get_value();
			for (size_t i=0; i<vdat.size(); i++) {
				result.push_back(vdat[i]);
			}
		} else {
			break;
		}

	} while (true);
	return true;
}

template<class KeyType, class ElementType, class LockType> void IndexSDB<
		KeyType, ElementType, LockType>::getValueIn(const vector<KeyType>& vKey,
		vector<ElementType>& result) {

	for (size_t i=0; i<vKey.size(); i++) {
		getValue(vKey[i], result);
	}
}

template<class KeyType, class ElementType, class LockType> bool IndexSDB<
		KeyType, ElementType, LockType>::getValueBetween(const KeyType& lowKey,
		const KeyType& highKey, vector<ElementType>& result) {

	if (lowKey.compare(highKey)> 0) {
		return false;
	}
	vector<myDataType> vDat;

	myKeyType iLowKey(lowKey, 0);
	myKeyType iHighKey = _getUpper(highKey);

	_sdb.getValueBetween(vDat, iLowKey, iHighKey);
	_dump(vDat, result);
	return true;

}

template<class KeyType, class ElementType, class LockType> bool IndexSDB<
		KeyType, ElementType, LockType>::del(const KeyType& key) {

	unsigned int offset = 0;
	do {
		myKeyType ikey(key, offset++);
		if (_sdb.del(ikey)) {

		} else {
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
