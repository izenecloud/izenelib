#ifndef IZENELIB_IR_BTREEINDEXERMANAGER_H
#define IZENELIB_IR_BTREEINDEXERMANAGER_H

#include "BTreeIndexer.h"
#include <types.h>
#include <boost/variant.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/not.hpp>
#include <boost/bind.hpp>
#include <boost/thread/locks.hpp>
#include <boost/unordered_map.hpp>
#include <boost/iterator/iterator_facade.hpp>
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
    BTreeIndexerManager(const std::string& dir, Directory* pDirectory, const std::map<std::string, PropertyType>& type_map);

    ~BTreeIndexerManager();
public:
    typedef std::vector<docid_t> ValueType;
    
    template<class T>
    BTreeIndexer<T>* getIndexer(const std::string& property_name)
    {
        //TODO refine lock
        boost::unique_lock<boost::shared_mutex> lock(mutex_);
        BTreeIndexer<T>* result = NULL;
        boost::unordered_map<std::string, void*>::iterator it = instance_map_.find(property_name);
        if(it==instance_map_.end())
        {
            //boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);
            result = new BTreeIndexer<T>(dir_+"/bt_property."+property_name, property_name);
            result->open();
            instance_map_.insert(std::make_pair( property_name, (void*)result) );
        }
        else
        {
            void* pv = it->second;
            result = (BTreeIndexer<T>*)pv;
        }
        return result;
    }
    
    void add(const std::string& property_name, const PropertyType& key, docid_t docid);

    void remove(const std::string& property_name, const PropertyType& key, docid_t docid);
    
    std::size_t count(const std::string& property_name);
    
    bool seek(const std::string& property_name, const PropertyType& key);

    void getNoneEmptyList(const std::string& property_name, const PropertyType& key, BitVector& docs);

    void getValue(const std::string& property_name, const PropertyType& key, BitVector& docs);

    void getValue(const std::string& property_name, const PropertyType& key, std::vector<docid_t>& docList);

    void getValueBetween(const std::string& property_name, const PropertyType& key1, const PropertyType& key2, BitVector& docs);

    void getValueLess(const std::string& property_name, const PropertyType& key, BitVector& docs);

    void getValueLessEqual(const std::string& property_name, const PropertyType& key, BitVector& docs);

    void getValueGreat(const std::string& property_name, const PropertyType& key, BitVector& docs);

    void getValueGreatEqual(const std::string& property_name, const PropertyType& key, BitVector& docs);

    void getValueIn(const std::string& property_name, const std::vector<PropertyType>& keys, BitVector& docs);

    void getValueNotIn(const std::string& property_name, const std::vector<PropertyType>& keys, BitVector& docs);

    void getValueNotEqual(const std::string& property_name, const PropertyType& key, BitVector& docs);

    void getValueStart(const std::string& property_name, const PropertyType& key, BitVector& docs);

    void getValueEnd(const std::string& property_name, const PropertyType& key, BitVector& docs);

    void getValueSubString(const std::string& property_name, const PropertyType& key, BitVector& docs);
    
    void flush();
    
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
        if(!pFilter_)
        {
            pFilter_.reset(new BitVector(max_doc));
        }
        pFilter_->set(docId);
    }

    ///This Iterator is a template class, which requires the client to specify the type
    ///Another kinds of design is to use boost::variant to contain all kinds of BTreeIteratorType
    ///And then define a series of visitors, assign_visitor, including increment_visitor, dereference_visitor
    ///It's verbose, so we only uses a template based Iterator definition
    ///Additionally, this Iterator is a based on the persistent container of BTreeIndexer,
    ///So it should be used after flush operation has been called.
    template<typename KeyType>
    class Iterator 
        : public boost::iterator_facade<Iterator<KeyType>, std::pair<KeyType,ValueType> const, boost::forward_traversal_tag >
    {
    typedef typename BTreeIndexer<KeyType>::iterator BTreeIteratorType;
    public:
        Iterator()
            :indexer_(NULL)
            ,iterEnd_()
        {}

        explicit Iterator(const BTreeIndexerManager& indexer,
                                        const std::string& property_name)
            :indexer_(&indexer)
            ,iterEnd_()
        {
            iter_ = const_cast<BTreeIndexerManager&>(indexer).getIndexer<KeyType>(property_name)->begin();
            //increment();
        }

    private:
        friend class boost::iterator_core_access;

        void increment() 
        { 
            if(indexer_)
            {
                ++iter_;
                if(iter_ == iterEnd_)
                    indexer_ = NULL;
            }
        }

        const std::pair<KeyType,ValueType> & dereference() const {return *iter_; }

        bool equal(const Iterator<KeyType>& rhs) const
        {
            return (this == &rhs) || (indexer_ == 0 && rhs.indexer_ == 0)
                        || (indexer_ == rhs.indexer_ && iter_ == rhs.iter_);
        }

        const BTreeIndexerManager* indexer_;
        BTreeIteratorType iter_;
        BTreeIteratorType iterEnd_;
    };

    template<typename KeyType>
    Iterator<KeyType> begin(const std::string& property_name)
    {
        return Iterator<KeyType>(*this, property_name);
    }

    template<typename KeyType>
    Iterator<KeyType> end()
    {
        return Iterator<KeyType>();
    }

