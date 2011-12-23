#include <ir/index_manager/index/rtype/BTreeIndexerManager.h>
#include <am/sequence_file/ssfr.h>
#include <boost/serialization/variant.hpp>

using namespace izenelib::ir::indexmanager;

BTreeIndexerManager::BTreeIndexerManager(const std::string& dir, Directory* pDirectory, const std::map<std::string, PropertyType>& type_map)
:dir_(dir), pDirectory_(pDirectory)
{
  for( std::map<std::string, PropertyType>::const_iterator it = type_map.begin(); it!=type_map.end(); ++it)
  {
//       LOG(INFO)<<"add map "<<it->first<<" , "<<it->second.which()<<std::endl;
      type_map_.insert(*it);
  }
}

BTreeIndexerManager::~BTreeIndexerManager()
{
    flush();
    boost::unordered_map<std::string, PropertyType>::iterator it = type_map_.begin();
    while(it!=type_map_.end())
    {
        izenelib::util::boost_variant_visit(boost::bind(mdelete_visitor(), this, it->first, _1), it->second);
        ++it;
    }
}

void BTreeIndexerManager::doFilter_(BitVector& docs)
{
    if ( pFilter_&& pFilter_->any() )
    {
        docs.logicalNotAnd(*pFilter_);
    }
}


void BTreeIndexerManager::add(const std::string& property_name, const PropertyType& key, docid_t docid)
{
    izenelib::util::boost_variant_visit(boost::bind(madd_visitor(), this, property_name, _1, docid), key);
}

void BTreeIndexerManager::remove(const std::string& property_name, const PropertyType& key, docid_t docid)
{
    izenelib::util::boost_variant_visit(boost::bind(mremove_visitor(), this, property_name, _1, docid), key);
}


void BTreeIndexerManager::flush()
{
    if(pFilter_ && pFilter_->any())
    {
        pFilter_->write(pDirectory_, BTREE_DELETED_DOCS);
    }
    boost::unordered_map<std::string, PropertyType>::iterator it = type_map_.begin();
    while(it!=type_map_.end())
    {
//         LOG(INFO)<<"Flushing property : "<<it->first<<" , "<<it->second.which()<<std::endl;
        izenelib::util::boost_variant_visit(boost::bind(mflush_visitor(), this, it->first, _1), it->second);
        ++it;
    }
//     LOG(INFO)<<"Flushed all properties."<<std::endl;
}

std::size_t BTreeIndexerManager::count(const std::string& property_name)
{
    std::size_t count = 0;
    boost::unordered_map<std::string, PropertyType>::const_iterator it = type_map_.find(property_name);
    if(it!=type_map_.end())
    {
        izenelib::util::boost_variant_visit(boost::bind(mcount_visitor(), this, property_name, _1, boost::ref(count)), it->second);
    }
    return count;
    
}

bool BTreeIndexerManager::seek(const std::string& property_name, const PropertyType& key)
{
    bool find = false;
    izenelib::util::boost_variant_visit(boost::bind(mseek_visitor(), this, property_name, _1, boost::ref(find)), key);
    return find;
}

void BTreeIndexerManager::getNoneEmptyList(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    getValue(property_name, key, docs);
    doFilter_(docs);
}

void BTreeIndexerManager::getValue(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(mget_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValue(const std::string& property_name, const PropertyType& key, std::vector<docid_t>& docList)
{
    izenelib::util::boost_variant_visit(boost::bind(mget2_visitor(), this, property_name, _1, boost::ref(docList)), key);
    if (pFilter_)
    {
        std::vector<docid_t> tmpIdList;
        for(size_t i = 0; i < docList.size(); i++)
        {
            if(!pFilter_->test(docList[i]))
                tmpIdList.push_back(docList[i]);
        }
        docList.swap(tmpIdList);
    }
}

void BTreeIndexerManager::getValueBetween(const std::string& property_name, const PropertyType& key1, const PropertyType& key2, BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(mbetween_visitor(), this, property_name, _1, _2, boost::ref(docs)), key1, key2);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueLess(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(mless_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
    
}

void BTreeIndexerManager::getValueLessEqual(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(mless_equal_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueGreat(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(mgreat_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueGreatEqual(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(mgreat_equal_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueIn(const std::string& property_name, const std::vector<PropertyType>& keys, BitVector& docs)
{
    for(std::size_t i=0;i<keys.size();i++)
    {
        BitVector bv;
        getValue(property_name, keys[i], bv);
        docs |= bv;
    }
}

void BTreeIndexerManager::getValueNotIn(const std::string& property_name, const std::vector<PropertyType>& keys, BitVector& docs)
{
    getValueIn(property_name, keys, docs);
    docs.toggle();
    doFilter_(docs);
}

void BTreeIndexerManager::getValueNotEqual(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    getValue(property_name, key, docs);
    docs.toggle();
    doFilter_(docs);
}

void BTreeIndexerManager::getValueStart(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(mstart_equal_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueEnd(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(mend_equal_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueSubString(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    izenelib::util::boost_variant_visit(boost::bind(msub_string_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

