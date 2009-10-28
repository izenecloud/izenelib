/**
* @file        CommonItem.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief Define some common types of Indexer, these types or classes are the interfaces between IndexManager and other Managers.
*/
#ifndef COMMON_ITEM_H_
#define COMMON_ITEM_H_

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/index/PropertyItem.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <map>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

typedef boost::shared_ptr<std::deque<loc_t> > MyPositionPtr;


class CommonItem
{
public:
    CommonItem() {}

    CommonItem(const CommonItem& clone)
    {
        docid = clone.docid;
        tfMap = clone.tfMap;
        commonItemProperty = clone.commonItemProperty;
    }

    ~CommonItem() {}
public:
    void setCollectionID(collectionid_t colid_)
    {
        collectionid = colid_;
    }

    void setDocID(docid_t docid_)
    {
        docid = docid_;
    }

    void addProperty(std::string property, MyPositionPtr positions, freq_t tf)
    {
        if (commonItemProperty.find(property) == commonItemProperty.end())
        {
            commonItemProperty.insert(make_pair(property, positions));
            tfMap.insert(make_pair(property, tf));
        }
        else
        {
            commonItemProperty.erase(property);
            commonItemProperty.insert(make_pair(property, positions));
            tfMap.insert(make_pair(property, tf));
        }
    }


    void add(std::map<string, PropertyItem>& result)
    {
        for(std::map<string, PropertyItem>::iterator iter = result.begin(); iter != result.end(); ++iter)
        {
            commonItemProperty[iter->first] = iter->second.positions;
            tfMap[iter->first] = iter->second.tf;
        }
    }

    bool operator == (const CommonItem &other) const
    {
        return (docid == other.docid) && (collectionid == other.collectionid);
    }
public:
    docid_t docid;

    collectionid_t collectionid;

    std::map<std::string,freq_t> tfMap;

    std::map<std::string,MyPositionPtr > commonItemProperty;
};

}

NS_IZENELIB_IR_END

#endif