private:
    
    bool checkPropertyName_(const std::string& propertyName);
    bool checkType_(const std::string& propertyName, const PropertyType& value);
    void doFilter_(BitVector& docs);
    
private:
    std::string dir_;
    Directory* pDirectory_;
    boost::unordered_map<std::string, void*> instance_map_;
    boost::unordered_map<std::string, PropertyType> type_map_;
    boost::shared_ptr<BitVector> pFilter_;
    boost::shared_mutex mutex_;

};

/// all modifer visitor below    
class madd_visitor : public boost::static_visitor<void>
{
public:
    
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const izenelib::util::UString& v, docid_t docid)
    {
        if(v.length()>0)
        {
            BTreeIndexer<izenelib::util::UString>* pindexer = manager->getIndexer<izenelib::util::UString>(property_name);
            pindexer->add(v, docid);
        }
    }
    
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, docid_t docid)
    {
        BTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        pindexer->add(v, docid);
    }
    
    
};

class mremove_visitor : public boost::static_visitor<void>
{
public:
    
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const izenelib::util::UString& v, docid_t docid)
    {
        if(v.length()>0)
        {
            BTreeIndexer<izenelib::util::UString>* pindexer = manager->getIndexer<izenelib::util::UString>(property_name);
            pindexer->remove(v, docid);
        }
    }
    
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, docid_t docid)
    {
        BTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        pindexer->remove(v, docid);
    }
};

class mflush_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v)
    {
        BTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        pindexer->flush();
    }
};

class mdelete_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v)
    {
        BTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        delete pindexer;
    }
};

class mcount_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, std::size_t& count)
    {
        BTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        count = pindexer->count();
    }
};

///all read only with one key visitors below
class mseek_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, bool& find)
    {
        BTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        find = pindexer->seek(v);
    }
};

class mget_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, BitVector& docs)
    {
        BTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        pindexer->getValue(v, docs);
    }
};

class mget2_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, std::vector<docid_t>& docs)
    {
        BTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        pindexer->getValue(v, docs);
    }
};

class mless_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, BitVector& docs)
    {
        BTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        pindexer->getValueLess(v, docs);
    }
};

class mless_equal_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, BitVector& docs)
    {
        BTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        pindexer->getValueLessEqual(v, docs);
    }
};

class mgreat_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, BitVector& docs)
    {
        BTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        pindexer->getValueGreat(v, docs);
    }
};

class mgreat_equal_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, BitVector& docs)
    {
        BTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        pindexer->getValueGreatEqual(v, docs);
    }
};

class mnot_equal_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, BitVector& docs)
    {
        BTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        pindexer->getValueNotEqual(v, docs);
    }
};

class mstart_equal_visitor : public boost::static_visitor<void>
{
public:
    
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const izenelib::util::UString& v, BitVector& docs)
    {
        BTreeIndexer<izenelib::util::UString>* pindexer = manager->getIndexer<izenelib::util::UString>(property_name);
        pindexer->getValueStart(v, docs);
    }
    
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, BitVector& docs)
    {
        std::cout<<"error, call getValueStart in no ustring type"<<std::endl;
    }
};

class mend_equal_visitor : public boost::static_visitor<void>
{
public:
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const izenelib::util::UString& v, BitVector& docs)
    {
        BTreeIndexer<izenelib::util::UString>* pindexer = manager->getIndexer<izenelib::util::UString>(property_name);
        pindexer->getValueEnd(v, docs);
    }
    
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, BitVector& docs)
    {
        std::cout<<"error, call getValueEnd in no ustring type"<<std::endl;
    }
};

class msub_string_visitor : public boost::static_visitor<void>
{
public:
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const izenelib::util::UString& v, BitVector& docs)
    {
        BTreeIndexer<izenelib::util::UString>* pindexer = manager->getIndexer<izenelib::util::UString>(property_name);
        pindexer->getValueSubString(v, docs);
    }
    
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& v, BitVector& docs)
    {
        std::cout<<"error, call getValueSubString in no ustring type"<<std::endl;
    }
};

///all read only with two keys visitors below
class mbetween_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T& low, const T& high, BitVector& docs)
    {
        BTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
        pindexer->getValueBetween(low, high, docs);
    }
    
    template<typename T1, typename T2>
    void operator()(BTreeIndexerManager* manager, const std::string& property_name, const T1& low, const T2& high, BitVector& docs)
    {
        
    }
};

///all read only with list keys visitors below
// class min_visitor : public boost::static_visitor<void>
// {
// public:
//     template<typename T>
//     void operator()(BTreeIndexerManager* manager, const std::string& property_name, const std::vector<T>& v_list, BitVector& docs)
//     {
//         BTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
//         pindexer->getValueIn(v_list, docs);
//     }
// };
// 
// class mnot_in_visitor : public boost::static_visitor<void>
// {
// public:
//     template<typename T>
//     void operator()(BTreeIndexerManager* manager, const std::string& property_name, const std::vector<T>& v_list, BitVector& docs)
//     {
//         BTreeIndexer<T>* pindexer = manager->getIndexer<T>(property_name);
//         pindexer->getValueNotIn(v_list, docs);
//     }
// };


}

NS_IZENELIB_IR_END

#endif 
