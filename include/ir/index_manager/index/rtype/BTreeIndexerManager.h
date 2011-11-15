
#ifndef IZENELIB_IR_BTREEINDEXERMANAGER_H
#define IZENELIB_IR_BTREEINDEXERMANAGER_H

#include "BTreeIndexer.h"

#include <boost/variant.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/not.hpp>
#include <boost/bind.hpp>
#include <boost/unordered_map.hpp>
#include <functional>

#include <algorithm>
#include <string>

NS_IZENELIB_IR_BEGIN


namespace indexmanager
{
   


/**
 * BTreeIndexer
 * @brief BTreeIndexer is a wrapper which takes charges of managing all b-tree handlers.
 */

class BTreeIndexerManager
{
    
    
public:
    BTreeIndexerManager(const std::string& dir)
    :dir_(dir)
    {
    }

    ~BTreeIndexerManager()
    {
    }
public:
    
    template<class T>
    CBTreeIndexer<T>* getIndexer(const std::string& property_name)
    {
        CBTreeIndexer<T>* result = NULL;
        boost::unordered_map<std::string, void*>::iterator it = instance_map_.find(property_name);
        if(it==instance_map_.end())
        {
            result = new CBTreeIndexer<T>(dir_+"/property."+property_name, property_name);
            result->open();
            instance_map_.insert(std::make_pair( property_name, (void*)result) );
        }
        else
        {
            void* pv = it->second;
            result = (CBTreeIndexer<T>*)pv;
        }
        return result;
    }
    
    bool seek(const std::string& property_name, const PropertyType& key);

    void getNoneEmptyList(const std::string& property_name, const PropertyType& key, BitVector& docs);

    void add(const std::string& property_name, const PropertyType& key, docid_t docid, bool isUpdate = false);

    void remove(const std::string& property_name, const PropertyType& key, docid_t docid);

    void getValue(const std::string& property_name, const PropertyType& key, BitVector& docs);

    void getValue(const std::string& property_name, const PropertyType& key, std::vector<docid_t>& docList);
    
//     std::size_t getValueBetween(const std::string& property_name, const PropertyType& lowKey, const PropertyType& highKey, KeyType* & data, std::size_t maxDoc )
//     {
//     
//         return 0;
//     }

    void getValueBetween(const std::string& property_name, const PropertyType& key1, const PropertyType& key2, BitVector& docs)
    {
    }

    void getValueLess(const std::string& property_name, const PropertyType& key, BitVector& docs)
    {
        
        
    }

    void getValueLessEqual(const std::string& property_name, const PropertyType& key, BitVector& docs)
    {
    }

    void getValueGreat(const std::string& property_name, const PropertyType& key, BitVector& docs)
    {
    }

    void getValueGreatEqual(const std::string& property_name, const PropertyType& key, BitVector& docs)
    {
    }

    void getValueIn(const std::string& property_name, const std::vector<PropertyType>& keys, BitVector& docs)
    {
    }

    void getValueNotIn(const std::string& property_name, const std::vector<PropertyType>& keys, BitVector& docs)
    {
    }

    void getValueNotEqual(const std::string& property_name, const PropertyType& key, BitVector& docs)
    {
    }

    void getValueStart(const std::string& property_name, const PropertyType& key, BitVector& docs)
    {
    }

    void getValueEnd(const std::string& property_name, const PropertyType& key, BitVector& docs)
    {
    }

    void getValueSubString(const std::string& property_name, const PropertyType& key, BitVector& docs)
    {
    }
    
    void flush()
    {
        //TODO how to flush all?
    }
    void setFilter(boost::shared_ptr<BitVector> pBitVector)
    {
        pFilter_ = pBitVector;
    }

    boost::shared_ptr<BitVector> getFilter()
    {
        return pFilter_;
    }

    void delDocument(size_t max_doc, docid_t docId)
    {
    }

private:
    
        
private:
    std::string dir_;
    boost::unordered_map<std::string, void*> instance_map_;
    boost::shared_ptr<BitVector> pFilter_;

};

class mseek_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, bool& find)
    {
        CBTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        find = pindexer->seek(v);
    }
};
    
class madd_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, docid_t docid)
    {
        CBTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        pindexer->add(v, docid, false);
    }
};

class maddu_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, docid_t docid)
    {
        CBTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        pindexer->add(v, docid, true);
    }
};

class mremove_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, docid_t docid)
    {
        CBTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        pindexer->remove(v, docid);
    }
};

class mget_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, BitVector& docids)
    {
        CBTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        pindexer->getValue(v, docids);
    }
};

}

NS_IZENELIB_IR_END

#endif 
