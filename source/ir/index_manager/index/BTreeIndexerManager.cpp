#include <ir/index_manager/index/rtype/BTreeIndexerManager.h>

using namespace izenelib::ir::indexmanager;

bool BTreeIndexerManager::seek(const std::string& property_name, const PropertyType& key)
{
    bool find = false;
    izenelib::util::boost_variant_visit(boost::bind(mseek_visitor(), this, property_name, _1, boost::ref(find)), key);
    return find;
}

void BTreeIndexerManager::getNoneEmptyList(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    getValue(property_name, key, docs);
}

void BTreeIndexerManager::add(const std::string& property_name, const PropertyType& key, docid_t docid, bool isUpdate)
{
    if(!isUpdate)
    {
        izenelib::util::boost_variant_visit(boost::bind(madd_visitor(), this, property_name, _1, docid), key);
    }
    else
    {
        izenelib::util::boost_variant_visit(boost::bind(maddu_visitor(), this, property_name, _1, docid), key);
    }
}

void BTreeIndexerManager::remove(const std::string& property_name, const PropertyType& key, docid_t docid)
{
    izenelib::util::boost_variant_visit(boost::bind(mremove_visitor(), this, property_name, _1, docid), key);
}

void BTreeIndexerManager::getValue(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(mget_visitor(), this, property_name, _1, boost::ref(docs)), key);
}

void BTreeIndexerManager::getValue(const std::string& property_name, const PropertyType& key, std::vector<docid_t>& docList)
{
    BitVector docmap;
    getValue(property_name, key, docmap);
    for(std::size_t docid=1;docid<docmap.size();docid++)
    {
        if(docmap.test(docid))
        {
            docList.push_back(docid);
        }
    }
}
