/**
 * @file        BTreeIndex.h
 * @author     Yingfeng Zhang
 * @version     SF1 v5.0
 * @brief head file of BTree Indexer, it is an enclosion of wiselib::IndexSDB
 */

#ifndef BTREEINDEX_H
#define BTREEINDEX_H
#include <sdb/SequentialDB.h>
#include <sdb/IndexSDB.h>
#include <sdb/TrieIndexSDB.h>
#include <util/BoostVariantUtil.h>

#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/utility/BitVector.h>
#include <ir/index_manager/utility/StringUtils.h>
#include <ir/index_manager/store/Directory.h>

#include <boost/variant.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/not.hpp>
#include <boost/bind.hpp>
#include <functional>

#include <algorithm>
#include <string>

NS_IZENELIB_IR_BEGIN

#define MAX_NUMERICSIZER 32768

namespace indexmanager
{

template<class T>
struct IndexKeyType
{
    collectionid_t cid;
    fieldid_t fid;
    T value;
    typedef T type;

    IndexKeyType():cid(0),fid(0),value(T()) {}

    IndexKeyType(collectionid_t c, fieldid_t f, T& v)
            :cid(c), fid(f), value(v)
    {}
    template<class Archive> void serialize(Archive & ar,const unsigned int version)
    {
        ar & fid;
        ar & cid;
        ar & value;
    }

    int compare(const IndexKeyType<T>& other) const
    {
        if (cid != other.cid)
            return cid-other.cid;
        else
            if (fid != other.fid)
                return fid-other.fid;
            else
                return __compare(other,static_cast<boost::is_arithmetic<T>*>(0));
    }

    bool isPrefix(const IndexKeyType<T>& other) const
    {
        if (cid != other.cid)
            return false;
        else
        {
            if (fid != other.fid)
                return false;
            else
            {
                String str;
                other.value.substr(str, 0, value.length());
                if (str == value)
                    return true;
                else
                    return false;
            }
        }
    }

    void display() const
    {
        cout<<cid<<" ";
        cout<<fid<<" ";
        //cout<<value;
    }
    bool sameField(const IndexKeyType<T>& other) const
    {
        return ((cid == other.cid) && (fid == other.fid));
    }

private:
    int __compare(const IndexKeyType<T>& other, const boost::mpl::true_*) const
    {
        if ( value> other.value)
        {
            return 1;
        }
        else if (value < other.value)
        {
            return -1;
        }
        else
            return 0;
        //return (int)(value - other.value);
    }
    int __compare(const IndexKeyType<T>& other, const boost::mpl::false_*) const
    {
        return value.compare(other.value);
    }
};

/**
 * BTreeIndex
 */

template <class KeyType>
class BTreeIndex:public izenelib::sdb::IndexSDB<KeyType,docid_t,izenelib::util::ReadWriteLock>
{
    typedef typename KeyType::type KeyValueType;
    typedef izenelib::sdb::iKeyType<KeyType> myKeyType;
    typedef std::vector<docid_t> myValueType;
    typedef DataType<myKeyType, myValueType> myDataType;
public:
    typedef typename IndexSDB<KeyType,docid_t,izenelib::util::ReadWriteLock>::IndexSDBCursor IndexSDBCursor;

public:

    BTreeIndex(string& fileName) :
            izenelib::sdb::IndexSDB<KeyType,docid_t,izenelib::util::ReadWriteLock>(fileName) //0, use default compare
    {
    }
    bool remove(const KeyType& key, docid_t docID);

    bool seek(const KeyType& key)
    {
        myKeyType firstKey(key, 0);
        if ( !this->_sdb.hasKey(firstKey) )
            return false;
        return true;
    }

    void getDocList(const KeyType& key,BitVector& result)
    {
        myValueType vdat;
        unsigned int offset = 0;
        do
        {
            myKeyType ikey(key, offset++);
            if (this->_sdb.getValue(ikey, vdat))
                for (size_t i=0; i<vdat.size(); i++)
                    result.set(vdat[i]);
            else
                break;
        }
        while (true);
    }

