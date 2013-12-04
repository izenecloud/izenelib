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
typedef BTreeIndexer<KeyType> IndexerType;
typedef uint32_t docid_t;
typedef std::vector<docid_t> RefValueType;
typedef izenelib::ir::indexmanager::Compare<KeyType> CompareType;
typedef InMemoryBTreeIndexer<KeyType, docid_t> RefType;

public:


    BTreeTestRunner(const std::string& test_path, int max_key = 1000)
    :indexer_(test_path, "this_is_test")
    , min_docid_(1), max_docid_(10000000)
//     , low_(low), high_(high)
    , insert_ratio_(80), flush_ratio_(10), read_thread_count_(3), ge_only_(false)
    , write_limit_(10000000), write_count_(0), end_(false)
    , read_wait_ms_(1), max_key_(max_key)
    {

    }

    void start()
    {
        if(!indexer_.open())
        {
            throw std::runtime_error("indexer open error");
        }

        boost::thread twrite(boost::bind( &BTreeTestRunner::write_thread, this));
        std::vector<boost::thread* > rthreads;
        for(uint32_t i=0;i<read_thread_count_;++i)
        {
            boost::thread twrite(boost::bind( &BTreeTestRunner::read_thread, this));
            rthreads.push_back(&twrite);
        }
        twrite.join();
        for(uint32_t i=0;i<read_thread_count_;++i)
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
            RandomGenerator<KeyType>::Gen(0, max_key_, key);
            docid_t docid;
            RandomGenerator<docid_t>::Gen(min_docid_, max_docid_, docid);
            uint32_t ifi;
            RandomGenerator<uint32_t>::Gen(0, 99, ifi);
            boost::lock_guard<boost::shared_mutex> lock(mutex_);//add lock to prevent the inconsistent between indexer and ref
            if(ifi<insert_ratio_)
            {
                indexer_.add(key, docid);
                ref_.add(key, docid);
                //LOG(ERROR)<<"[W]1,"<<key<<","<<docid<<std::endl;
//                 record_.append(1, key, docid);
            }
            else
            {
                indexer_.remove(key, docid);
                ref_.remove(key, docid);
                //LOG(ERROR)<<"[W]2,"<<key<<","<<docid<<std::endl;
//                 record_.append(2, key, docid);
            }
            uint32_t iff;
            RandomGenerator<uint32_t>::Gen(0, 99, iff);
            if(iff<flush_ratio_)
            {
                //LOG(ERROR)<<"[W]flushing"<<std::endl;
                indexer_.flush();
                //LOG(ERROR)<<"[W]flushed"<<std::endl;
//                 std::cout<<"%%%flushed"<<std::endl;
            }
            ++write_count_;
            if(write_count_>=write_limit_)
            {
                end();
            }

            if(write_count_%1000==0)
            {
                LOG(ERROR)<<"[W]write count "<<write_count_<<std::endl;
            }
        }
    }

    void read_thread()
    {
        while(!end_)
        {
            {
                boost::shared_lock<boost::shared_mutex> lock(mutex_);
                bool result = false;
                if(!ge_only_)
                {
                    result = RandomReadTest<KeyType>::Test(indexer_, ref_);
                }
                else
                {
                    result = RandomReadTest<KeyType>::SimpleTest(indexer_, ref_);
                }
                if(!result)
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
    uint32_t read_thread_count_;
    bool ge_only_;
    std::size_t write_limit_;
    std::size_t write_count_;
    bool end_;
    OperateRecord<KeyType> record_;
    int read_wait_ms_;
    boost::shared_mutex mutex_;
    int max_key_;
};

template <class KeyType>
class BTreePerformanceRunner
{
typedef BTreeIndexer<KeyType> IndexerType;
typedef uint32_t docid_t;
typedef std::vector<docid_t> RefValueType;
typedef izenelib::ir::indexmanager::Compare<KeyType> CompareType;
typedef InMemoryBTreeIndexer<KeyType, docid_t> RefType;

public:


    BTreePerformanceRunner(const std::string& test_path, uint32_t init_docid, uint32_t min, uint32_t max, uint32_t search_thread_count)
    :indexer_(test_path, "this_is_test")
    , init_docid_(init_docid), min_(min), max_(max), search_thread_count_(search_thread_count)
    , end_(false)
    {

    }

    void start()
    {
        if(!indexer_.open())
        {
            throw std::runtime_error("indexer open error");
        }
        for(uint32_t docid=1;docid<=init_docid_;docid++)
        {
            KeyType key;
            RandomGenerator<KeyType>::Gen(min_, max_, key);
            indexer_.add(key, docid);
        }
        //indexer_.flush();

        boost::thread twrite(boost::bind( &BTreePerformanceRunner::write_thread, this));
        //{
            //int retcode;
            //int policy;
            //pthread_t thread_id = (pthread_t) twrite.native_handle();
            //struct sched_param param;
            //if ((retcode = pthread_getschedparam(thread_id, &policy, &param)) != 0)
            //{
                //std::cerr<<"pthread_getschedparam fail"<<std::endl;
                //exit(EXIT_FAILURE);
            //}
            //std::cout << "INHERITED: ";
            //std::cout << "policy=" << ((policy == SCHED_FIFO)  ? "SCHED_FIFO" :
             //(policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???")
                                            //<< ", priority=" << param.sched_priority << std::endl;
            //policy = SCHED_FIFO;
            //param.sched_priority = 1;
            //if ((retcode = pthread_setschedparam(thread_id, policy, &param)) != 0)
            //{
                //errno = retcode;
                //perror("pthread_setschedparam");
                //exit(EXIT_FAILURE);
            //}
        //}
        std::vector<boost::thread* > rthreads;
        for(uint32_t i=0;i<search_thread_count_;++i)
        {
            boost::thread twrite(boost::bind( &BTreePerformanceRunner::search_thread, this));
            rthreads.push_back(&twrite);
            //int retcode;
            //int policy;
            //pthread_t thread_id = (pthread_t) twrite.native_handle();
            //struct sched_param param;
            //if ((retcode = pthread_getschedparam(thread_id, &policy, &param)) != 0)
            //{
                //std::cerr<<"pthread_getschedparam fail"<<std::endl;
                //exit(EXIT_FAILURE);
            //}
            //policy = SCHED_FIFO;
            //param.sched_priority = 99;
            //if ((retcode = pthread_setschedparam(thread_id, policy, &param)) != 0)
            //{
                //perror("pthread_setschedparam");
                //exit(EXIT_FAILURE);
            //}
        }
        twrite.join();
        for(uint32_t i=0;i<search_thread_count_;++i)
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
        izenelib::util::ClockTimer clocker;
        double all_time = 0.0;
        std::size_t count=0;
        while(!end_)
        {
            KeyType key;
            RandomGenerator<KeyType>::Gen(min_, max_, key);
            docid_t docid;
            RandomGenerator<docid_t>::Gen(1, init_docid_*1.5, docid);
            uint32_t ifi;
            RandomGenerator<uint32_t>::Gen(0, 99, ifi);
            clocker.restart();
            if(ifi<80)
            {
                indexer_.add(key, docid);
            }
            else
            {
                indexer_.remove(key, docid);
            }
            double time = clocker.elapsed();
            all_time+=time;
            count++;
            if(count%1000==0)
            {
                LOG(ERROR)<<"write avg "<<all_time/count<<std::endl;
            }
            uint32_t iff;
            RandomGenerator<uint32_t>::Gen(0, 999, iff);
            if(iff<1)
            {
                indexer_.flush();
            }
        }
    }

    void search_thread()
    {
        izenelib::util::ClockTimer clocker;
        double all_time = 0.0;
        std::size_t count=0;
        std::size_t timeout_count=0;
        while(!end_)
        {
            KeyType key = min_;
            //RandomGenerator<KeyType>::Gen(min_, max_, key);
            Bitset docs;
            clocker.restart();
            indexer_.getValueGreat(key, docs);
            double time = clocker.elapsed();
            all_time+=time;
            count++;
            if(time>=3.0)
            {
                timeout_count++;
                LOG(ERROR)<<"timeout "<<time<<","<<timeout_count<<"-"<<count<<std::endl;
            }
            if(count%10==0)
            {
                LOG(ERROR)<<"search avg "<<all_time/count<<std::endl;
            }
        }
    }

private:
    IndexerType indexer_;
    uint32_t init_docid_;
    KeyType min_;
    KeyType max_;
    uint32_t search_thread_count_;
    bool end_;

};
#endif
