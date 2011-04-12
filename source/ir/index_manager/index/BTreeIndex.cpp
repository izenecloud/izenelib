#include <boost/any.hpp>

#include <ir/index_manager/index/BTreeIndex.h>

using namespace std;
using namespace izenelib::util;

using namespace izenelib::ir::indexmanager;

namespace izenelib{ namespace ir{ namespace indexmanager{
template <>
void BTreeIndex<IndexKeyType<String> >::getSuffix(const IndexKeyType<String>& key, BitVector& result)
{
    String str("",String::UTF_8);
    IndexKeyType<String> strkey(key.cid,key.fid,str);
    myKeyType ikey(strkey, 0);
    myValueType ival;
    IndexSDBCursor locn= this->_sdb.search(ikey);
    do
    {
        if(this->_sdb.get(locn, ikey, ival) )
            if(ikey.key.fid == key.fid)			
            {
                if ( IsSuffix(key.value, ikey.key.value) )
                {
                    for (size_t i=0; i<ival.size(); i++)
                        result.set(ival[i]);
                }
            }
            else break;
    }while(this->_sdb.seq(locn));
}


template <>
void BTreeIndex<IndexKeyType<String> >::getSubString(const IndexKeyType<String>& key, BitVector& result)
{
    String str("",String::UTF_8);
    IndexKeyType<String> strkey(key.cid,key.fid,str);
    myKeyType ikey(strkey, 0);
    myValueType ival;

    IndexSDBCursor locn;
    this->_sdb.search(ikey,locn);
    do
    {
        if(this->_sdb.get(locn, ikey, ival) )
            if(ikey.key.fid == key.fid)
            {
                if ( IsSubString(key.value, ikey.key.value) )
                {
                    for (size_t i=0; i<ival.size(); i++)
                        result.set(ival[i]);
                }
            }
            else break;
    }while (this->_sdb.seq(locn));
}

template<typename T> 
BTreeIndex<IndexKeyType<T> >* BTreeIndexer::getIndexer()
{
    return NULL;
}

template<> 
BTreeIndex<IndexKeyType<int64_t> >* BTreeIndexer::getIndexer()
{
    return pBTreeIntIndexer_;
}

template<> 
BTreeIndex<IndexKeyType<uint64_t> >* BTreeIndexer::getIndexer()
{
    return pBTreeUIntIndexer_;
}

template<> 
BTreeIndex<IndexKeyType<float> >* BTreeIndexer::getIndexer()
{
    return pBTreeFloatIndexer_;
}

template<> 
BTreeIndex<IndexKeyType<double> >* BTreeIndexer::getIndexer()
{
    return pBTreeDoubleIndexer_;
}

template<> 
BTreeIndex<IndexKeyType<String> >* BTreeIndexer::getIndexer()
{
    return pBTreeUStrIndexer_;
}

template<>
void add_visitor::operator()(BTreeIndexer* pIndexer, collectionid_t colid,fieldid_t fid, String& v, docid_t docid)
{
    trim(v);
    IndexKeyType<String> key(colid, fid, v);
    pIndexer->getIndexer<String>()->add_nodup(key, docid);
    //BTreeIndexer::getTrieIndexer()->add_suffix(v, fid, docid);
};
	
}}}

BTreeIndexer::BTreeIndexer(string location, int degree, size_t cacheSize, size_t maxDataSize)
{
    string path(location);
    path.append("/int.bti");
    pBTreeIntIndexer_ = new BTreeIndex<IndexKeyType<int64_t> >(path);
    pBTreeIntIndexer_->initialize(maxDataSize/10, degree, maxDataSize, cacheSize);

    path.clear();
    path = location+"/uint.bti";
    pBTreeUIntIndexer_ = new BTreeIndex<IndexKeyType<uint64_t> >(path);
    pBTreeUIntIndexer_->initialize(maxDataSize/10, degree, maxDataSize, cacheSize);

    path.clear();
    path = location+"/float.bti";
    pBTreeFloatIndexer_ = new BTreeIndex<IndexKeyType<float> >(path);
    pBTreeFloatIndexer_->initialize(maxDataSize/10, degree, maxDataSize, cacheSize);

    path.clear();
    path = location+"/double.bti";
    pBTreeDoubleIndexer_ = new BTreeIndex<IndexKeyType<double> >( path);
    pBTreeDoubleIndexer_->initialize(maxDataSize/10, degree, maxDataSize, cacheSize);

    path.clear();
    path = location+"/ustr.bti";
    pBTreeUStrIndexer_ = new BTreeIndex<IndexKeyType<String> >(path);
    pBTreeUStrIndexer_->initialize(maxDataSize/10, degree, maxDataSize, cacheSize);

/*
    path.clear();
    path = location+"/usuf.bti";
    pBTreeUStrSuffixIndexer_ = new BTreeTrieIndex<String>( path);
    pBTreeUStrSuffixIndexer_->open();
*/	
}