    bool get(const KeyType& key,BitVector& result)
    {
        myValueType vdat;
        unsigned int offset = 0;
        myKeyType firstKey(key, 0);
        if ( !this->_sdb.hasKey(firstKey) )
            return false;
        do
        {
            myKeyType ikey(key, offset++);
            if (this->_sdb.getValue(ikey, vdat))
                for (size_t i=0; i<vdat.size(); i++)
                    result.set(vdat[i]);
            else
                break;
        }
        while (true);
        return true;
    }

    bool get(const KeyType& key,std::vector<docid_t>& result)
    {
        myValueType vdat;
        unsigned int offset = 0;
        myKeyType firstKey(key, 0);
        if ( !this->_sdb.hasKey(firstKey) )
            return false;
        do
        {
            myKeyType ikey(key, offset++);
            if (this->_sdb.getValue(ikey, vdat))
                for (size_t i=0; i<vdat.size(); i++)
                    result.push_back(vdat[i]);
            else
                break;
        }
        while (true);
        return true;
    }

    size_t get_between(const KeyType& lowKey, const KeyType& highKey, KeyValueType* & data, size_t maxDoc )
    {
        if (comp_(lowKey, highKey) > 0)
        {
            return 0;
        }
        myKeyType ikey(lowKey, 0);
        myValueType ival;
        IndexSDBCursor locn;
        this->_sdb.search(ikey, locn);
        docid_t currDoc;
        size_t count = 0;

        do
        {
            if (this->_sdb.get(locn, ikey, ival))
            {
                if(comp_(ikey.key, highKey) <= 0)
                {
                    for (size_t i=0; i<ival.size(); i++)
                    {
                        currDoc = ival[i];
                        if (currDoc >= maxDoc)
                        {
                            KeyValueType* ppBytes = new KeyValueType[maxDoc + MAX_NUMERICSIZER];
                            memcpy(ppBytes, data, maxDoc * sizeof(KeyValueType));
                            memset(ppBytes+maxDoc, 0, MAX_NUMERICSIZER * sizeof(KeyValueType));
                            delete[] data;
                            data = ppBytes;
                            maxDoc = maxDoc + MAX_NUMERICSIZER;
                        }
                        data[currDoc] = ikey.key.value;
                    }
                    count += ival.size();
                }
                else
                    break;
            }
        }while (this->_sdb.seq(locn, ESD_FORWARD));

        return count;
    }

    void get_between(const KeyType& lowKey, const KeyType& highKey, BitVector& result)
    {
        if (comp_(lowKey, highKey) > 0)
        {
            return;
        }
        myKeyType ikey(lowKey, 0);
        myValueType ival;
        IndexSDBCursor locn;
        this->_sdb.search(ikey, locn);

        do
        {
            if(this->_sdb.get(locn, ikey, ival))
            {
                if(ikey.key.fid == highKey.fid)	
                {
                    if (comp_(ikey.key, highKey) <= 0)
                    {
                        for (size_t i=0; i<ival.size(); i++)
                            result.set(ival[i]);
                    }
                }
                else break;
            }
        }while(this->_sdb.seq(locn, ESD_FORWARD));
    }

    bool get_without(const KeyType& key,BitVector& result)
    {
        myValueType vdat;
        unsigned int offset = 0;
        myKeyType firstKey(key, 0);
        result.setAll();
        if ( !this->_sdb.hasKey(firstKey) )
            return false;
        do
        {
            myKeyType ikey(key, offset++);
            if (this->_sdb.getValue(ikey, vdat))
                for (size_t i=0; i<vdat.size(); i++)
                    result.clear(vdat[i]);
            else
                break;
        }
        while (true);
        return true;
    }

    void getGreat(const KeyType& key, BitVector& result)
    {
        myKeyType ikey(key, 0);
        myValueType ival;
        IndexSDBCursor locn;
        this->_sdb.search(ikey, locn);

        do
        {
            if(this->_sdb.get(locn, ikey, ival))
            {
                if(ikey.key.fid == key.fid)
                {
                    if(comp_(ikey.key, key) > 0)
                    {
                        for (size_t i=0; i<ival.size(); i++)
                            result.set(ival[i]);
                    }
                }
                else break;
            }
        }while (this->_sdb.seq(locn, ESD_FORWARD));
    }

