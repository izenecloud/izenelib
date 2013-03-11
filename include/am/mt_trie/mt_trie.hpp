/**
 * @brief A high-performance and multi-threaded trie implementation based on Hdb.
 *        Incremental update is also supported.
 *        See TR "A high-performance and multi-threaded trie based on HdbTrie" for details.
 * @author Wei Cao
 * @date 2009-12-1
 */

#ifndef _MT_TRIE_H_
#define _MT_TRIE_H_

#include <cstdio>
#include <iostream>
#include <fstream>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/nvp.hpp>

#include <util/ThreadModel.h>

#include "sampler.h"
#include "partition_trie.hpp"

NS_IZENELIB_AM_BEGIN

template <typename StringType>
class MtTrie
{
public:
    enum{ L1CacheThreshold = 50 };

    typedef uint64_t TrieNodeIDType;
    typedef PartitionTrie<StringType, TrieNodeIDType> PartitionTrieType;
    typedef FinalTrie<StringType, TrieNodeIDType> FinalTrieType;
    typedef FinalL1Trie<StringType, TrieNodeIDType> FinalL1TrieType;

public:
    /**
     *  @param partitionNum - how many partitions do we divide all input terms into.
     */
    MtTrie(const std::string& name, const int partitionNum)
    :   name_(name), configPath_(name_ + ".config.xml"),
        writeCachePath_(name_+".writecache"),
        sampler_(name_ + ".sampler"),
        trie_(name_ + ".trie"),
        l1Trie_(name_ + ".l1.trie")
    {
        if( partitionNum <= 0 || partitionNum > 256 ) {
            throw std::runtime_error( logHead() + "wrong partitionNum,\
                should between 0 and 256");
        }

        if(load()) {
            if(partitionNum_ != partitionNum)
                throw std::runtime_error( logHead() + "inconsistent partitionNum");
        } else {
            partitionNum_ = partitionNum;
            boundariesInitialized_ = false;
            boundaries_.clear();
            partitionsInitialized_.resize(partitionNum_, false);
            startNodeID_.resize(partitionNum_, NodeIDTraits<TrieNodeIDType>::MinValue);
            l1TrieStartNodeID_.resize(partitionNum_, NodeIDTraits<TrieNodeIDType>::MinValue);

            trieInitialized_ = false;
        }

        sync();
    }

    void open()
    {
        remove( writeCachePath_.c_str() );
        writeCache_.open(writeCachePath_.c_str(), std::ofstream::out|
            std::ofstream::binary | std::ofstream::app );
        if(writeCache_.fail())
            std::cerr << logHead() << " fail to open write cache" << std::endl;

        trie_.open();
        l1Trie_.open();
    }

    void close()
    {
        writeCache_.close();
        trie_.close();
        l1Trie_.close();
    }

    void flush()
    {
        sync();
        writeCache_.flush();
        trie_.flush();
        l1Trie_.flush();
    }

    /**
     * @brief Append term into write cache of MtTrie.
     */
	void insert(const StringType & term)
	{
        try {
             int len = (int) term.size();
             writeCache_.write((char*) &len, sizeof(int));
             writeCache_.write((const char*)term.c_str(), term.size());
             sampler_.collect(term);
        } catch(const std::exception & e) {
            std::cerr << logHead() << " fail to update writecache" << std::endl;
        }
    }

    /**
     * @brief Main control flow, invoking multiple threads processing string collection.
     *        Called after all insert() operations.
     * @param   filename - input file that contains all terms.
     *          threadNum - how many threads do we start, each thread could process
     *                      one or more partitions.
     */
    void executeTask(const int threadNum) {
        threadNum_ = threadNum;
        if( threadNum_<= 0)
            throw std::runtime_error(logHead() + "wrong threadNum, should be larger than 0");
        if( partitionNum_ < threadNum_ ) {
            std::cout << logHead() << " threadNum(" << threadNum << ") is less than partitionNum("
                << partitionNum_ << ", shrink it" << std::endl;
            threadNum_ = partitionNum_;
        }

        if( !boundariesInitialized_ ) {
            std::set<StringType> samples;
            sampler_.getSamples(samples);

            std::cout << logHead() << "start computing " << (partitionNum_-1) <<
                " boundaries from " << samples.size() << " samples" << std::endl;

            if( samples.size() > (size_t)(partitionNum_-1) ) {
                int count = 0;
                size_t interval = samples.size()/partitionNum_;

                typename std::set<StringType>::iterator it = samples.begin();
                while(count < partitionNum_-1) {
                    for(size_t i=0; i<interval; i++)
                        it++;
                    boundaries_.push_back(*it);
                    count ++;
                }
            } else {
                std::cout << logHead() << "number of terms isn't enough to be partitioned to "
                    << partitionNum_ << " partitions" << std::endl;
                sync();
                return;
            }

            boundariesInitialized_ = true;

            sync();

        } else {
            std::cout << logHead() << "skip computing boundareis, already exist"
                << std::endl;
        }

        writeCache_.flush();
        writeCache_.close();
        splitInput();
        remove( writeCachePath_.c_str() );
        writeCache_.open(writeCachePath_.c_str(), std::ofstream::out | std::ofstream::binary );

        // multi-threaded building tries on partitions
        std::cout << logHead() << "Build " << partitionNum_ << " partitions in "
            << threadNum_ << " threads" << std::endl;

        processedPartitions_ = 0;
        for( int i=0; i < threadNum_; i++ )
          workerThreads_.create_thread(boost::bind(&MtTrie::taskBody, this, i) );
        workerThreads_.join_all();
        std::cout << std::endl;

        // merging tries on partitions into one
        mergeTries();

        flush();
    }

