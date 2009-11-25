/**
* @file        CollectionInfo.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief associate the FieldsInfo object with each Collection
*/
#ifndef COLLECTIONINFO_H
#define COLLECTIONINFO_H

#include <ir/index_manager/index/FieldInfo.h>

#include <map>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
/**
*@brief Associate FieldsInfo object with each collection
*@note Each collection has different FieldsInfo
*/
class CollectionInfo
{
public:
    CollectionInfo(collectionid_t id);

    CollectionInfo(collectionid_t id, FieldsInfo* pInfo);

    CollectionInfo(CollectionInfo& src);

    ~CollectionInfo();
public:
    void setId(collectionid_t id) { id_ = id; }

    collectionid_t getId() { return id_; }
    ///set the FieldsInfo object to associate it with collection id_.
    void setFieldsInfo(FieldsInfo* pInfo) { pFieldsInfo_ = pInfo; }
    ///return the associated FieldsInfo object
    FieldsInfo* getFieldsInfo() { return pFieldsInfo_; }

    void setOwn(bool own) { ownFieldInfo_ = own; }
private:
    collectionid_t id_;

    FieldsInfo* pFieldsInfo_;

    bool ownFieldInfo_;
};
/**
*@brief A collection of CollectionInfo, it is composed of CollectionInfo
*@note It is the manager class of CollectionInfo
*/

class CollectionsInfo
{
public:
    CollectionsInfo();
    CollectionsInfo(CollectionsInfo& src);
    ~CollectionsInfo();
public:
    void addCollection(CollectionInfo* pColInfo)
    {
        collectionsInfoMap_.insert(std::pair<collectionid_t, CollectionInfo*>(pColInfo->getId(),pColInfo));
    }
    /**
    * read collectionsInfo from "fdi" file
    */
    void read(IndexInput* pIndexInput);
    /**
    * read collectionsInfo from "fdi" file
    */
    void write(IndexOutput* pIndexOutput);

    void clear();

    void reset();

    CollectionInfo* getCollectionInfo(collectionid_t cid)
    {
        return collectionsInfoMap_[cid];
    }

    int32_t numCollections()
    {
        return (int32_t)collectionsInfoMap_.size();
    }

public:
    CollectionInfo* operator[](int32_t i)
    {
        return collectionsInfoMap_[i];
    }
    void startIterator()
    {
        colInfosIterator_ = collectionsInfoMap_.begin();
    }
    bool hasNext()
    {
        return (colInfosIterator_ != collectionsInfoMap_.end());
    }
    CollectionInfo* next()
    {
        return (colInfosIterator_++)->second;
    }
    void removeCollectionInfo(collectionid_t colID);
private:
    std::map<collectionid_t, CollectionInfo*> collectionsInfoMap_;
    std::map<collectionid_t, CollectionInfo*>::iterator colInfosIterator_;
};

}

NS_IZENELIB_IR_END

#endif
