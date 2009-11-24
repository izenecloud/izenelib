/**
* @file        IndexerCollectionMeta.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief   The main configuration interface exposed to user of IndexManager
* it is used to describe the document schema of one collection
*/
#ifndef INDEXER_COLLECTIONMETA_H
#define INDEXER_COLLECTIONMETA_H

#include <ir/index_manager/index/IndexerPropertyConfig.h>

#include <string>
#include <set>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
class IndexerCollectionMeta
{
public:
    IndexerCollectionMeta() {}

    IndexerCollectionMeta(const IndexerCollectionMeta& other)
        :colId_(other.colId_)
        ,name_(other.name_)
        ,encoding_(other.encoding_)
        ,schema_(other.schema_)
    {}

    void setColId(uint32_t id)
    {
        colId_ = id;
    }

    uint32_t getColId() const
    {
        return colId_;
    }

    void setName( const std::string & name )
    {
        name_ = name;
    }

    std::string getName() const
    {
        return name_;
    }


    void setEncoding( const std::string & encoding )
    {
        encoding_ = encoding;
    }

    std::string getEncoding() const
    {
        return encoding_;
    }


    void addPropertyConfig( const IndexerPropertyConfig & property )
    {
        schema_.insert( property );
        //initDocumentSchema();///bad smell here
    }


    void setDocumentSchema( const std::set<IndexerPropertyConfig, IndexerPropertyConfigComp> & schema )
    {
        schema_ = schema;
    }

    void initDocumentSchema()
    {
        unsigned int property_id = 0;
        std::set<IndexerPropertyConfig, IndexerPropertyConfigComp> schema;
        for(std::set<IndexerPropertyConfig, IndexerPropertyConfigComp>::iterator iter = schema_.begin(); iter != schema_.end(); ++iter)
        {
            IndexerPropertyConfig propertyConfig = *iter;
            propertyConfig.setPropertyId(property_id++);
            schema.insert(propertyConfig);
        }
        schema_.clear();
        schema_ = schema;
    }

    const std::set<IndexerPropertyConfig, IndexerPropertyConfigComp> & getDocumentSchema() const
    {
        return schema_;
    }

protected:
    /// @brief	Collection ID. Created by IDManager.
    uint32_t colId_;

    /// @brief	The Collection's name given by the user
    std::string name_;

    /// @brief	The encoding type of the Collection
    std::string encoding_;

    /// @brief	The DocumentSchemaConfig of this Collection
    std::set<IndexerPropertyConfig, IndexerPropertyConfigComp> schema_;

};

}

NS_IZENELIB_IR_END

#endif