    void getGreatEqual(const KeyType& key, BitVector& result)
    {
        myKeyType ikey(key, 0);
        myValueType ival;
        IndexSDBCursor locn;
        this->_sdb.search(ikey, locn);
        do
        {
            if(this->_sdb.get(locn, ikey, ival))
            {
                if(ikey.key.fid == key.fid)				
                {
                    if(comp_(ikey.key, key) >= 0)
                    {
                        for (size_t i=0; i<ival.size(); i++)
                            result.set(ival[i]);
                    }
                }
                else break;
            }
        }while (this->_sdb.seq(locn, ESD_FORWARD));


    }

    void getLess(const KeyType& key, BitVector& result)
    {
        myKeyType ikey(key, 0);
        myValueType ival;
        IndexSDBCursor locn;
        this->_sdb.search(ikey, locn);
        do
        {
            if(this->_sdb.get(locn, ikey, ival))
            {
                if(ikey.key.fid == key.fid)
                {
                    if (comp_(ikey.key, key) < 0)
                    {
                        for (size_t i=0; i<ival.size(); i++)
                            result.set(ival[i]);
                    }
                }
                else break;
            }
        }while (this->_sdb.seq(locn, ESD_BACKWARD));
    }

    void getLessEqual(const KeyType& key, BitVector& result)
    {
        myKeyType ikey(key, 0);
        myValueType ival;
        IndexSDBCursor locn;
        this->_sdb.search(ikey, locn);
        do
        {
            if(this->_sdb.get(locn, ikey, ival))
            {
                if(ikey.key.fid == key.fid)
                {
                    if (comp_(ikey.key, key) <= 0)
                    {
                        for (size_t i=0; i<ival.size(); i++)
                            result.set(ival[i]);
                    }
                }
                else break;
            }
        }while (this->_sdb.seq(locn, ESD_BACKWARD));
    }

    void getPrefix(const KeyType& key, BitVector& result)
    {
        myKeyType ikey(key, 0);
        myValueType ival;

        IndexSDBCursor locn= this->_sdb.search(ikey);
        do 
        {
            if(this->_sdb.get(locn, ikey, ival) )
            {
                if(ikey.key.fid == key.fid)
                {
                    if ( isPrefix1(key, ikey.key))
                    {
                        for (size_t i=0; i<ival.size(); i++)
                            result.set(ival[i]);
                    }
                }
                else break;
            }
        }while(this->_sdb.seq(locn));
    }


    void getSuffix(const KeyType& key, BitVector& result);

    void getSubString(const KeyType& key, BitVector& result);
};

template <class KeyType>
bool BTreeIndex<KeyType>::remove(const KeyType& key, docid_t docID)
{
    vector<docid_t> vDat;
    getValue(key, vDat);

    //if we can make sure there are no duplicate inserting items, e.g, documentID
    //can't be the same as the existing items in IndexSDB, i.e nodup is true.
    vector<docid_t>::iterator iter = find(vDat.begin(), vDat.end(), docID);
    if (iter != vDat.end() )
    {
        vDat.erase(iter);
        return update(key, vDat);
    }
    else
    {
        return false;
    }

}

template <typename StringType, typename LockType =izenelib::util::ReadWriteLock>
class BTreeTrieIndex
{
public:
    typedef SequentialDB<std::pair<std::pair<StringType,fieldid_t>, docid_t>, NullType> SDBTYPE;
    typedef typename SDBTYPE::SDBCursor SDBCursor;
public:

    BTreeTrieIndex(const string& fileName) 
        :sdb_(fileName)
    {
    }
    bool open()
    {
        return sdb_.open();
    }

