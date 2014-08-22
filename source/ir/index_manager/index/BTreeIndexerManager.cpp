#include <ir/index_manager/index/rtype/BTreeIndexerManager.h>
#include <am/sequence_file/ssfr.h>
#include <boost/serialization/variant.hpp>

NS_IZENELIB_IR_BEGIN

namespace indexmanager
{

BTreeIndexerManager::BTreeIndexerManager(const std::string& dir, Directory* pDirectory,
    const std::map<std::string, PropertyType>& type_map,
    const std::set<std::string>& usePerformanceProperty,
    const std::set<std::string>& no_preload_props)
:dir_(dir), pDirectory_(pDirectory), use_per_props_(usePerformanceProperty), no_preload_props_(no_preload_props)
{
  for (std::map<std::string, PropertyType>::const_iterator it = type_map.begin(); it!=type_map.end(); ++it)
  {
//       LOG(INFO)<<"add map "<<it->first<<" , "<<it->second.which()<<std::endl;
      type_map_.insert(*it);
  }
  LOG(INFO) << "usePerformanceProperty SIZE:" <<usePerformanceProperty.size();
}

BTreeIndexerManager::~BTreeIndexerManager()
{
    flush();
    boost::unordered_map<std::string, PropertyType>::iterator it = type_map_.begin();
    while (it!=type_map_.end())
    {
        izenelib::util::boost_variant_visit(boost::bind(mdelete_visitor(), this, it->first, _1), it->second);
        ++it;
    }
}

void BTreeIndexerManager::doFilter_(Bitset& docs)
{
    if (pFilter_)
    {
        docs -= *pFilter_;
    }
}

bool BTreeIndexerManager::checkPropertyName_(const std::string& propertyName)
{
    boost::unordered_map<std::string, PropertyType>::iterator it = type_map_.find(propertyName);
    if (it==type_map_.end())
    {
        LOG(ERROR)<<"Property name "<<propertyName<<" does not exists."<<std::endl;
        return false;
    }
    return true;
}

bool BTreeIndexerManager::checkType_(const std::string& propertyName, const PropertyType& value)
{
    boost::unordered_map<std::string, PropertyType>::iterator it = type_map_.find(propertyName);
    if (it==type_map_.end())
    {
        LOG(ERROR)<<"Property name "<<propertyName<<" does not exists."<<std::endl;
        return false;
    }
    if (value.which() != it->second.which() )
    {
        LOG(ERROR)<<"Property "<<propertyName<<" expecting which()="<<it->second.which()<<", but get "<<value.which()<<std::endl;
        return false;
    }
    return true;
}

void BTreeIndexerManager::usePreLoadRang(const std::string& property_name)
{
    std::cout <<"usePreLoadRang ...." << std::endl;
    BTreeIndexer<float>* pindexer = this->getIndexer<float>(property_name);
    pindexer->setPreLoadGreatEqual();
}

void BTreeIndexerManager::add(const std::string& property_name, const PropertyType& key, docid_t docid)
{
    if (!checkType_(property_name, key)) return;

    if (use_per_props_.find(property_name) != use_per_props_.end())
    //if (property_name == "Price")
    {
        float value = boost::get<float>(key);
        int32_t value_convert = (int32_t)value;
        PropertyType convert_key = (float)value_convert;
        izenelib::util::boost_variant_visit(boost::bind(madd_visitor(), this, property_name, _1, docid), convert_key);
    }
    else
        izenelib::util::boost_variant_visit(boost::bind(madd_visitor(), this, property_name, _1, docid), key);
}

void BTreeIndexerManager::remove(const std::string& property_name, const PropertyType& key, docid_t docid)
{
    if (!checkType_(property_name, key)) return;

    if (use_per_props_.find(property_name) != use_per_props_.end())
    //if (property_name == "Price")
    {
        float value =  boost::get<float>(key);
        int32_t value_convert = (int32_t)value;
        PropertyType convert_key = (float)value_convert;
        izenelib::util::boost_variant_visit(boost::bind(mremove_visitor(), this, property_name, _1, docid), convert_key);
    }
    else
        izenelib::util::boost_variant_visit(boost::bind(mremove_visitor(), this, property_name, _1, docid), key);
}


void BTreeIndexerManager::flush()
{
    if (pFilter_ && pFilter_->any())
    {
        pFilter_->write(pDirectory_, BTREE_DELETED_DOCS);
    }
    boost::unordered_map<std::string, PropertyType>::iterator it = type_map_.begin();
    while (it!=type_map_.end())
    {
//         LOG(INFO)<<"Flushing property : "<<it->first<<" , "<<it->second.which()<<std::endl;
        izenelib::util::boost_variant_visit(boost::bind(mflush_visitor(), this, it->first, _1), it->second);
        ++it;
    }
//     LOG(INFO)<<"Flushed all properties."<<std::endl;
}

std::size_t BTreeIndexerManager::count(const std::string& property_name)
{
    if (!checkPropertyName_(property_name)) return 0;
    std::size_t count = 0;
    boost::unordered_map<std::string, PropertyType>::const_iterator it = type_map_.find(property_name);
    if (it!=type_map_.end())
    {
        izenelib::util::boost_variant_visit(boost::bind(mcount_visitor(), this, property_name, _1, boost::ref(count)), it->second);
    }
    return count;

}

bool BTreeIndexerManager::seek(const std::string& property_name, const PropertyType& key)
{
    if (!checkType_(property_name, key)) return false;
    bool find = false;
    izenelib::util::boost_variant_visit(boost::bind(mseek_visitor(), this, property_name, _1, boost::ref(find)), key);
    return find;
}

void BTreeIndexerManager::getNoneEmptyList(const std::string& property_name, const PropertyType& key, Bitset& docs)
{
    if (!checkType_(property_name, key)) return;
    getValue(property_name, key, docs);
}

void BTreeIndexerManager::getValue(const std::string& property_name, const PropertyType& key, Bitset& docs, bool needFilter)
{
    if (!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(mget_visitor(), this, property_name, _1, boost::ref(docs)), key);
    if (needFilter)
    {
        doFilter_(docs);
    }
}

void BTreeIndexerManager::getValue(const std::string& property_name, const PropertyType& key, ValueType& docList)
{
    if (!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(mget2_visitor(), this, property_name, _1, boost::ref(docList)), key);
    if (pFilter_)
    {
        if (docList.which() == 0)
        {
            DocListType tmpIdList;
            DocListType& rawdata = boost::get<DocListType>(docList);
            for (size_t i = 0; i < rawdata.size(); i++)
            {
                if (!pFilter_->test(rawdata[i]))
                    tmpIdList.push_back(rawdata[i]);
            }
            rawdata.swap(tmpIdList);
        }
        else
        {
            doFilter_(boost::get<Bitset>(docList));
        }
    }
}

void BTreeIndexerManager::getValueBetween(const std::string& property_name, const PropertyType& key1, const PropertyType& key2, Bitset& docs)
{
    if (!checkType_(property_name, key1)) return;
    if (!checkType_(property_name, key2)) return;
    izenelib::util::boost_variant_visit(boost::bind(mbetween_visitor(), this, property_name, _1, _2, boost::ref(docs)), key1, key2);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueLess(const std::string& property_name, const PropertyType& key, Bitset& docs)
{
    if (!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(mless_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueLessEqual(const std::string& property_name, const PropertyType& key, Bitset& docs)
{
    if (!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(mless_equal_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueGreat(const std::string& property_name, const PropertyType& key, Bitset& docs)
{
    if (!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(mgreat_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueGreatEqual(const std::string& property_name, const PropertyType& key, Bitset& docs)
{
    if (!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(mgreat_equal_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueIn(const std::string& property_name, const std::vector<PropertyType>& keys, Bitset& docs, bool needFilter)
{
    for (std::size_t i=0;i<keys.size();i++)
    {
        if (!checkType_(property_name, keys[i]))
            return;
    }

    for (std::size_t i=0;i<keys.size();i++)
    {
        izenelib::util::boost_variant_visit(boost::bind(mget_visitor(), this, property_name, _1, boost::ref(docs)), keys[i]);
    }

    if (needFilter)
    {
        doFilter_(docs);
    }
}

void BTreeIndexerManager::getValueNotIn(const std::string& property_name, const std::vector<PropertyType>& keys, Bitset& docs)
{
    for (std::size_t i=0;i<keys.size();i++)
    {
        if (!checkType_(property_name, keys[i])) return;
    }
    getValueIn(property_name, keys, docs, false);
    docs.flip();
    docs.reset(0); // reset docid 0 after flip
    doFilter_(docs);
}

void BTreeIndexerManager::getValueNotEqual(const std::string& property_name, const PropertyType& key, Bitset& docs)
{
    if (!checkType_(property_name, key)) return;
    getValue(property_name, key, docs);
    docs.flip();
    docs.reset(0); // reset docid 0 after flip
    doFilter_(docs);
}

void BTreeIndexerManager::getValueStart(const std::string& property_name, const PropertyType& key, Bitset& docs)
{
    if (!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(mstart_equal_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueEnd(const std::string& property_name, const PropertyType& key, Bitset& docs)
{
    if (!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(mend_equal_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValueSubString(const std::string& property_name, const PropertyType& key, Bitset& docs)
{
    if (!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(msub_string_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

void BTreeIndexerManager::getValuePGS(const std::string& property_name, const PropertyType& key, Bitset& docs)
{
    if (!checkType_(property_name, key)) return;
    izenelib::util::boost_variant_visit(boost::bind(mpgs_visitor(), this, property_name, _1, boost::ref(docs)), key);
    doFilter_(docs);
}

}

NS_IZENELIB_IR_END
