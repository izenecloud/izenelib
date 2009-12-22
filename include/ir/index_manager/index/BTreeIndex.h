/**
 * @file        BTreeIndex.h
 * @author     Yingfeng Zhang
 * @version     SF1 v5.0
 * @brief head file of BTree Indexer, it is an enclosion of wiselib::IndexSDB
 */

#ifndef BTREEINDEX_H
#define BTREEINDEX_H

#include <ir/index_manager/index/IndexerDocument.h>
#include <ir/index_manager/utility/BitVector.h>
#include <ir/index_manager/utility/StringUtils.h>

#include <sdb/SequentialDB.h>
#include <sdb/IndexSDB.h>
#include <sdb/TrieIndexSDB.h>
#include <util/BoostVariantUtil.h>

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

    void get_between(const KeyType& lowKey, const KeyType& highKey, KeyValueType* & data, size_t maxDoc )
    {
        if (comp_(lowKey, highKey) > 0)
        {
            return;
        }
        myKeyType ikey(lowKey, 0);
        myValueType ival;
        IndexSDBCursor locn;
        this->_sdb.search(ikey, locn);
        docid_t currDoc;
        while (this->_sdb.get(locn, ikey, ival) )
        {
            if (comp_(ikey.key, highKey) <= 0)
            {
                for (size_t i=0; i<ival.size(); i++)
                {
                    currDoc = ival[i];
                    if (currDoc >= maxDoc)
                    {
                        KeyValueType* ppBytes = new KeyValueType[maxDoc + MAX_NUMERICSIZER];
                        memcpy(ppBytes, data, maxDoc * sizeof(KeyValueType));
                        memset(ppBytes+maxDoc, 0, maxDoc * sizeof(KeyValueType));
                        delete[] data;
                        data = ppBytes;
                        maxDoc = maxDoc + MAX_NUMERICSIZER;
                    }
                    data[currDoc] = ikey.key.value;
                }
                this->_sdb.seq(locn, ESD_FORWARD);
            }
            else
                break;
        }
    }


    bool get_without(const KeyType& key,BitVector& result)
    {
        myValueType vdat;
        unsigned int offset = 0;
        myKeyType firstKey(key, 0);
        if ( !this->_sdb.hasKey(firstKey) )
            return false;
        result.setAll();
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
        while (this->_sdb.get(locn, ikey, ival) )
        {
            if (ikey.key.fid != key.fid)
                break;
            if (comp_(ikey.key, key) > 0)
            {
                for (size_t i=0; i<ival.size(); i++)
                    result.set(ival[i]);
            }
            this->_sdb.seq(locn, ESD_FORWARD);
        }
    }

    void getGreatEqual(const KeyType& key, BitVector& result)
    {
        myKeyType ikey(key, 0);
        myValueType ival;
        IndexSDBCursor locn;
        this->_sdb.search(ikey, locn);
        while (this->_sdb.get(locn, ikey, ival) )
        {
            if (ikey.key.fid != key.fid)
                break;
            if (comp_(ikey.key, key) >= 0)
            {
                for (size_t i=0; i<ival.size(); i++)
                    result.set(ival[i]);
            }
            this->_sdb.seq(locn, ESD_FORWARD);
        }


    }

    void getLess(const KeyType& key, BitVector& result)
    {
        myKeyType ikey(key, 0);
        myValueType ival;
        IndexSDBCursor locn;
        this->_sdb.search(ikey, locn);
        while (this->_sdb.get(locn, ikey, ival) )
        {
            if (ikey.key.fid != key.fid)
                break;
            if (comp_(ikey.key, key) < 0)
            {
                for (size_t i=0; i<ival.size(); i++)
                    result.set(ival[i]);
            }
            this->_sdb.seq(locn, ESD_BACKWARD);
        }
    }

    void getLessEqual(const KeyType& key, BitVector& result)
    {
        myKeyType ikey(key, 0);
        myValueType ival;
        IndexSDBCursor locn;
        this->_sdb.search(ikey, locn);
        while (this->_sdb.get(locn, ikey, ival) )
        {
            if (ikey.key.fid != key.fid)
                break;
            if (comp_(ikey.key, key) <= 0)
            {
                for (size_t i=0; i<ival.size(); i++)
                    result.set(ival[i]);
            }
            this->_sdb.seq(locn, ESD_BACKWARD);
        }
    }

    void getPrefix(const KeyType& key, BitVector& result)
    {
        myKeyType ikey(key, 0);
        myValueType ival;

        IndexSDBCursor locn=	this->_sdb.search(ikey);
        while (this->_sdb.get(locn, ikey, ival) )
        {
            if (ikey.key.fid != key.fid)
                break;
            if ( isPrefix1(key, ikey.key) )
            {
                for (size_t i=0; i<ival.size(); i++)
                    result.set(ival[i]);
                this->_sdb.seq(locn);
            }
            else
                break;
        }
    }
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
            if (skey.first.second != fid)
                continue;
            if (isPrefix1(key, skey.first.first) )
            {
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
            if (skey.first.second != fid)
                continue;
            if (key == skey.first.first)
            {
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

class BTreeIndexerInterface
{
public:
    virtual ~BTreeIndexerInterface() {};

    virtual void add(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid) = 0;

    virtual void remove(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid) = 0;
};

/**
 * BTreeIndexer
 * @brief BTreeIndexer is a wrapper which takes charges of managing all b-tree handlers.
 */
class BTreeIndexer:public BTreeIndexerInterface
{
public:
    BTreeIndexer(string location, int degree, size_t cacheSize, size_t maxDataSize);

    virtual ~BTreeIndexer();
public:
    void add(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid);

    void remove(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid);

    void getValue(collectionid_t colID, fieldid_t fid, PropertyType& value, BitVector& docs);

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

    template<typename T>
    struct BTreeIndexerFactory
    {
        static BTreeIndex<IndexKeyType<T> >* get()
        {
            return NULL;
        }
    };

    template<typename T> static BTreeIndex<IndexKeyType<T> >* getIndexer()
    {
        return BTreeIndexerFactory<T>::get();
    }

    static BTreeTrieIndex<String>* getTrieIndexer()
    {
        return pBTreeUStrSuffixIndexer_;
    }

private:

    static BTreeIndex<IndexKeyType<int64_t> >* pBTreeIntIndexer_;

    static BTreeIndex<IndexKeyType<uint64_t> >* pBTreeUIntIndexer_;

    static BTreeIndex<IndexKeyType<float> >* pBTreeFloatIndexer_;

    static BTreeIndex<IndexKeyType<double> >* pBTreeDoubleIndexer_;

    static BTreeIndex<IndexKeyType<String> >* pBTreeUStrIndexer_;

    static BTreeTrieIndex<String>* pBTreeUStrSuffixIndexer_;

};

template<>
struct BTreeIndexer::BTreeIndexerFactory<int64_t>
{
    static BTreeIndex<IndexKeyType<int64_t> >* get()
    {
        return BTreeIndexer::pBTreeIntIndexer_;
    };
};

template<>
struct BTreeIndexer::BTreeIndexerFactory<uint64_t>
{
    static BTreeIndex<IndexKeyType<uint64_t> >* get()
    {
        return BTreeIndexer::pBTreeUIntIndexer_;
    };
};

template<>
struct BTreeIndexer::BTreeIndexerFactory<float>
{
    static BTreeIndex<IndexKeyType<float> >* get()
    {
        return BTreeIndexer::pBTreeFloatIndexer_;
    };
};

template<>
struct BTreeIndexer::BTreeIndexerFactory<double>
{
    static BTreeIndex<IndexKeyType<double> >* get()
    {
        return BTreeIndexer::pBTreeDoubleIndexer_;
    };
};

template<>
struct BTreeIndexer::BTreeIndexerFactory<String>
{
    static BTreeIndex<IndexKeyType<String> >* get()
    {
        return BTreeIndexer::pBTreeUStrIndexer_;
    };
};

class add_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(collectionid_t colid,fieldid_t fid, T& v, docid_t docid)
    {
        __operator<T>::apply(colid, fid, v, docid);
    }

private:

    template<typename T>
    struct __operator
    {
        static void apply(collectionid_t& colid, fieldid_t& fid, T& v, docid_t& docid)
        {
            IndexKeyType<T> key(colid, fid, v);
            BTreeIndexer::getIndexer<T>()->add_nodup(key, docid);
        }
    };

};

template<>
struct add_visitor::__operator<String>
{
    static void apply(collectionid_t& colid, fieldid_t& fid, String& v, docid_t& docid)
    {
        trim(v);
        IndexKeyType<String> key(colid, fid, v);
        BTreeIndexer::getIndexer<String>()->add_nodup(key, docid);
        BTreeIndexer::getTrieIndexer()->add_suffix(v, fid, docid);
    }
};

class remove_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(collectionid_t colid,fieldid_t fid,T& v, docid_t docid)
    {
        IndexKeyType<T> key(colid, fid, v);
        BTreeIndexer::getIndexer<T>()->remove(key, docid);
    }
};

