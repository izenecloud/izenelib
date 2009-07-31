/**
* @file        CommonItem.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Define some common types of Indexer, these types or classes are the interfaces between IndexManager and other Managers.
*/
#ifndef COMMON_ITEM_H_
#define COMMON_ITEM_H_

#include <ir/index_manager/utility/system.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <map>
#include <string>
#include <vector>
#include <algorithm>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

typedef boost::shared_ptr<std::vector<loc_t> > PositionPtr;

class CommonItem
{
public:
    CommonItem() {}

    CommonItem(const CommonItem& clone)
    {
        docid = clone.docid;
        // Added by TuanQuang Nguyen, 14 Oct 2008
        // Copy collection id
        collectionid = clone.collectionid;
        commonItemProperty = clone.commonItemProperty;
    }

    ~CommonItem() {}
public:
    // Added by TuanQuang Nguyen, 14 Oct, 2008
    // Add function setCollectionID
    void setCollectionID(collectionid_t colid_)
    {
        collectionid = colid_;
    }

    void setDocID(docid_t docid_)
    {
        docid = docid_;
    }
    void addProperty(std::string property, std::vector<loc_t>*positions)
    {
        if (commonItemProperty.find(property) == commonItemProperty.end())
            commonItemProperty.insert(make_pair(property, positions));
        else
        {
            commonItemProperty.erase(property);
            commonItemProperty.insert(make_pair(property, positions));
        }
    }

    inline void merge(CommonItem& src)
    {
        assert(docid == src.docid);
        for (std::map<std::string,PositionPtr>::iterator iter = commonItemProperty.begin();
                iter != commonItemProperty.end(); ++iter)
        {
            std::map<std::string,PositionPtr>::iterator srcIter = src.commonItemProperty.find(iter->first);
            if (srcIter != src.commonItemProperty.end())
            {
                iter->second->resize(iter->second->size()+srcIter->second->size());
                copy_backward (srcIter->second->begin(), srcIter->second->end(), iter->second->end() );
            }
        }
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & docid;
        ar & commonItemProperty;
    }

    bool operator == (const CommonItem &other) const
    {
        return (docid == other.docid) && (collectionid == other.collectionid);
    }
public:
    docid_t docid;

    // TuanQuang Nguyen, 14 Oct, 2008: Add collectionid member variable
    collectionid_t collectionid;

    std::map<std::string,PositionPtr> commonItemProperty;
};

}

NS_IZENELIB_IR_END

#endif
