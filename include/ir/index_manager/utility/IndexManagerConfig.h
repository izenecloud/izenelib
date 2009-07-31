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
                memory_(0),
                maxIndexTerms_(0),
                cacheDocs_(0)
        {}
    private:
        friend class boost::serialization::access;

        template <typename Archive>
        void serialize( Archive & ar, const unsigned int version )
        {
            ar & indexLocation_;
            ar & accessMode_;
            ar & memory_;
            ar & maxIndexTerms_;
            ar & cacheDocs_;
        }


    public:
        /// @brief  Working directory
        std::string indexLocation_;

        /// @brief  access mode of index file
        std::string accessMode_;

        /**
         * @brief  the size of memory used by index cache
         * @details
         * Whe size of the memory cache, when the memory cache is full, the indexes in memory will be
         * flushed into one barrel and a new barrel will be generated
         */
        int64_t memory_;

        /**
         * @brief  max indexed terms of a document
         * @details
         * when the memory is nearly full,when indexing a  new document ,this size will be used to
         * malloc an emergency memory pool
         */
        int32_t maxIndexTerms_;

        /**
         * @brief the cached document number of IndexWriter
         * @details
         * When the cached document reaches this number, these documents will be indexed
         */
        int32_t cacheDocs_;
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

    /**
     * @brief   Stores "Distribute strategy" configuration of IndexManager
     */
    class _distributestrategy
    {

    private:
        friend class boost::serialization::access;

        template <typename Archive>
        void serialize( Archive & ar, const unsigned int version )
        {
            ar & rpcport_;
            ar & batchport_;
            ar & iplist_;
        }
    public:
        /**
         * @brief   The working mode of Indexer.
         * @details
         * It could be "local", which means it works locally, "distribute", which means the indexer works distributedly
         */
        std::string rpcport_;

        /**
         * @brief   The listenport number for indexprocess
         */
        std::string batchport_;

        /**
         * @brief   The parameter setting for distribute strategy
         * @details
         *  the indexerprocess list, with format of:
         * "IP1|IP2|..."
         */
        std::string iplist_;
    };


    /**
     * @brief   Stores "advance" configuration of IndexManager
     */
    class _advance
    {
    private:
        friend class boost::serialization::access;

        template <typename Archive>
        void serialize( Archive & ar, const unsigned int version )
        {
            ar & MMS_;
            ar & uptightAlloc_;
        }

    public:
        /**
         * @brief   Memory Management Strategy
         * @details
         * it can be:
         * -# const:n  const means memory is allocated with constant size and n stands for the size
         * -# exp:n:k
         * -#explimit:n:k:l
         */
        std::string MMS_;

        class _uptightAlloc
        {

        public:
            _uptightAlloc() :
                    memSize_(0),
                    chunkSize_(0)
            {}
        private:
            friend class boost::serialization::access;

            template <typename Archive>
            void serialize( Archive & ar, const unsigned int version )
            {
                ar & memSize_;
                ar & chunkSize_;
            }

        public:
            /**
             * @brief   memory request size
             * @details
             * When the memory cache of building up index is exhausted, if the documents have not yet
             * been finished indexing, then it will enter the state of "UPTIGHT MEMORY ALLOCATE",
             * then it resent the request to allocate memory with default size of
             * IndexStrategy.maxIndexTerms*chunkSize/2, or with the size configured here.
             */
            int32_t memSize_;

            /**
             * @brief   chunk size of posting.
             */
            int32_t chunkSize_;
        };
        _uptightAlloc uptightAlloc_;
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
        ar & mergeStrategy_;
        ar & storeStrategy_;
        ar & advance_;
        ar & distributeStrategy_;
        ar & logLevel_;
        ar & collectionMetaNameMap_;
    }

public:
    //----------------------------  PUBLIC MEMBER VARIABLES  ----------------------------

    /// @brief  Stores "index strategy" configurationof IndexManager
    _indexstrategy indexStrategy_;

    /// @brief  Stores "merge strategy" configuration of IndexManager
    _mergestrategy mergeStrategy_;

    /// @brief  Stores "Store strategy" configuration of IndexManager
    _storestrategy storeStrategy_;

    /// @brief  Stores "Distribute strategy" configuration of IndexManager
    _distributestrategy distributeStrategy_;

    /// @brief  Stores "advance" configuration of IndexManager
    _advance advance_;

    /// @brief  disable_all,enable_all,default_level(info),fatal,err,warn,info,dbg
    std::string logLevel_;


private:
    //----------------------------  PRIVATE MEMBER VARIABLES  ----------------------------

    /// @brief  Maps Collection name to Collection meta information
    std::map<std::string, IndexerCollectionMeta> collectionMetaNameMap_;
};

} // namespace

NS_IZENELIB_IR_END

#endif //_INDEX_MANAGER_CONFIG_H_