    /**
     * @brief Get a list of values whose keys match the wildcard query. Only "*" and
     * "?" are supported currently, legal input looks like "ea?th", "her*", or "*ear?h".
     * @return true at leaset one result found.
     *         false nothing found.
     */
    bool findRegExp(const StringType& regexp,
        std::vector<StringType>& keyList,
        int maximumResultNumber)
    {
        std::vector<StringType> l1SearchResult;
        std::vector<int> l1Count;
        l1Trie_.findRegExp(regexp, l1SearchResult, l1Count, maximumResultNumber > 100? maximumResultNumber:100 );
        for(int i=0; i<maximumResultNumber; i++) {
            if(l1Count.size() == 0) break;
            int max = 0; int maxIndex = 0;
            for(size_t j=0; j<l1Count.size(); j++) {
                if( l1Count[j] > max ) {
                    max = l1Count[j];
                    maxIndex = j;
                }
            }
            //if(max == 0) break;
            keyList.push_back(l1SearchResult[maxIndex]);
            l1Count.erase(l1Count.begin()+maxIndex);
            l1SearchResult.erase(l1SearchResult.begin()+maxIndex);
        }

        if(keyList.size() < (unsigned)maximumResultNumber) {
            std::vector<StringType> l2SearchResult;
            trie_.findRegExp(regexp, l2SearchResult, maximumResultNumber);
            for(typename std::vector<StringType>::iterator it = l2SearchResult.begin(); it!=l2SearchResult.end(); it++ ) {
                if( std::find(keyList.begin(), keyList.end(), *it) == keyList.end() ) {
                    keyList.push_back(*it);
                    if(keyList.size() == (unsigned)maximumResultNumber) break;
                }
            }
        }
        return keyList.size()==0 ? false:true;
    }

protected:

    std::string logHead() {
        return "MtTrie[" + name_ + "] ";
    }

    /**
     * @return starting point of NodeID in a specific partition.
     */
    TrieNodeIDType getPartitionStartNodeID(int partitionId) {
        TrieNodeIDType ret = 1;
        ret <<= (sizeof(TrieNodeIDType)*8 - 8);
        ret *= partitionId;
        return ret;
    }

    /**
     * @return prefix of files associated with a specific partition.
     */
    std::string getPartitionName(int partitionId) {
        return name_ + ".partition" + boost::lexical_cast<std::string>(partitionId);
    }

    /**
     * @brief Split orignal input files into multiple partitions.
     */
    void splitInput() {

        std::ifstream input;
        input.open(writeCachePath_.c_str(), std::ifstream::in|std::ifstream::binary );
        if(input.fail())
            throw std::runtime_error("failed open write cache " + writeCachePath_);

        long sizeofInput;
        input.seekg(0, ios_base::end);
        sizeofInput = input.tellg();
        input.seekg(0, ios_base::beg);

        std::ofstream* output = new std::ofstream[partitionNum_];
        for(int i = 0; i<partitionNum_; i++ ) {
            std::string outputName = getPartitionName(i) + ".input";
            output[i].open(outputName.c_str(), std::ofstream::out|std::ofstream::binary|std::ofstream::app);
            if(output[i].fail())
                throw std::runtime_error("failed prepare input for " + getPartitionName(i) );
        }

        StringType term;
        int buffersize = 0;
        char charBuffer[256];
        int progress = 0;
        long sizeofNextProgress = 0;
        long sizeofProcessed = 0;

        while(!input.eof())
        {
            input.read((char*)&buffersize, sizeof(int));
            if(input.eof()) break;
            // skip too long term
            if(buffersize > 256) {
                input.seekg(buffersize, std::ifstream::cur);
                continue;
            }
            input.read( charBuffer, buffersize );
            term.assign( std::string(charBuffer,buffersize) );

            if(term.size() > 0) {
                int pos = std::lower_bound(boundaries_.begin(), boundaries_.end(),
                        term) - boundaries_.begin();
                output[pos].write((char*)&buffersize, sizeof(int));
                output[pos].write(charBuffer, buffersize);
            }

            sizeofProcessed += (buffersize + sizeof(int));
            if(sizeofProcessed >= sizeofNextProgress ) {
                std::cout << "\r" << logHead() << "Split input, progress ["
                    << progress << "%]" << std::flush;
                progress ++;
                sizeofNextProgress = (long)( ((double)(progress)/(double)100)
                    *(double)sizeofInput );
            }
        }
        input.close();
        for(int i = 0; i<partitionNum_; i++ ) {
            output[i].close();
        }
        delete[] output;
        std::cout << std::endl;
    }