    void getValuePrefix(const StringType& key,const fieldid_t& fid, BitVector& result)
    {
        SDBCursor locn = this->sdb_.search(make_pair(make_pair(key,fid), 0) );
        std::pair<std::pair<StringType,fieldid_t>, docid_t> skey;
        StringType lstr;
        NullType sval;
        while (this->sdb_.get(locn, skey, sval) )
        {
            if (isPrefix1(key, skey.first.first) )
            {   
                if (skey.first.second == fid)
                    result.set(skey.second);
                this->sdb_.seq(locn);
            }
            else
                break;
        }
    }

    void getValueSuffix(const StringType& key, const fieldid_t& fid, BitVector& result)
    {
        SDBCursor locn = this->sdb_.search(make_pair(make_pair(key,fid), 0 ) );
        std::pair<std::pair<StringType,fieldid_t>, docid_t> skey;
        StringType lstr;
        NullType sval;
        while (this->sdb_.get(locn, skey, sval) )
        {
            if (key == skey.first.first)
            {
	            if (skey.first.second == fid)
                    result.set(skey.second);
                this->sdb_.seq(locn);
            }
            else
                break;
        }

    }


    bool add_suffix(const StringType& key, const fieldid_t& fid, const docid_t& item)
    {
        if (sdb_.hasKey(make_pair(make_pair(key,fid), item) ) )
            return false;
        size_t pos = 0;
        for (; pos<key.length(); pos++)
        {
            StringType suf = key.substr(pos);
            add(suf, fid, item);
        }
        return false;
    }

    bool add(const StringType& key, const fieldid_t& fid, const docid_t& item)
    {
        sdb_.insertValue(make_pair(make_pair(key,fid), item) );
        return false;
    }

    void display()
    {
        sdb_.display();
    }

    void commit()
    {
        sdb_.commit();
    }

    void flush()
    {
        sdb_.flush();
    }
protected:
    SDBTYPE sdb_;

};

/**
 * BTreeIndexer
 * @brief BTreeIndexer is a wrapper which takes charges of managing all b-tree handlers.
 */
class BTreeIndexer
{
public:
    BTreeIndexer(Directory* pDirectory, string location, int degree, size_t cacheSize, size_t maxDataSize);

    virtual ~BTreeIndexer();
public:
    bool seek(collectionid_t colID, fieldid_t fid, PropertyType& value);

    void getNoneEmptyList(collectionid_t colID, fieldid_t fid, PropertyType& value, BitVector& docs);

    void add(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid);

    void remove(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid);

    void getValue(collectionid_t colID, fieldid_t fid, PropertyType& value, BitVector& docs);

    void getValue(collectionid_t colID, fieldid_t fid, PropertyType& value, std::vector<docid_t>& docList);

    void getValueBetween(collectionid_t colID, fieldid_t fid, PropertyType& value1, PropertyType& value2, BitVector& docs);

    void getValueLess(collectionid_t colID, fieldid_t fid, PropertyType& value, BitVector& docs);

    void getValueLessEqual(collectionid_t colID, fieldid_t fid, PropertyType& value, BitVector& docs);

    void getValueGreat(collectionid_t colID, fieldid_t fid, PropertyType& value, BitVector& docs);

    void getValueGreatEqual(collectionid_t colID, fieldid_t fid, PropertyType& value, BitVector& docs);

    void getValueIn(collectionid_t colID, fieldid_t fid, vector<PropertyType>& values, BitVector& docs);

    void getValueNotIn(collectionid_t colID, fieldid_t fid, vector<PropertyType>& values, BitVector& docs);

    void getValueNotEqual(collectionid_t colID, fieldid_t fid, PropertyType& value, BitVector& docs);

    void getValueStart(collectionid_t colID, fieldid_t fid, PropertyType& value, BitVector& docs);

    void getValueEnd(collectionid_t colID, fieldid_t fid, PropertyType& value, BitVector& docs);

    void getValueSubString(collectionid_t colID, fieldid_t fid, PropertyType& value, BitVector& docs);

    void flush();

    void setFilter(boost::shared_ptr<BitVector> pBitVector);