class get_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(collectionid_t colid, fieldid_t fid, T& v, BitVector& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        BTreeIndexer::getIndexer<T>()->get(key, docids);
    }
};

class get_without_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(collectionid_t colid, fieldid_t fid, T& v, BitVector& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        BTreeIndexer::getIndexer<T>()->get_without(key, docids);
    }
};

template<typename T>
class get_between_visitor : public boost::static_visitor<void>
{
public:
    void operator()(collectionid_t colid,fieldid_t fid, T& v1, T& v2, BitVector& docids)
    {
        IndexKeyType<T> key1(colid, fid,v1);
        IndexKeyType<T> key2(colid, fid,v2);
        BTreeIndexer::getIndexer<T>()->getBetween(key1,key2,docids);
    }
};

class get_less_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(collectionid_t colid, fieldid_t fid, T& v, BitVector& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        BTreeIndexer::getIndexer<T>()->getLess(key, docids);
    }
};

class get_less_equal_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(collectionid_t colid, fieldid_t fid, T& v, BitVector& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        BTreeIndexer::getIndexer<T>()->getLessEqual(key, docids);
    }
};

class get_great_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(collectionid_t colid, fieldid_t fid, T& v, BitVector& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        BTreeIndexer::getIndexer<T>()->getGreat(key, docids);
    }
};

class get_great_equal_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(collectionid_t colid, fieldid_t fid, T& v, BitVector& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        BTreeIndexer::getIndexer<T>()->getGreatEqual(key, docids);
    }
};

}

NS_IZENELIB_IR_END

#endif /*BTreeIndex_H*/
