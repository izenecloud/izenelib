#include <ir/index_manager/index/rtype/BTreeIndexerManager.h>
#include <am/sequence_file/ssfr.h>
#include <boost/serialization/variant.hpp>

NS_IZENELIB_IR_BEGIN
namespace indexmanager{

template <>
std::size_t BTreeIndexer<String>::convertAllValue(std::size_t maxDoc, uint32_t* & data)
{
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
    std::size_t result = 0;

    String lowKey;
    std::auto_ptr<BaseEnumType> term_enum(getEnum_(lowKey));
    std::pair<String, ValueType> kvp;
    docid_t docid = 0;
    while(term_enum->next(kvp))
    {
        for(uint32_t i=0;i<kvp.second.size();i++)
        {
            docid = kvp.second[i];
            if (docid >= maxDoc) break;
            data[docid] = kvp.first.empty() ? 0 : 1;
            ++result;
        }
    }
    return result;
}


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
    if (pFilter_)
    {
        docs.logicalNotAnd(*pFilter_);
    }
}

bool BTreeIndexerManager::checkPropertyName_(const std::string& propertyName)
{
    boost::unordered_map<std::string, PropertyType>::iterator it = type_map_.find(propertyName);
    if(it==type_map_.end())
    {
        LOG(ERROR)<<"Property name "<<propertyName<<" does not exists."<<std::endl;
        return false;
    }
    return true;
}

bool BTreeIndexerManager::checkType_(const std::string& propertyName, const PropertyType& value)
{
    boost::unordered_map<std::string, PropertyType>::iterator it = type_map_.find(propertyName);
    if(it==type_map_.end())
    {
        LOG(ERROR)<<"Property name "<<propertyName<<" does not exists."<<std::endl;
        return false;
    }
    if(value.which() != it->second.which() )
    {
        LOG(ERROR)<<"Property "<<propertyName<<" expecting which()="<<it->second.which()<<", but get "<<value.which()<<std::endl;
        return false;
    }
    return true;
}

void BTreeIndexerManager::add(const std::string& property_name, const PropertyType& key, docid_t docid)
{
    if(!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(madd_visitor(), this, property_name, _1, docid), key);
}

void BTreeIndexerManager::remove(const std::string& property_name, const PropertyType& key, docid_t docid)
{
    if(!checkType_(property_name, key)) return;
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
    if(!checkPropertyName_(property_name)) return 0;
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
    if(!checkType_(property_name, key)) return false;
    bool find = false;
    izenelib::util::boost_variant_visit(boost::bind(mseek_visitor(), this, property_name, _1, boost::ref(find)), key);
    return find;
}

void BTreeIndexerManager::getNoneEmptyList(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    if(!checkType_(property_name, key)) return;
    getValue(property_name, key, docs);
}

void BTreeIndexerManager::getValue(const std::string& property_name, const PropertyType& key, BitVector& docs, bool needFilter)
{
    if(!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(mget_visitor(), this, property_name, _1, boost::ref(docs)), key);
    if (needFilter)
    {
        doFilter_(docs);
    }
}

void BTreeIndexerManager::getValue(const std::string& property_name, const PropertyType& key, std::vector<docid_t>& docList)
{
    if(!checkType_(property_name, key)) return;
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
    if(!checkType_(property_name, key1)) return;
    if(!checkType_(property_name, key2)) return;
    izenelib::util::boost_variant_visit(boost::bind(mbetween_visitor(), this, property_name, _1, _2, boost::ref(docs)), key1, key2);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueLess(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    if(!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(mless_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueLessEqual(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    if(!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(mless_equal_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueGreat(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    if(!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(mgreat_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueGreatEqual(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    if(!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(mgreat_equal_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueIn(const std::string& property_name, const std::vector<PropertyType>& keys, BitVector& docs, bool needFilter)
{
    for(std::size_t i=0;i<keys.size();i++)
    {
        if(!checkType_(property_name, keys[i]))
            return;
    }

    for(std::size_t i=0;i<keys.size();i++)
    {
        izenelib::util::boost_variant_visit(boost::bind(mget_visitor(), this, property_name, _1, boost::ref(docs)), keys[i]);
    }

    if (needFilter)
    {
        doFilter_(docs);
    }
}

void BTreeIndexerManager::getValueNotIn(const std::string& property_name, const std::vector<PropertyType>& keys, BitVector& docs)
{
    for(std::size_t i=0;i<keys.size();i++)
    {
        if(!checkType_(property_name, keys[i])) return;
    }
    getValueIn(property_name, keys, docs, false);
    docs.toggle();
    docs.clear(0); // clear docid 0 after toggle
    doFilter_(docs);
}

void BTreeIndexerManager::getValueNotEqual(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    if(!checkType_(property_name, key)) return;
    getValue(property_name, key, docs);
    docs.toggle();
    docs.clear(0); // clear docid 0 after toggle
    doFilter_(docs);
}

void BTreeIndexerManager::getValueStart(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    if(!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(mstart_equal_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueEnd(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    if(!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(mend_equal_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueSubString(const std::string& property_name, const PropertyType& key, BitVector& docs)
{
    if(!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(msub_string_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}


}

NS_IZENELIB_IR_END
