/**
 * @brief A high-performance and multi-threaded trie implementation based on Hdb.
 *        Incremental update is also supported.
 *        See TR "A high-performance and multi-threaded trie based on HdbTrie" for details.
 * @author Wei Cao
 * @date 2009-12-1
 */

#ifndef _MT_TRIE_H_
#define _MT_TRIE_H_

#include <iostream>
#include <fstream>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/nvp.hpp>

#include "sampler.h"
#include "partition_trie.hpp"

NS_IZENELIB_AM_BEGIN

template <typename StringType>
class MtTrie
{
public:
    typedef uint64_t TrieNodeIDType;

public:
    /**
     *  @param partitionNum - how many partitions do we divide all input terms into.
     */
    MtTrie(const std::string& name, const int partitionNum)
    :   name_(name), configPath_(name_ + ".config.xml"),
        writeCachePath_(name_+".writecache"),
        trie_(name_ + ".trie")
    {
        if( partitionNum <= 0 || partitionNum > 256 ) {
            throw std::runtime_error( logHead() + "wrong partitionNum,\
                should between 0 and 256");
        }

        std::ifstream config(configPath_.c_str(), std::ifstream::in);
        if( config ) {
            boost::archive::xml_iarchive xml(config);
            try {
                xml >> boost::serialization::make_nvp("MtTrie", *this);
            } catch (...) {
                throw std::runtime_error( logHead() + "config file corrputed");
            }
            config.close();

            if(partitionNum_ != partitionNum)
                throw std::runtime_error( logHead() + "inconsistent partitionNum");
        }

        partitionNum_ = partitionNum;

        needSample_ = false;
        if(boundaries_.size() != (size_t)(partitionNum_-1) )
            needSample_ = true;

        skipNodeID_.resize(partitionNum_);
    }

    void open()
    {
        writeCache_.open(writeCachePath_.c_str(), std::ofstream::out|
            std::ofstream::binary | std::ofstream::app );
        if(writeCache_.fail())
            std::cerr << logHead() << " fail to open write cache" << std::endl;

        trie_.open();
    }

    void close()
    {
        writeCache_.close();
        trie_.close();
    }

    void sync()
    {
        std::ofstream config(configPath_.c_str(), std::ifstream::out);
        boost::archive::xml_oarchive xml(config);
        xml << boost::serialization::make_nvp("MtTrie", *this);
        config.flush();
    }

    void flush()
    {
        sync();
        writeCache_.flush();
        trie_.flush();
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

             if(needSample_) {
                sampler_.collect(term);
             }
        } catch(const std::exception & e) {
            std::cerr << logHead() << " fail to update writecache" << std::endl;
//            writeCache_.close();
//            writeCache_.open(writeCachePath_.c_str(), std::ofstream::out|
//                std::ofstream::binary|std::ofstream::app);
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

        if( needSample_ ) {
            std::cout << logHead() << "start computing boundaries" << std::endl;

            std::set<StringType> samples;
            sampler_.getSamples(samples);

            std::cout << logHead() << samples.size() << " smaples are selected" << std::endl;

            boundaries_.clear();
            if( samples.size() > (size_t)(partitionNum_-1) ) {
                // caculate boundaries between partitions
                size_t interval = samples.size()/partitionNum_;

                int count = 0;
                typename std::set<StringType>::iterator it =
                    samples.begin();
                while(count < partitionNum_-1) {
                    for(size_t i=0; i<interval; i++)
                        it++;
                    boundaries_.push_back(*it);
                    //std::cout << *it << std::endl;
                    count ++;
                }
            } else {
                std::cout << logHead() << "number of terms isn't enough to be partitioned to "
                    << partitionNum_ << " partitions" << std::endl;
                return;
            }

            needSample_ = false;

            sync();
        } else {
            std::cout << logHead() << "skip computing boundareis, already exist"
                << std::endl;
        }

        std::cout << logHead() << "start spliting inputs" << std::endl;

        writeCache_.flush();
        writeCache_.close();

        splitInput();

        remove( writeCachePath_.c_str() );
        writeCache_.open(writeCachePath_.c_str(), std::ofstream::out | std::ofstream::binary );

        std::cout << logHead() << "Start building tries with " << threadNum_
            << " threads on " << partitionNum_ << " partitions" << std::endl;

        // multi-threaded building tries on partitions
        for( int i=0; i < threadNum_; i++ )
          workerThreads_.create_thread(boost::bind(&MtTrie::taskBody, this, i) );
        workerThreads_.join_all();

        // merging tries on partitions into one
        std::cout << logHead() << "Start merge tries " << std::endl;
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
        int maximumResultNumber = 100)
    {
        return trie_.findRegExp(regexp, keyList, maximumResultNumber);
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

        while(!input.eof())
        {
            input.read((char*)&buffersize, sizeof(int));
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
        }
        input.close();
        for(int i = 0; i<partitionNum_; i++ ) {
            output[i].close();
        }
        delete[] output;
    }

