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


    bool addPropertyConfig( const IndexerPropertyConfig & property )
    {
        return schema_.insert( property ).second;
    }


    void setDocumentSchema( const std::set<IndexerPropertyConfig> & schema )
    {
        schema_ = schema;
    }


    const std::set<IndexerPropertyConfig> & getDocumentSchema() const
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
    std::set<IndexerPropertyConfig> schema_;

};

}

NS_IZENELIB_IR_END

#endif
