#ifndef _INDEX_MANAGER_CONFIG_H_
#define _INDEX_MANAGER_CONFIG_H_

#include <ir/index_manager/index/IndexerCollectionMeta.h>

#include <stdint.h>
#include <string>
#include <map>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

/**
 * @brief   IndexManagerConfig Holds the configuration data of IndexManager and provides interfaces for the IndexManager
 *          to retrieve the data.
 */
class IndexManagerConfig
{

private:
    //----------------------------  PRIVATE INNER CLASS DEFINITIONS  ----------------------------

    /**
     * @brief   Stores "index strategy" configurationof IndexManager
     */
    class _indexstrategy
    {
    public:
        _indexstrategy() :
                memory_(0)
        {}
    private:
        friend class boost::serialization::access;

        template <typename Archive>
        void serialize( Archive & ar, const unsigned int version )
        {
            ar & indexLocation_;
            ar & memory_;
        }


    public:
        /// @brief  Working directory
        std::string indexLocation_;
        /**
         * @brief  the size of memory used by index cache
         * @details
         * Whe size of the memory cache, when the memory cache is full, the indexes in memory will be
         * flushed into one barrel and a new barrel will be generated
         */
        int64_t memory_;
    };

    /**
     * @brief   Stores "merge strategy" configuration of IndexManager
     */
    class _mergestrategy
    {

    private:
        friend class boost::serialization::access;

        template <typename Archive>
        void serialize( Archive & ar, const unsigned int version )
        {
            ar & strategy_;
            ar & param_;
        }
    public:
        /**
         * @brie    the merge method of index.
         * @details
         * It could be "OPT" or "DBT", OPT means all the postings exist in only one barrel,
         * it could provide higher search performance while much lower indexing performance,"DBT" is default
         */
        std::string strategy_;

        /// @brief  param of merge method
        std::string param_;
    };

    /**
     * @brief   Stores "Store strategy" configuration of IndexManager
     */
    class _storestrategy
    {

    private:
        friend class boost::serialization::access;

        template <typename Archive>
        void serialize( Archive & ar, const unsigned int version )
        {
            ar & param_;
        }
    public:
        /// @brief  whether the indexes are stored in file or memory
        std::string param_;
    };

public:
    //----------------------------  CONSTRUCTORS  ----------------------------

    IndexManagerConfig() {}

    ~IndexManagerConfig() {}


    bool addCollectionMeta( const IndexerCollectionMeta& collectionMeta )

    {
        if ( collectionMetaNameMap_.insert(
                    std::pair<std::string, IndexerCollectionMeta>( collectionMeta.getName(), collectionMeta )
                ).second )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool setCollectionMetaList( const std::vector<IndexerCollectionMeta> & list )
    {
        unsigned int i=0;
        for ( i = 0; i < list.size(); i++ )
        {
            if ( addCollectionMeta( list[i] ) == false )
            {
                return false;
            }
        }
        return true;
    }

    const std::map<std::string, IndexerCollectionMeta> & getCollectionMetaNameMap() const
    {
        return collectionMetaNameMap_;
    }

    void setCollectionMetaNameMap( const std::map<std::string, IndexerCollectionMeta> & map )
    {
        collectionMetaNameMap_ = map;
    }

private:
    //----------------------------	PRIVATE SERIALIZTAION FUNCTION	----------------------------

    friend class boost::serialization::access;

    template <typename Archive>
    void serialize( Archive & ar, const unsigned int version )
    {
        ar & indexStrategy_;
        ar & storeStrategy_;
    }

public:
    //----------------------------  PUBLIC MEMBER VARIABLES  ----------------------------

    /// @brief  Stores "index strategy" configurationof IndexManager
    _indexstrategy indexStrategy_;

    /// @brief  Stores "Store strategy" configuration of IndexManager
    _storestrategy storeStrategy_;


private:
    //----------------------------  PRIVATE MEMBER VARIABLES  ----------------------------

    /// @brief  Maps Collection name to Collection meta information
    std::map<std::string, IndexerCollectionMeta> collectionMetaNameMap_;
};

} // namespace

NS_IZENELIB_IR_END

#endif //_INDEX_MANAGER_CONFIG_H_