    void taskBody(int threadId) {
        int partitionId = threadId;
        while(partitionId < partitionNum_) {
            std::string inputName = getPartitionName(partitionId) + ".input";
            std::string trieName = getPartitionName(partitionId) + ".trie";

            std::ifstream input;
            input.open(inputName.c_str(), std::ifstream::in|
                std::ifstream::binary );
            if(input.fail())
                throw std::runtime_error("failed open input for "
                        + getPartitionName(partitionId));

            PartitionTrie2<StringType> trie(trieName);
            trie.open();

            // Initialize trie at the first time.
            if(trie.num_items() == 0) {
                // Insert all boundaries into trie, to ensure NodeIDs are consistent
                // in all tries, the details of reasoning and proof see MtTrie TR.
                for(size_t i =0; i<boundaries_.size(); i++) {
                    trie.insert(boundaries_[i]);
                }
                trie.setbase(getPartitionStartNodeID(partitionId));
            }
            skipNodeID_[partitionId] = trie.nextNodeID();

            // Insert all terms in this partition
            StringType term;
            int buffersize = 0;
            char charBuffer[256];
            while(!input.eof())
            {
                input.read((char*)&buffersize, sizeof(int));
                input.read( charBuffer, buffersize );
                term.assign( std::string(charBuffer,buffersize) );
                trie.insert(term);
            }
            trie.optimize();
            trie.close();
            input.close();
            remove( inputName.c_str() );
            partitionId += threadNum_;
        }
    }

    void mergeTries() {

        // Initialize trie at the first time.
        if(trie_.num_items() == 0) {
            for(size_t i =0; i<boundaries_.size(); i++) {
                trie_.insert(boundaries_[i]);
            }
        }

        for(int i=0; i<partitionNum_; i++) {
            std::string p = getPartitionName(i) + ".trie";
            PartitionTrie2<StringType> pt(p);
            pt.open();
            trie_.concatenate(pt, skipNodeID_[i]);
            pt.close();
        }

        trie_.optimize();
        trie_.flush();
    }

protected:


    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::make_nvp("PartitionNumber", partitionNum_);

        std::vector<std::string> buffer;
        for(size_t i =0; i< boundaries_.size(); i++ ) {
            buffer.push_back( std::string((char*)boundaries_[i].c_str(),
                    boundaries_[i].size()) );
        }
        ar & boost::serialization::make_nvp("Boundaries", buffer);

        ar & boost::serialization::make_nvp("StreamSampler", sampler_);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::make_nvp("PartitionNumber", partitionNum_);

        std::vector<std::string> buffer;
        ar & boost::serialization::make_nvp("Boundaries", buffer);
        for(size_t i =0; i< buffer.size(); i++ ) {
            boundaries_.push_back( StringType(buffer[i]) );
        }

        ar & boost::serialization::make_nvp("StreamSampler", sampler_);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()


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

    /// @brief how many partitions do we divide all input terms into.
    int partitionNum_;

    /// @brief boundaries between partitions, there are partitionNum-1 elements.
    std::vector<StringType> boundaries_;

    /// @brief whether we need sample inputs.
    bool needSample_;

    /// @brief collect given numbers of samples from input term stream.
    StreamSampler<StringType> sampler_;

    /// @brief next available NodeID of tries on each partition before insertions,
    ///        skip all previous NodeID when merging a partition trie into the final trie.
    ///        designed for supporting incremental updates. there are partitionNum_ elements.
    std::vector<TrieNodeIDType> skipNodeID_;

    /// @brief number of work threads.
    int threadNum_;

    /// @brief work thread pool.
    boost::thread_group workerThreads_;

    /// @brief the final(major) trie, findRegExp are operating this trie.
    PartitionTrie2<StringType, TrieNodeIDType> trie_;

};

NS_IZENELIB_AM_END

#endif