    void taskBody(int threadId) {
        int partitionId = threadId;
        while(partitionId < partitionNum_) {

            std::string inputName = getPartitionName(partitionId) + ".input";
            std::ifstream input(inputName.c_str(), std::ifstream::in|std::ifstream::binary);

            std::string trieName = getPartitionName(partitionId) + ".trie";
            PartitionTrieType trie(trieName);
            trie.open();

            std::string l1TrieName = getPartitionName(partitionId) + ".l1.trie";
            PartitionTrieType l1Trie(l1TrieName);
            l1Trie.open();

            // Initialize trie at the first time.
            if( !partitionsInitialized_[partitionId] ) {
                // Insert all boundaries into trie, to ensure NodeIDs are consistent
                // in all tries, the details of reasoning and proof see MtTrie TR.
                for(size_t i =0; i<boundaries_.size(); i++) {
                    trie.insert(boundaries_[i], 0);
                    l1Trie.insert(boundaries_[i], 0);
                }
                trie.setBaseNID(getPartitionStartNodeID(partitionId));
                startNodeID_[partitionId] = trie.getNextNID();

                l1Trie.setBaseNID(getPartitionStartNodeID(partitionId));
                l1TrieStartNodeID_[partitionId] = l1Trie.getNextNID();

                partitionsInitialized_[partitionId] = true;
            }

            if(input) {
                // Insert all terms in this partition
                StringType term;
                int buffersize = 0;
                char charBuffer[256];
                while(!input.eof())
                {
                    input.read((char*)&buffersize, sizeof(int));
                    if(input.eof()) break;
                    input.read( charBuffer, buffersize );
                    term.assign( std::string(charBuffer,buffersize) );

                    int count = 0;
                    if (trie.get(term, count)) {
                        trie.update(term, ++count);
                    } else {
                        trie.insert(term, ++count);
                    }
                    if(count >= L1CacheThreshold) {
                        l1Trie.update(term, count);
                    }
                }
                trie.optimize();
                l1Trie.optimize();

                input.close();
                remove( inputName.c_str() );
            }

            trie.close();
            l1Trie.close();

            printLock_.lock();
            processedPartitions_ ++;
            std::cout << "\r" << logHead() << "Build partitions, progress ["
                << (100*processedPartitions_)/partitionNum_ << "%]" << std::flush;
            printLock_.unlock();

            partitionId += threadNum_;
        }
    }

    void mergeTries() {

        // Initialize trie at the first time.
        if(!trieInitialized_) {
            for(size_t i =0; i<boundaries_.size(); i++) {
                trie_.insert(boundaries_[i]);
                l1Trie_.insert(boundaries_[i], 0);
            }
            trieInitialized_ = true;
        }

        int mergedPartitions = 0;
        for(int i=0; i<partitionNum_; i++) {
            PartitionTrieType pt(getPartitionName(i) + ".trie");
            pt.open();
            PartitionTrieType l1pt(getPartitionName(i) + ".l1.trie");
            l1pt.open();

            mergeToFinal(trie_, pt, startNodeID_[i]);
            mergeToFinalL1(l1Trie_, l1pt, l1TrieStartNodeID_[i]);
            startNodeID_[i] = pt.getNextNID();
            l1TrieStartNodeID_[i] = l1pt.getNextNID();
            sync();

            pt.close();
            l1pt.close();
            mergedPartitions ++;
            std::cout << "\r" << logHead() << "Merge partitions, progress ["
                << (100*mergedPartitions)/(partitionNum_+1) << "%]" << std::flush;
        }

        trie_.optimize();
        trie_.flush();

        l1Trie_.optimize();
        l1Trie_.flush();

        std::cout << "\r" << logHead() << "Merge partitions, progress [100%]"
            << std::endl << std::flush;
    }

protected:

    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::make_nvp("PartitionNumber", partitionNum_);

