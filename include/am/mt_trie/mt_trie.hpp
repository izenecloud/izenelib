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

#include "mt_trie_header.h"
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
    :   name_(name),
        writeCachePath_(name_+".writecache"),
        header_(name_ + ".header.xml"),
        trie_(name_ + ".trie")
    {
        if( partitionNum <= 0 || partitionNum > 256 ) {
            throw std::runtime_error( logHead() +
                "wrong partitionNum, should between 0 and 256");
        }
        if(!header_.exist()) {
            header_.partitionNum = partitionNum;
            header_.flush();
        } else {
            if(header_.partitionNum != partitionNum)
                throw std::runtime_error( logHead() +
                    "inconsistent partitionNum");
            boundaries_ = header_.boundaries;
        }

        needSample_ = false;
        if(boundaries_.size() != (size_t)(partitionNum-1) ) {
            boundaries_.clear();
            needSample_ = true;
            sampleStep_ = 1;
            sampleCounter_ = 0;
        }

        skipNodeID_.resize(partitionNum);
    }

    void open()
    {
        writeCache_.open(writeCachePath_.c_str(), std::ofstream::out|
            std::ofstream::binary );
        if(writeCache_.fail())
            std::cerr << logHead() << " fail to open write cache" << std::endl;

        writeCacheLine_ = 0;

        trie_.open();
    }

    void close()
    {
        writeCache_.close();
        trie_.close();
    }

    void flush()
    {
        writeCache_.flush();
        header_.flush();
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
                 sampleCounter_ ++;
                 if(sampleCounter_ == sampleStep_) {
                    size_t measure = writeCacheLine_-lastSampleLine_;

                    // havn't collect enough samples from sequence
                    if(samples_.size() < sampleNum_) {
                        samples_.insert(std::make_pair(writeCacheLine_ , term));
                        associations_.insert(std::make_pair(writeCacheLine_ ,  mesures_.insert(
                            std::make_pair(measure, writeCacheLine_) )));
                        lastSampleLine_ = writeCacheLine_;
                        sampleCounter_ = 0;
                    } else {
                        //  replace existing samples with new one
                        size_t existingLeastMeasure = mesures_.begin()->first;

                        if( existingLeastMeasure < measure ) {
                            // step 1. erase replaced sample from samples_
                            size_t replacedLine = mesures_.begin()->second;
                            samples_.erase(replacedLine);
                            mesures_.erase(mesures_.begin());
                            associations_.erase(replacedLine);

                            // step 2. add measure of replaced line to the next line
                            size_t updatedLine = samples_.upper_bound(replacedLine)->first;
                            size_t updatedMeasure = existingLeastMeasure +
                                associations_[updatedLine]->first;
                            mesures_.erase(associations_[updatedLine]);
                            associations_.erase(updatedLine);
                            associations_.insert(std::make_pair( updatedLine, mesures_.insert(
                                std::make_pair(updatedMeasure, updatedLine) )));

                            // step 3. insert new line
                            samples_.insert(std::make_pair(writeCacheLine_, term));
                            associations_.insert(std::make_pair( writeCacheLine_, mesures_.insert(
                                std::make_pair(measure, writeCacheLine_))));
                            lastSampleLine_ = writeCacheLine_;
                        } else {
                            sampleStep_ *= 2;
                        }
                    }
                 }
             }

             writeCacheLine_ ++;
        } catch(const std::exception & e) {
            std::cerr << logHead() << " write cache locked" << std::endl;
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
        if( header_.partitionNum < threadNum_ ) {
            std::cout << logHead() << " threadNum(" << threadNum << ") is less than partitionNum("
                << header_.partitionNum << ", shrink it" << std::endl;
            threadNum_ = header_.partitionNum;
        }

        writeCache_.flush();
        writeCache_.close();

        if( boundaries_.size() != (size_t)(header_.partitionNum-1) ) {
            std::cout << logHead() << "start computing boundaries" << std::endl;

            boundaries_.clear();
            computeBoundaries(1000);
            if(boundaries_.size() == (size_t)(header_.partitionNum-1) ) {
                header_.boundaries = boundaries_;
                header_.flush();
                needSample_ = false;
            } else {
                std::cout << logHead() << "number of terms isn't enough to be partitioned to "
                    << header_.partitionNum << " partitions" << std::endl;
                return;
            }
        } else {
            std::cout << logHead() << "skip computing boundareis, already exist"
                << std::endl;
        }

        std::cout << logHead() << "start spliting inputs" << std::endl;
        splitInput();

        std::cout << logHead() << "Start building tries with " << threadNum_
            << " threads on " << header_.partitionNum << " partitions" << std::endl;
        // multi-threaded building tries on partitions
        for( int i=0; i < threadNum_; i++ )
          workerThreads_.create_thread(boost::bind(&MtTrie::taskBody, this, i) );
        workerThreads_.join_all();

        // merging tries on partitions into one
        std::cout << logHead() << "Start merge tries " << std::endl;
        mergeTries();

        writeCacheLine_ = 0;
        writeCache_.open(writeCachePath_.c_str(), std::ofstream::out|
            std::ofstream::binary );
        if(writeCache_.fail())
            std::cerr << logHead() << "fail to reopen write cache" << std::endl;
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
     * @brief Skip over all input terms and select some samples to get
     *        statistical information such as file's length,
     *        how many terms it contains, how to split this file
     *        to allow multiple threads rescan it at the same time.
     *        how to divide distinct terms into partitions, and so on.
     */
    void computeBoundaries(int sampleNumber) {

        std::cout << "MtTrie["<< name_ << "] preporcessing.. " << samples_.size()
            << " smaples are selected" << std::endl;

        std::set<StringType> sortedSamples;
        {
            for( typename std::map<size_t, StringType>::iterator it =
                samples_.begin(); it != samples_.end(); it++ ) {
                sortedSamples.insert(it->second);
            }
        }

        if(sortedSamples.size()) {
            // caculate boundaries between partitions
            size_t interval = sortedSamples.size()/header_.partitionNum;

            int count = 0;
            typename std::set<StringType>::iterator it =
                sortedSamples.begin();
            while(count < header_.partitionNum-1) {
                for(size_t i=0; i<interval; i++)
                    it++;
                boundaries_.push_back(*it);
                //std::cout << *it << std::endl;
                count ++;
            }
        }
    }

    /**
     * @brief Split orignal input files into multiple partitions.
     */
    void splitInput() {

        std::ifstream input;
        input.open(writeCachePath_.c_str(), std::ifstream::in|std::ifstream::binary );
        if(input.fail())
            throw std::runtime_error("failed open write cache " + writeCachePath_);

        std::ofstream* output = new std::ofstream[header_.partitionNum];
        for(int i = 0; i<header_.partitionNum; i++ ) {
            std::string outputName = getPartitionName(i) + ".input";
            output[i].open(outputName.c_str(), std::ofstream::out|std::ofstream::binary);
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
        for(int i = 0; i<header_.partitionNum; i++ ) {
            output[i].close();
        }
        delete[] output;
    }

    void taskBody(int threadId) {
        int partitionId = threadId;
        while(partitionId < header_.partitionNum) {
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

        for(int i=0; i<header_.partitionNum; i++) {
            std::string p = getPartitionName(i) + ".trie";
            PartitionTrie2<StringType> pt(p);
            pt.open();
            trie_.concatenate(pt, skipNodeID_[i]);
            pt.close();
        }

        trie_.optimize();
        trie_.flush();
    }

private:

    /// @brief prefix of assoicated files' name.
    std::string name_;

    /// @brief path of write cache file.
    std::string writeCachePath_;

    /// @brief file acts as write cache.
    ///        all inputs are cached in this file first,
    ///        then inserted into trie in executeTask().
    std::ofstream writeCache_;

    /// @brief boundaries between partitions, there are partitionNum-1 elements.
    std::vector<StringType> boundaries_;

    /// @brief whether we need sample inputs.
    bool needSample_;

    /// @brief number of samples that need to collect.
    const static size_t sampleNum_ = 1000;

    /// @brief for implementing sampling at insert-time
    size_t sampleStep_;
    size_t sampleCounter_;
    size_t writeCacheLine_;
    size_t lastSampleLine_;
    std::map<size_t, StringType> samples_;
    std::multimap<size_t, size_t> mesures_;
    std::map<size_t, std::multimap<size_t, size_t>::iterator> associations_;

    /// @brief store persistent states of MtTrie.
    MtTrieHeader<StringType> header_;

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
