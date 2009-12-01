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
    :   name_(name), partitionNum_(partitionNum);
        writeCacheName_(name_+".writecache"),
        trie_(name_ + ".trie"),
        fileLines_(0)
    {
        if( partitionNum_ <= 0 || partitionNum_ > 256 ) {
            throw std::runtime_error( logHead() +
                "wrong partitionNum, should between 0 and 256");
        }
        partitionState_.resize(partitionNum_);
    }

    void open()
    {
        writeCache_.open(writeCacheName_.c_str(), std::ofstream::out|
            std::ofstream::binary );
        if(writeCache_.fail())
            std::cerr << logHead() << " fail to open write cache" << std::endl;

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
             writeCache_.write((const char*)term.c_str(), word.size());
             fileLines_ ++;
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
        if( partitionNum_ < threadNum_ ) {
            std::cout << logHead() << " threadNum(" << threadNum << ") is less than partitionNum("
                << partitionNum_ << ", shrink it" << std::end;
            threadNum_ = partitionNum_;
        }

        writeCache_.flush();
        writeCache_.close();

        // Sample 1000 records to estimate segments.
        std::cout << logHead() << "Start preprocess" << std::endl;
        preprocess();

        std::cout << logHead() << "Start building tries with " << threadNum_
            << " threads on " << partitionNum_ << " partitions" << std::endl;
        // multi-threaded building tries on partitions
        for( int i=0; i < threadNum_; i++ )
          workerThreads_.create_thread(boost::bind(&MtTrie::taskBody, this, i) );
        workerThreads_.join_all();

        // merging tries on partitions into one
        std::cout << logHead() << "Start merge tries " << std::endl;
        mergeTries();

        fileLines_ = 0;
        writeCache_.open(writeCacheName_.c_str(), std::ofstream::out|
            std::ofstream::binary );
//            std::ofstream::trunc|std::ofstream::binary );
        if(writeCache_.fail())
            std::cerr << logHead() << "fail to reopen write cache" << std::endl;
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
        return partitionId * (1 << (sizeof(TrieNodeIDType)*8 - 8));
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
        std::set<StringType> samples;

        std::ifstream input;
        input.open(writeCacheName_.c_str(), std::ofstream::in|std::ofstream::binary );
        if(input.fail())
            throw std::runtime_error("failed open file " + writeCacheName_);

        std::cout << "MtTrie["<< name_ << "] input file contains " << fileLength_
            << " bytes, " << fileLines_ << " words" << std::endl;

        int sampleStep = 1 + (line/sampleNumber);
        std::cout << "MtTrie["<< name_ << "] extract a sample for every "
            << sampleStep << " words" << std::endl;

        line = 0;
        buffersize = 0;
        char charBuffer[256];
        StringType term;

        while(!input.eof())
        {
            input.read((char*)&buffersize, sizeof(int));
            if( line%sampleStep || buffersize > 256 ) {
                input.seekg(buffersize, ifstream::cur);
            } else {
                // selected as a sample
                input.read( charBuffer, buffersize );
                term.assign( std::string(charBuffer,buffersize) );
                if(term.size() > 0) {
                    ///TODO
                    samples.insert(term);
                }
            }
            line ++;
        }
        input.close();

        std::cout << "MtTrie["<< name_ << "] preporcessing.. " << samples.size()
            << " smaples are selected" << std::endl;

        if(samples.size()) {
            // caculate boundaries between partitions
            size_t interval = samples.size()/partitionNum_;
            int count = 0;
            typename std::set<StringType>::iterator it = samples.begin();

            while(count < partitionNum_-1) {
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
        input.open(writeCacheName_.c_str(), std::ifstream::in|std::ifstream::binary );
        if(input.fail())
            throw std::runtime_error("failed open write cache " + writeCacheName_);

        std::ofstream* output = new std::ofstream[partitionNum_];
        for(int i = 0; i<partitionNum_; i++ ) {
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
                if(partitionNum_ == 1) {
                    output[0].write((char*)&buffersize, sizeof(int));
                    output[0].write(charBuffer, buffersize);
                } else {
                    if(term.compare(boundaries_[0]) < 0) {
                        output[0].write((char*)&buffersize, sizeof(int));
                        output[0].write(charBuffer, buffersize);
                    } else if(term.compare(boundaries_[boundaries_.size()-1]) >= 0) {
                        output[boundaries_.size()].write((char*)&buffersize, sizeof(int));
                        output[boundaries_.size()].write(charBuffer, buffersize);
                    } else {
                        for(size_t i=1; i<boundaries_.size(); i++) {
                            int c1 = term.compare(boundaries_[i-1]);
                            int c2 = term.compare(boundaries_[i]);
                            if(c1 >=0 && c2<0 ) {
                                output[i].write((char*)&buffersize, sizeof(int));
                                output[i].write(charBuffer, buffersize);
                                break;
                            }
                        }
                    }
                }
            }
        }
        input.close();
        for(int i = 0; i<partitionNum_; i++ ) {
            output[i].close();

            std::string outputName = getPartitionName(i) + ".input";
            remove( outputName.c_str() );
        }
        delete[] output;
    }

    void preprocess() {
        if(boundaries_.size() == 0) {
            std::cout << logHead() << "start computing boundaries" << std::endl;
            computeBoundaries(1000);
        } else {
            std::cout << logHead() << "skip computing boundareis" << std::endl;
        }

        std::cout << logHead() << "start spliting inputs" << std::endl;
        splitInput();

        // Initialize trie at the first time.
        if(trie_.num_items() == 0) {
            for(size_t i =0; i<boundaries_.size(); i++) {
                trie_.insert(boundaries_[i]);
            }
            overlappingNodeID_ = trie_.nextNodeID();
        }
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

            PartitionTrie2<StringType> trie(trieName,
                getPartitionStartNodeID(partitionId));
            trie.open();

            // Initialize trie at the first time.
            if(trie.num_items() == 0) {
                // Insert all boundaries into trie, to ensure NodeIDs are consistent
                // in all tries, the details of reasoning and proof see MtTrie TR.
                trie.concatenate(trie_);
            }
            partitionState_[i] = trie.nextNodeID();

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

            trie.close();
            input.close();
            partitionId += threadNum_;
        }
    }

    void mergeTries() {

        for(int i=0; i<partitionNum_; i++) {
            std::string p = getPartitionName(i) + ".trie";
            PartitionTrie2<StringType> pt(p);
            pt.open();
            trie_.concatenate(pt, partitionState_[i]);
            pt.close();
        }

        trie_.optimize();
    }

private:

    std::string name_;

    std::string writeCacheName_;

    std::ofstream writeCache_;

    /// @brief how many partitions do we divide all input terms into.
    int partitionNum_;

    /// @brief boundaries between partitions, there are partitionNum_-1 elements.
    std::vector<StringType> boundaries_;

    /// @brief how many term occurences.
    size_t fileLines_;

    int threadNum_;

    boost::thread_group workerThreads_;

    std::vector<TrieNodeIDType> partitionState_;

    PartitionTrie2<StringType, TrieNodeIDType> trie_;

};

NS_IZENELIB_AM_END

#endif
