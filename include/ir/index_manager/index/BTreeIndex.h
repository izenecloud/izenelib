/**
* @file        BTreeIndex.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief head file of BTree Indexer, it is an enclosion of wiselib::IndexSDB
*/

#ifndef BTREEINDEX_H
#define BTREEINDEX_H

#include <ir/index_manager/index/IndexerDocument.h>

#include <sdb/SequentialDB.h>
#include <sdb/IndexSDB.h>
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

namespace indexmanager{


template<class T>
struct IndexKeyType
{
    collectionid_t cid;
    fieldid_t fid;
    T value;

    IndexKeyType():cid(0),fid(0),value(T()){}

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
        else{
            if (fid != other.fid)
                return false;
            else{
                String str;
                other.value.substr(str, 0, value.size());
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
        return (int)(value - other.value);
    }
    int __compare(const IndexKeyType<T>& other, const boost::mpl::false_*) const
    {
        return value.compare(other.value);
    }
};

struct USuffix
{
    String suf;

    USuffix(){}

    USuffix(String& other) :suf(other) {}
    template<class Archive> void serialize(Archive & ar,const unsigned int version)
    {
        ar & suf;
    }
    int compare(const USuffix& other) const
    {
        return suf.compare(other.suf);
    }
    bool isPrefix(const USuffix& other) const
    {
        String str;
        other.suf.substr(str, 0, suf.size());
        if (str == suf)
            return true;
        else
            return false;
    }
    void display() const
    {
        //cout<<suf;
    }
};


/**
* BTreeIndex
*/


template <class KeyType>
class BTreeIndex:public izenelib::sdb::IndexSDB<KeyType,docid_t>
{
public:

    BTreeIndex(string& fileName) :
            izenelib::sdb::IndexSDB<KeyType,docid_t>(fileName) //0, use default compare
    {
    }
    bool remove(const KeyType& key, docid_t docID);
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

    void getValue(collectionid_t colID, fieldid_t fid, PropertyType& value, vector<docid_t>& docs);

    void getValueBetween(collectionid_t colID, fieldid_t fid, PropertyType& value1, PropertyType& value2, vector<docid_t>& docs);

    void getValueLess(collectionid_t colID, fieldid_t fid, PropertyType& value, vector<docid_t>& docs);

    void getValueLessEqual(collectionid_t colID, fieldid_t fid, PropertyType& value, vector<docid_t>& docs);

    void getValueGreat(collectionid_t colID, fieldid_t fid, PropertyType& value, vector<docid_t>& docs);

    void getValueGreatEqual(collectionid_t colID, fieldid_t fid, PropertyType& value, vector<docid_t>& docs);

    void getValueIn(collectionid_t colID, fieldid_t fid, vector<PropertyType>& values, vector<docid_t>& docs);

    void getValueNotIn(collectionid_t colID, fieldid_t fid, vector<PropertyType>& values, vector<docid_t>& docs);

    void getValueNotEqual(collectionid_t colID, fieldid_t fid, PropertyType& value, vector<docid_t>& docs);

    void getValueStart(collectionid_t colID, fieldid_t fid, PropertyType& value, vector<docid_t>& docs);

    void getValueEnd(collectionid_t colID, fieldid_t fid, PropertyType& value, vector<docid_t>& docs);

    void getValueSubString(collectionid_t colID, fieldid_t fid, PropertyType& value, vector<docid_t>& docs);

    void flush();

    template<typename T>
    struct BTreeIndexerFactory
    {
        static BTreeIndex<IndexKeyType<T> >* get(){return NULL;}
    };

    template<typename T> static BTreeIndex<IndexKeyType<T> >* getIndexer()
    {
        return BTreeIndexerFactory<T>::get();
    }

private:
    void uniqueResults(vector<docid_t>& docs)
    {
        sort(docs.begin(),docs.end());
        vector<docid_t>::iterator docIt = unique(docs.begin(),docs.end());
        docs.resize(docIt - docs.begin());
    }
private:

    static BTreeIndex<IndexKeyType<int> >* pBTreeIntIndexer_;

    static BTreeIndex<IndexKeyType<unsigned int> >* pBTreeUIntIndexer_;

    static BTreeIndex<IndexKeyType<float> >* pBTreeFloatIndexer_;

    static BTreeIndex<IndexKeyType<double> >* pBTreeDoubleIndexer_;

    static BTreeIndex<IndexKeyType<String> >* pBTreeUStrIndexer_;

    static izenelib::sdb::IndexSDB<USuffix, String>* pBTreeUStrSuffixIndexer_;

};


template<>
struct BTreeIndexer::BTreeIndexerFactory<int>
{
	static BTreeIndex<IndexKeyType<int> >* get()
	{
		return BTreeIndexer::pBTreeIntIndexer_;
	};
};

template<>
struct BTreeIndexer::BTreeIndexerFactory<unsigned int>
{
	static BTreeIndex<IndexKeyType<unsigned int> >* get()
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
    void operator()(collectionid_t colid,fieldid_t fid, T& v,  docid_t docid)
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
            BTreeIndexer::getIndexer<T>()->add(key, docid);
        }
    };

};

template<>
struct add_visitor::__operator<String>
{
    static void apply(collectionid_t& colid, fieldid_t& fid, String& v, docid_t& docid)
    {
        IndexKeyType<String> key(colid, fid, v);
        BTreeIndexer::getIndexer<String>()->add(key, docid);
#ifdef INDEX_USE_STAR_SEARCH
        size_t pos = 0;
        for (; pos<v.length(); pos++)
        {
            UString suf;
            v.substr(suf,pos);
            USuffix skey(suf);
            BTreeIndexer::pBTreeUStrSuffixIndexer_->add(skey, v);
        }
#endif
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
    void operator()(collectionid_t colid, fieldid_t fid, T& v, vector<docid_t>& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        BTreeIndexer::getIndexer<T>()->getValue(key, docids);
    }
};

template<typename T>
class get_between_visitor : public boost::static_visitor<void>
{
public:
    void operator()(collectionid_t colid,fieldid_t fid, T& v1, T& v2, vector<docid_t>& docids)
    {
        IndexKeyType<T> key1(colid, fid,v1);
        IndexKeyType<T> key2(colid, fid,v2);
        BTreeIndexer::getIndexer<T>()->getValueBetween(key1,key2,docids);
    }
};

class get_less_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(collectionid_t colid, fieldid_t fid, T& v, vector<docid_t>& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        BTreeIndexer::getIndexer<T>()->getValueLess(key, docids);
    }
};

class get_less_equal_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(collectionid_t colid, fieldid_t fid, T& v, vector<docid_t>& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        BTreeIndexer::getIndexer<T>()->getValueLessEqual(key, docids);
    }
};

class get_great_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(collectionid_t colid, fieldid_t fid, T& v, vector<docid_t>& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        BTreeIndexer::getIndexer<T>()->getValueGreat(key, docids);
    }
};

class get_great_equal_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(collectionid_t colid, fieldid_t fid, T& v, vector<docid_t>& docids)
    {
        IndexKeyType<T> key(colid, fid, v);
        BTreeIndexer::getIndexer<T>()->getValueGreatEqual(key, docids);
    }
};

}

NS_IZENELIB_IR_END

#endif /*BTreeIndex_H*/