BTreeIndexer::~BTreeIndexer()
{
    flush();

    if (pBTreeIntIndexer_)
    {
        delete pBTreeIntIndexer_;
        pBTreeIntIndexer_ = NULL;
    }
    if (pBTreeUIntIndexer_)
    {
        delete pBTreeUIntIndexer_;
        pBTreeUIntIndexer_ = NULL;
    }
    if (pBTreeFloatIndexer_)
    {
        delete pBTreeFloatIndexer_;
        pBTreeFloatIndexer_ = NULL;
    }
    if (pBTreeDoubleIndexer_)
    {
        delete pBTreeDoubleIndexer_;
        pBTreeDoubleIndexer_ = NULL;
    }
    if (pBTreeUStrIndexer_)
    {
        delete pBTreeUStrIndexer_;
        pBTreeUStrIndexer_ = NULL;
    }
/*	
    if (pBTreeUStrSuffixIndexer_)
    {
        delete pBTreeUStrSuffixIndexer_;
        pBTreeUStrSuffixIndexer_ = NULL;
    }
*/	
}

void BTreeIndexer::add(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid)
{
    izenelib::util::boost_variant_visit(boost::bind(add_visitor(), this, colID, fid, _1, docid), value);
}

void BTreeIndexer::remove(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid)
{
    izenelib::util::boost_variant_visit(boost::bind(remove_visitor(), this, colID, fid, _1, docid), value);
}

void BTreeIndexer::getValue(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_visitor(), this, colID, fid, _1, boost::ref(docs)), value);
}

void BTreeIndexer::getValueNotEqual(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_without_visitor(), this, colID, fid, _1, boost::ref(docs)), value);
}

void BTreeIndexer::getValueBetween(collectionid_t colID, fieldid_t fid, PropertyType& value1, PropertyType& value2, BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_between_visitor(), this, colID, fid, _1, _2, boost::ref(docs)),value1,value2);	
}

void BTreeIndexer::getValueLess(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_less_visitor(), this, colID, fid, _1, boost::ref(docs)), value);
}

void BTreeIndexer::getValueLessEqual(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_less_equal_visitor(), this, colID, fid, _1, boost::ref(docs)), value);
}

void BTreeIndexer::getValueGreat(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_great_visitor(), this, colID, fid, _1, boost::ref(docs)), value);
}

void BTreeIndexer::getValueGreatEqual(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_great_equal_visitor(), this, colID, fid, _1, boost::ref(docs)), value);
}

void BTreeIndexer::getValueIn(collectionid_t colID, fieldid_t fid, vector<PropertyType>& values,BitVector& docs)
{
    for (size_t i = 0; i < values.size(); i++)
        izenelib::util::boost_variant_visit(boost::bind(get_visitor(), this, colID, fid, _1, boost::ref(docs)), values[i]);
}

void BTreeIndexer::getValueNotIn(collectionid_t colID, fieldid_t fid, vector<PropertyType>& values,BitVector& docs)
{
    for (size_t i = 0; i < values.size(); i++)
        getValue(colID, fid, values[i], docs);
    docs.toggle();
}

void BTreeIndexer::getValueStart(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    try
    {
        IndexKeyType<String> key(colID,fid,boost::get<String>(value));
        pBTreeUStrIndexer_->getPrefix(key,docs);
    }
    catch (...)
    {
        SF1V5_THROW(ERROR_UNSUPPORTED,"unsupported operation");
    }
}

void BTreeIndexer::getValueEnd(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    try
    {
        //pBTreeUStrSuffixIndexer_->getValueSuffix(boost::get<String>(value),fid, docs);
        IndexKeyType<String> key(colID,fid,boost::get<String>(value));
        pBTreeUStrIndexer_->getSuffix(key,docs);
    }
    catch (...)
    {
        SF1V5_THROW(ERROR_UNSUPPORTED,"unsupported operation");
    }
}

void BTreeIndexer::getValueSubString(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    try
    {
        //pBTreeUStrSuffixIndexer_->getValuePrefix(boost::get<String>(value), fid, docs);
        IndexKeyType<String> key(colID,fid,boost::get<String>(value));
        pBTreeUStrIndexer_->getSubString(key,docs);
    }
    catch (...)
    {
        SF1V5_THROW(ERROR_UNSUPPORTED,"unsupported operation");
    }
}

void BTreeIndexer::flush()
{
    if (pBTreeIntIndexer_) pBTreeIntIndexer_->commit();
    if (pBTreeUIntIndexer_) pBTreeUIntIndexer_->commit();
    if (pBTreeFloatIndexer_) pBTreeFloatIndexer_->commit();
    if (pBTreeDoubleIndexer_) pBTreeDoubleIndexer_->commit();
    if (pBTreeUStrIndexer_) pBTreeUStrIndexer_->commit();
    //if (pBTreeUStrSuffixIndexer_) pBTreeUStrSuffixIndexer_->flush();
}

