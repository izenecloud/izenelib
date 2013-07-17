/**
 * @file        IndexManagerConfig.h
 * @author     Yingfeng Zhang
 * @version     SF1 v5.0
 * @brief The major configuration interface exposed to user of Indexmanager
 */
 #ifndef _INDEX_MANAGER_CONFIG_H_
#define _INDEX_MANAGER_CONFIG_H_

#include <ir/index_manager/index/IndexerCollectionMeta.h>

#include <stdint.h>
#include <string>
#include <map>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

#define POSTINGMERGE_BUFFERSIZE 32768

enum IndexLevel
{
    DOCLEVEL,  /// position posting does not create
    WORDLEVEL ///  position postings create
};

enum SamplePolicy
{
    WITHSAMPLE,
    NOSAMPLE
};
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
        explicit _indexstrategy() :
                memory_(0),
                indexLevel_(WORDLEVEL),
                indexDocLength_(false),
                skipInterval_(8),
                maxSkipLevel_(3),
                isIndexBTree_(true)
        {}
    private:
        friend class boost::serialization::access;

        template <typename Archive>
        void serialize( Archive & ar, const unsigned int version )
        {
            ar & indexLocation_;
            ar & indexLocations_;
            ar & indexMode_;
            ar & optimizeSchedule_;
            ar & memory_;
            ar & indexLevel_;
            ar & indexDocLength_;
            ar & skipInterval_;
            ar & samplePolicy_;
            ar & maxSkipLevel_;
            ar & isIndexBTree_;
        }


    public:
        /// @brief  Working directory
        std::string indexLocation_;
        /// @brief all working directory candidates
        ///
        /// directory name relative to the base index directory
        std::vector<std::string> indexLocations_;
        /// @bried index mode
        std::string indexMode_;

        std::string optimizeSchedule_;
        /**
         * @brief  the size of memory used by index cache
         * @details
         * Whe size of the memory cache, when the memory cache is full, the indexes in memory will be
         * flushed into one barrel and a new barrel will be generated
         */
        int64_t memory_;

        IndexLevel indexLevel_;

        bool indexDocLength_;

        int skipInterval_;

        int samplePolicy_;

        int maxSkipLevel_;

        /// true for generate BTree index, false for not to generate.
        bool isIndexBTree_;
    };

    /**
     * @brief   Stores "merge strategy" configuration of IndexManager
     */
    class _mergestrategy
    {
    public:
        _mergestrategy()
            :isAsync_(true),
            requireIntermediateFileForMerging_(true),
            memPoolSizeForPostingMerger_(POSTINGMERGE_BUFFERSIZE*512)
        {}

    private:
        friend class boost::serialization::access;

        template <typename Archive>
        void serialize( Archive & ar, const unsigned int version )
        {
            ar & param_;
            ar & isAsync_;
            ar & requireIntermediateFileForMerging_;
            ar & memPoolSizeForPostingMerger_;
        }
    public:
        /// @brief  param of merge method:
        /// NO - no merge
        /// DEFAULT - use the merge strategy defined by IndexManager
        std::string param_;

        /**
         * true for merge asynchronously (merge index in a separated thread),
         * false for merge synchronously (single thread for both build index and merge index).
         */
        bool isAsync_;

        /// when merging postings on disks(not SSD), if skiplist is enabled,
        /// we need an extra storage space to store merged posting.
        /// in some cases, such an extra storage could not be file, because it will lead to
        /// low efficiency caused by too many random read, and it will be much more serious
        /// when there exist mutiple disks with huge capacity.
        bool requireIntermediateFileForMerging_;

        /// Each posting merger requires a seperate mem pool, the size of which should
        /// be able to contain single posting. Default value is 16MB
        size_t memPoolSizeForPostingMerger_;
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
        ar & mergeStrategy_;
    }

public:
    //----------------------------  PUBLIC MEMBER VARIABLES  ----------------------------

    /// @brief  Stores "index strategy" configurationof IndexManager
    _indexstrategy indexStrategy_;

    /// @brief  Stores "Store strategy" configuration of IndexManager
    _storestrategy storeStrategy_;

    /// @brief  Stores "Merge strategy" configuration of IndexManager
    _mergestrategy mergeStrategy_;
private:
    //----------------------------  PRIVATE MEMBER VARIABLES  ----------------------------

    /// @brief  Maps Collection name to Collection meta information
    std::map<std::string, IndexerCollectionMeta> collectionMetaNameMap_;
};

} // namespace

NS_IZENELIB_IR_END

#endif //_INDEX_MANAGER_CONFIG_H_
