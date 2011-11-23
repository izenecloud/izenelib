
#ifndef IZENELIB_IR_BTREETESTFRAMEWORK_H_
#define IZENELIB_IR_BTREETESTFRAMEWORK_H_

#include <exception>
#include <ir/index_manager/index/rtype/BTreeIndexer.h>
#include <ir/index_manager/index/rtype/InMemoryBTreeIndexer.h>
#include <boost/thread.hpp>
#include "RandomGenerator.h"
#include "RandomReadTest.h"
#include "OperateRecord.h"
using namespace izenelib::ir::indexmanager;

template <class KeyType>
class BTreeTestRunner
{
typedef CBTreeIndexer<KeyType> IndexerType;
typedef uint32_t docid_t;
typedef std::vector<docid_t> RefValueType;
typedef izenelib::ir::indexmanager::Compare<KeyType> CompareType;
typedef InMemoryBTreeIndexer<KeyType, docid_t> RefType;
    
public:
    
    
    BTreeTestRunner(const std::string& test_path)
    :indexer_(test_path, "this_is_test")
    , min_docid_(1), max_docid_(100)
//     , low_(low), high_(high)
    , insert_ratio_(80), flush_ratio_(5)
    , write_limit_(10000000), write_count_(0), end_(false)
    , read_wait_ms_(1)
    {
        
    }
    
    void start()
    {
        if(!indexer_.open())
        {
            throw std::runtime_error("indexer open error");
        }
        
        boost::thread twrite(boost::bind( &BTreeTestRunner::write_thread, this)); 
        uint32_t read_thread_count = 1;
        std::vector<boost::thread* > rthreads;
        for(uint32_t i=0;i<read_thread_count;++i)
        {
            boost::thread twrite(boost::bind( &BTreeTestRunner::read_thread, this));
            rthreads.push_back(&twrite);
        }
        twrite.join();
        for(uint32_t i=0;i<read_thread_count;++i)
        {
            rthreads[i]->join();
        }
        indexer_.close();
    }
    
    void end()
    {
        end_ = true;
    }
    
    void write_thread()
    {
        while(!end_)
        {
            KeyType key;
            RandomGenerator<KeyType>::Gen(key);
            docid_t docid;
            RandomGenerator<docid_t>::Gen(min_docid_, max_docid_, docid);
            uint32_t ifi;
            RandomGenerator<uint32_t>::Gen(0, 99, ifi);
            boost::lock_guard<boost::shared_mutex> lock(mutex_);//add lock to prevent the inconsistent between indexer and ref
            if(ifi<insert_ratio_)
            {
                indexer_.add(key, docid);
                ref_.add(key, docid);
                std::cout<<"[W]1,"<<key<<","<<docid<<std::endl;
//                 record_.append(1, key, docid);
            }
            else
            {
                indexer_.remove(key, docid);
                ref_.remove(key, docid);
                std::cout<<"[W]2,"<<key<<","<<docid<<std::endl;
//                 record_.append(2, key, docid);
            }
            uint32_t iff;
            RandomGenerator<uint32_t>::Gen(0, 99, iff);
            if(iff<flush_ratio_)
            {
                std::cout<<"[W]flushing"<<std::endl;
                indexer_.flush();
                std::cout<<"[W]flushed"<<std::endl;
//                 std::cout<<"%%%flushed"<<std::endl;
            }
            ++write_count_;
            if(write_count_>=write_limit_)
            {
                end();
            }
            
            if(write_count_%100==0)
            {
                std::cout<<"[W]write count "<<write_count_<<std::endl;
            }
        }
    }
    
    void read_thread()
    {
        while(!end_)
        {
            {
                boost::shared_lock<boost::shared_mutex> lock(mutex_);
                if(!RandomReadTest<KeyType>::Test(indexer_, ref_))
                {
                    //do sth.
    //                 std::cout<<"Read test failed, recode : "<<std::endl;
    //                 std::cout<<record_<<std::endl;
    //                 record_.clear();
                    throw std::runtime_error("test failed");
                }
            }
            if(read_wait_ms_>0)
            {
                boost::this_thread::sleep( boost::posix_time::milliseconds(read_wait_ms_) );
            }
        }
    }
    
private:
    IndexerType indexer_;
    RefType ref_;
    docid_t min_docid_;
    docid_t max_docid_;
    KeyType low_;
    KeyType high_;
    uint32_t insert_ratio_; // 0-100 means 0% - 100%
    uint32_t flush_ratio_; // 0-100 means 0% - 100%
    std::size_t write_limit_;
    std::size_t write_count_;
    bool end_;
    OperateRecord<KeyType> record_;
    int read_wait_ms_;
    boost::shared_mutex mutex_;
};

#endif

