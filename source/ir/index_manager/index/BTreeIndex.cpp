#include <boost/any.hpp>

#include <ir/index_manager/index/BTreeIndex.h>

using namespace std;
using namespace wiselib;

using namespace izenelib::ir::indexmanager;

BTreeIndex<IndexKeyType<int> >* BTreeIndexer::pBTreeIntIndexer_ = NULL;

BTreeIndex<IndexKeyType<unsigned int> >* BTreeIndexer::pBTreeUIntIndexer_  = NULL;

BTreeIndex<IndexKeyType<float> >* BTreeIndexer::pBTreeFloatIndexer_  = NULL;

BTreeIndex<IndexKeyType<double> >* BTreeIndexer::pBTreeDoubleIndexer_  = NULL;

BTreeIndex<IndexKeyType<String> >* BTreeIndexer::pBTreeUStrIndexer_  = NULL;

izenelib::sdb::TrieIndexSDB2<String, String>* BTreeIndexer::pBTreeUStrSuffixIndexer_  = NULL;

BTreeIndexer::BTreeIndexer(string location, int degree, size_t cacheSize, size_t maxDataSize)
{
    string path(location);
    path.append("/int.bti");
    pBTreeIntIndexer_ = new BTreeIndex<IndexKeyType<int> >(path);
    pBTreeIntIndexer_->initialize(maxDataSize/10, degree, maxDataSize, cacheSize);

    path.clear();
    path = location+"/uint.bti";
    pBTreeUIntIndexer_ = new BTreeIndex<IndexKeyType<unsigned int> >(path);
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

    path.clear();
    path = location+"/usuf.bti";
    pBTreeUStrSuffixIndexer_ = new izenelib::sdb::TrieIndexSDB2<String, String>( path);
    pBTreeUStrSuffixIndexer_->open();
}

BTreeIndexer::~BTreeIndexer()
{
    flush();

    if (pBTreeIntIndexer_) {
        delete pBTreeIntIndexer_;
        pBTreeIntIndexer_ = NULL;
    }
    if (pBTreeUIntIndexer_) {
        delete pBTreeUIntIndexer_;
        pBTreeUIntIndexer_ = NULL;
    }
    if (pBTreeFloatIndexer_) {
        delete pBTreeFloatIndexer_;
        pBTreeFloatIndexer_ = NULL;
    }
    if (pBTreeDoubleIndexer_) {
        delete pBTreeDoubleIndexer_;
        pBTreeDoubleIndexer_ = NULL;
    }
    if (pBTreeUStrIndexer_) {
        delete pBTreeUStrIndexer_;
        pBTreeUStrIndexer_ = NULL;
    }
    if (pBTreeUStrSuffixIndexer_) {
        delete pBTreeUStrSuffixIndexer_;
        pBTreeUStrSuffixIndexer_ = NULL;
    }
}

void BTreeIndexer::add(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid)
{
    izenelib::util::boost_variant_visit(boost::bind(add_visitor(), colID, fid, _1, docid), value);
}

void BTreeIndexer::remove(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid)
{
    izenelib::util::boost_variant_visit(boost::bind(remove_visitor(), colID, fid, _1, docid), value);
}

void BTreeIndexer::getValue(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_visitor(), colID, fid, _1, docs), value);
}

void BTreeIndexer::getValueNotEqual(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_without_visitor(), colID, fid, _1, docs), value);
}

void BTreeIndexer::getValueBetween(collectionid_t colID, fieldid_t fid, PropertyType& value1, PropertyType& value2, BitVector& docs)
{
    boost::bind(get_between_visitor<PropertyType>(), colID, fid, value1, value2, docs);
}

void BTreeIndexer::getValueLess(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_less_visitor(), colID, fid, _1, docs), value);
}

void BTreeIndexer::getValueLessEqual(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_less_equal_visitor(), colID, fid, _1, docs), value);
}

void BTreeIndexer::getValueGreat(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_great_visitor(), colID, fid, _1, docs), value);
}

void BTreeIndexer::getValueGreatEqual(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_great_equal_visitor(), colID, fid, _1, docs), value);
}

void BTreeIndexer::getValueIn(collectionid_t colID, fieldid_t fid, vector<PropertyType>& values,BitVector& docs)
{
    for (size_t i = 0; i < values.size(); i++)
        izenelib::util::boost_variant_visit(boost::bind(get_visitor(), colID, fid, _1, docs), values[i]);
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
    } catch (...)
    {
        SF1V5_THROW(ERROR_UNSUPPORTED,"unsupported operation");
    }
}

void BTreeIndexer::getValueEnd(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    try
    {
        vector<String> vkey;
        pBTreeUStrSuffixIndexer_->getValueSuffix(boost::get<String>(value), vkey);

        vector<IndexKeyType<String> > keys;
        for (size_t i=0; i<vkey.size(); i++)
            pBTreeUStrIndexer_->get(IndexKeyType<String>(colID, fid, vkey[i]),docs);
    } catch (...)
    {
        SF1V5_THROW(ERROR_UNSUPPORTED,"unsupported operation");
    }
}

void BTreeIndexer::getValueSubString(collectionid_t colID, fieldid_t fid, PropertyType& value,BitVector& docs)
{
    try
    {
        vector<String> vkey;
        pBTreeUStrSuffixIndexer_->getValuePrefix(boost::get<String>(value), vkey);

        vector<IndexKeyType<String> > keys;
        for (size_t i=0; i<vkey.size(); i++)
            pBTreeUStrIndexer_->get(IndexKeyType<String>(colID, fid, vkey[i]),docs);
    } catch (...)
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
    if (pBTreeUStrSuffixIndexer_) pBTreeUStrSuffixIndexer_->commit();
}