    boost::shared_ptr<BitVector> getFilter();

    void delDocument(size_t max_doc, docid_t docId);

    template<typename T> BTreeIndex<IndexKeyType<T> >* getIndexer();

//    static BTreeTrieIndex<String>* getTrieIndexer()
//    {
//        return pBTreeUStrSuffixIndexer_;
//    }

private:

    BTreeIndex<IndexKeyType<int64_t> >* pBTreeIntIndexer_;

    BTreeIndex<IndexKeyType<uint64_t> >* pBTreeUIntIndexer_;

    BTreeIndex<IndexKeyType<float> >* pBTreeFloatIndexer_;

    BTreeIndex<IndexKeyType<double> >* pBTreeDoubleIndexer_;

    BTreeIndex<IndexKeyType<String> >* pBTreeUStrIndexer_;

    boost::shared_ptr<BitVector> pFilter_;

    Directory* pDirectory_;

//    static BTreeTrieIndex<String>* pBTreeUStrSuffixIndexer_;

};

class add_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexer* pIndexer, collectionid_t colid,fieldid_t fid, T& v, docid_t docid)
    {
        IndexKeyType<T> key(colid, fid, v);
        pIndexer->getIndexer<T>()->add_nodup(key, docid);
    }
};

class remove_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexer* pIndexer, collectionid_t colid,fieldid_t fid,T& v, docid_t docid)
    {
        IndexKeyType<T> key(colid, fid, v);
        pIndexer->getIndexer<T>()->remove(key, docid);
    }
};

class seek_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexer* pIndexer, collectionid_t colid, fieldid_t fid, T& v, bool& find)
    {
        IndexKeyType<T> key(colid, fid, v);
        find = pIndexer->getIndexer<T>()->seek(key);
    }
};

class get_value_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexer* pIndexer, collectionid_t colid, fieldid_t fid, T& v, BitVector& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        pIndexer->getIndexer<T>()->getDocList(key, docids);
    }
};

class get_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexer* pIndexer, collectionid_t colid, fieldid_t fid, T& v, BitVector& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        pIndexer->getIndexer<T>()->get(key, docids);
    }
};

class get_back_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexer* pIndexer, collectionid_t colid, fieldid_t fid, T& v, std::vector<docid_t>& docidList)
    {
        IndexKeyType<T> key(colid, fid, v);
        pIndexer->getIndexer<T>()->get(key, docidList);
    }
};

class get_without_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexer* pIndexer, collectionid_t colid, fieldid_t fid, T& v, BitVector& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        pIndexer->getIndexer<T>()->get_without(key, docids);
    }
};

class get_between_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexer* pIndexer, collectionid_t colid,fieldid_t fid, T& v1, T& v2, BitVector& docids)
    {
        IndexKeyType<T> key1(colid, fid,v1);
        IndexKeyType<T> key2(colid, fid,v2);
        pIndexer->getIndexer<T>()->get_between(key1,key2,docids);
    }
    template<typename T1, typename T2>
    void operator()(BTreeIndexer* pIndexer, collectionid_t colid,fieldid_t fid, T1& v1, T2& v2, BitVector& docids)
    {
    }
};

class get_less_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexer* pIndexer, collectionid_t colid, fieldid_t fid, T& v, BitVector& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        pIndexer->getIndexer<T>()->getLess(key, docids);
    }
};

class get_less_equal_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexer* pIndexer, collectionid_t colid, fieldid_t fid, T& v, BitVector& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        pIndexer->getIndexer<T>()->getLessEqual(key, docids);
    }
};

class get_great_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexer* pIndexer, collectionid_t colid, fieldid_t fid, T& v, BitVector& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        pIndexer->getIndexer<T>()->getGreat(key, docids);
    }
};

class get_great_equal_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexer* pIndexer, collectionid_t colid, fieldid_t fid, T& v, BitVector& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        pIndexer->getIndexer<T>()->getGreatEqual(key, docids);
    }
};

}

NS_IZENELIB_IR_END

#endif /*BTreeIndex_H*/