        ar & boost::serialization::make_nvp("BoundariesInitialized", boundariesInitialized_);
        std::vector<std::string> buffer;
        for(size_t i =0; i< boundaries_.size(); i++ ) {
            buffer.push_back( std::string((char*)boundaries_[i].c_str(),
                    boundaries_[i].size()) );
        }
        ar & boost::serialization::make_nvp("Boundaries", buffer);

        ar & boost::serialization::make_nvp("PartitionsInitialized", partitionsInitialized_);
        ar & boost::serialization::make_nvp("StartNodeID", startNodeID_);
        ar & boost::serialization::make_nvp("L1TrieStartNodeID", l1TrieStartNodeID_);

        ar & boost::serialization::make_nvp("TrieInitialized", trieInitialized_);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::make_nvp("PartitionNumber", partitionNum_);

        ar & boost::serialization::make_nvp("BoundariesInitialized", boundariesInitialized_);
        std::vector<std::string> buffer;
        ar & boost::serialization::make_nvp("Boundaries", buffer);
        for(size_t i =0; i< buffer.size(); i++ ) {
            boundaries_.push_back( StringType(buffer[i]) );
        }

        ar & boost::serialization::make_nvp("PartitionsInitialized", partitionsInitialized_);
        ar & boost::serialization::make_nvp("StartNodeID", startNodeID_);
        ar & boost::serialization::make_nvp("L1TrieStartNodeID", l1TrieStartNodeID_);

        ar & boost::serialization::make_nvp("TrieInitialized", trieInitialized_);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    bool load()
    {
        std::ifstream config(configPath_.c_str(), std::ifstream::in);
        if( config ) {
            boost::archive::xml_iarchive xml(config);
            try {
                xml >> boost::serialization::make_nvp("MtTrie", *this);
            } catch (...) {
                throw std::runtime_error( logHead() + "config file corrputed");
            }
            config.close();
            return true;
        }
        return false;
    }

    void sync()
    {
        std::string kNewConfigPath = configPath_ + ".new";

        // write to new config
        {
            std::ofstream config(kNewConfigPath.c_str(), std::ifstream::out);
            {
                boost::archive::xml_oarchive xml(config);
                xml << boost::serialization::make_nvp("MtTrie", *this);
                // flush archive to ofstream in ~xml
            }
            // flush to file in ~config
        }

        if (0 != ::rename(kNewConfigPath.c_str(), configPath_.c_str()))
        {
            throw std::runtime_error("Failed to rename " + kNewConfigPath);
        }
    }

private:

    /// @brief prefix of assoicated files' name.
    std::string name_;

    /// @brief load configuration from this file.
    std::string configPath_;

    /// @brief path of write cache file.
    std::string writeCachePath_;

    /// @brief file acts as write cache.
    ///        all inputs are cached in this file first,
    ///        then inserted into trie in executeTask().
    std::ofstream writeCache_;

    /// @brief collect given numbers of samples from input term stream.
    StreamSampler<StringType> sampler_;

    /// @brief how many partitions do we divide all input terms into.
    int partitionNum_;

    /// @brief Indicate whether boundaries have been determined.
    bool boundariesInitialized_;

    /// @brief boundaries between partitions, there are partitionNum-1 elements.
    std::vector<StringType> boundaries_;

    /// @brief Indicate whether partitions has been initialized.
    std::vector<bool> partitionsInitialized_;

    /// @brief next available NodeID of tries on each partition before insertions,
    ///        skip all previous NodeID when merging a partition trie into the final trie.
    ///        designed for supporting incremental updates. there are partitionNum_ elements.
    std::vector<TrieNodeIDType> startNodeID_;

    std::vector<TrieNodeIDType> l1TrieStartNodeID_;

    /// @brief Indicate whether the final trie has been initialized.
    bool trieInitialized_;

    /// @brief the final(major) trie, findRegExp are operating this trie.
    FinalTrieType trie_;

    /// @brief Cache of the final(major) trie, findRegExp are operating this trie first.
    /// @brief If enough candidates are gained from this trie, it will not search the major trie.
    FinalL1TrieType l1Trie_;

    /// @brief number of partitions that finishes building.
    int processedPartitions_;

    /// @brief Lock for correct printing between work threads
    ReadWriteLock printLock_;

    /// @brief number of work threads.
    int threadNum_;

    /// @brief work thread pool.
    boost::thread_group workerThreads_;
};

NS_IZENELIB_AM_END

#endif
