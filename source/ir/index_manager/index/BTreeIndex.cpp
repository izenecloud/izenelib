#include <boost/any.hpp>

#include <ir/index_manager/index/BTreeIndex.h>

using namespace std;
using namespace wiselib;

using namespace izenelib::ir::indexmanager;

BTreeIndex<IndexKeyType<int> >* BTreeIndexer::pBTreeIntIndexer_;

BTreeIndex<IndexKeyType<unsigned int> >* BTreeIndexer::pBTreeUIntIndexer_;

BTreeIndex<IndexKeyType<float> >* BTreeIndexer::pBTreeFloatIndexer_;

BTreeIndex<IndexKeyType<double> >* BTreeIndexer::pBTreeDoubleIndexer_;

BTreeIndex<IndexKeyType<wiselib::UString> >* BTreeIndexer::pBTreeUStrIndexer_;

izenelib::sdb::IndexSDB<USuffix, wiselib::UString>* BTreeIndexer::pBTreeUStrSuffixIndexer_;

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
    pBTreeUStrIndexer_ = new BTreeIndex<IndexKeyType<wiselib::UString> >(path);
    pBTreeUStrIndexer_->initialize(maxDataSize/10, degree, maxDataSize, cacheSize);

    path.clear();
    path = location+"/usuf.bti";
    pBTreeUStrSuffixIndexer_ = new izenelib::sdb::IndexSDB<USuffix, UString>( path);
    pBTreeUStrSuffixIndexer_->initialize(maxDataSize/10, degree, maxDataSize*2,cacheSize);

}

BTreeIndexer::~BTreeIndexer()
{
    flush();

    delete pBTreeIntIndexer_;
    delete pBTreeUIntIndexer_;
    delete pBTreeFloatIndexer_;
    delete pBTreeDoubleIndexer_;
    delete pBTreeUStrIndexer_;
    delete pBTreeUStrSuffixIndexer_;
}

void BTreeIndexer::add(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid)
{
    izenelib::util::boost_variant_visit(boost::bind(add_visitor(), colID, fid, _1, docid), value);
}

void BTreeIndexer::remove(collectionid_t colID, fieldid_t fid, PropertyType& value, docid_t docid)
{
    izenelib::util::boost_variant_visit(boost::bind(remove_visitor(), colID, fid, _1, docid), value);
}

void BTreeIndexer::getValue(collectionid_t colID, fieldid_t fid, PropertyType& value,vector<docid_t>& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_visitor(), colID, fid, _1, docs), value);
}

void BTreeIndexer::getValueBetween(collectionid_t colID, fieldid_t fid, PropertyType& value1, PropertyType& value2, vector<docid_t>& docs)
{
    //izenelib::util::boost_variant_visit(boost::bind(get_between_visitor(), colID, fid, _1, _2, docs), value1, value2);
    boost::bind(get_between_visitor<PropertyType>(), colID, fid, value1, value2, docs);
}

void BTreeIndexer::getValueLess(collectionid_t colID, fieldid_t fid, PropertyType& value,vector<docid_t>& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_less_visitor(), colID, fid, _1, docs), value);
    uniqueResults(docs);
}

void BTreeIndexer::getValueLessEqual(collectionid_t colID, fieldid_t fid, PropertyType& value,vector<docid_t>& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_less_equal_visitor(), colID, fid, _1, docs), value);
    uniqueResults(docs);
}

void BTreeIndexer::getValueGreat(collectionid_t colID, fieldid_t fid, PropertyType& value,vector<docid_t>& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_great_visitor(), colID, fid, _1, docs), value);
    uniqueResults(docs);
}

void BTreeIndexer::getValueGreatEqual(collectionid_t colID, fieldid_t fid, PropertyType& value,vector<docid_t>& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_great_equal_visitor(), colID, fid, _1, docs), value);
    uniqueResults(docs);
}

void BTreeIndexer::getValueIn(collectionid_t colID, fieldid_t fid, vector<PropertyType>& values,vector<docid_t>& docs)
{
    for (size_t i = 0; i < values.size(); i++)
        izenelib::util::boost_variant_visit(boost::bind(get_visitor(), colID, fid, _1, docs), values[i]);
    uniqueResults(docs);
}

void BTreeIndexer::getValueNotIn(collectionid_t colID, fieldid_t fid, vector<PropertyType>& values,vector<docid_t>& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_great_visitor(), colID, fid, _1, docs), values[0]);
    izenelib::util::boost_variant_visit(boost::bind(get_less_visitor(), colID, fid, _1, docs), values[0]);

    stable_sort(docs.begin(),docs.end());

    vector<docid_t> docs__;
    for (size_t i = 1; i < values.size(); i++)
        izenelib::util::boost_variant_visit(boost::bind(get_visitor(), colID, fid, _1, docs__), values[i]);

    sort(docs__.begin(),docs__.end());
    vector<docid_t>::iterator docIt = unique(docs__.begin(),docs__.end());
    docs__.resize(docIt - docs__.begin());

    size_t docSize = docs__.size();
    for (size_t i = 0; i < docSize; i++)
    {
        docIt = find(docs.begin(),docs.end(),docs__[i]);
        docs.erase(docIt);
    }
}

void BTreeIndexer::getValueNotEqual(collectionid_t colID, fieldid_t fid, PropertyType& value,vector<docid_t>& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(get_great_visitor(), colID, fid, _1, docs), value);
    izenelib::util::boost_variant_visit(boost::bind(get_less_visitor(), colID, fid, _1, docs), value);
    uniqueResults(docs);
}

void BTreeIndexer::getValueStart(collectionid_t colID, fieldid_t fid, PropertyType& value,vector<docid_t>& docs)
{
    try
    {
        IndexKeyType<wiselib::UString> key(colID,fid,boost::get<wiselib::UString>(value));
        pBTreeUStrIndexer_->getValuePrefix(key,docs);
    }catch (...)
    {
        SF1V5_THROW(ERROR_UNSUPPORTED,"unsupported operation");
    }
}

void BTreeIndexer::getValueEnd(collectionid_t colID, fieldid_t fid, PropertyType& value,vector<docid_t>& docs)
{
    try
    {
	vector<wiselib::UString> vkey;
	pBTreeUStrSuffixIndexer_->getValue(boost::get<wiselib::UString>(value), vkey);
	
	vector<IndexKeyType<wiselib::UString> > keys;
	for (size_t i=0; i<vkey.size(); i++)
		keys.push_back(IndexKeyType<wiselib::UString>(colID, fid, vkey[i]) );
	
	pBTreeUStrIndexer_->getValueIn(keys, docs);

	uniqueResults(docs);

    }catch (...)
    {
        SF1V5_THROW(ERROR_UNSUPPORTED,"unsupported operation");
    }
}

void BTreeIndexer::getValueSubString(collectionid_t colID, fieldid_t fid, PropertyType& value,vector<docid_t>& docs)
{
    try
    {
	vector<wiselib::UString> vkey;
	pBTreeUStrSuffixIndexer_->getValuePrefix(boost::get<wiselib::UString>(value), vkey);
	
	vector<IndexKeyType<wiselib::UString> > keys;
	for (size_t i=0; i<vkey.size(); i++)
		keys.push_back(IndexKeyType<wiselib::UString>(colID, fid, vkey[i]) );
	
	pBTreeUStrIndexer_->getValueIn(keys, docs);

	uniqueResults(docs);

    }catch (...)
    {
        SF1V5_THROW(ERROR_UNSUPPORTED,"unsupported operation");
    }
}

void BTreeIndexer::flush()
{
    pBTreeIntIndexer_->commit();
    pBTreeUIntIndexer_->commit();
    pBTreeFloatIndexer_->commit();
    pBTreeDoubleIndexer_->commit();
    pBTreeUStrIndexer_->commit();
    pBTreeUStrSuffixIndexer_->commit();
}


