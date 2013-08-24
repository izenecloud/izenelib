#ifndef SF1R_SHARED_SLOW_COUNTER_DEF_H
#define SF1R_SHARED_SLOW_COUNTER_DEF_H

#include <stdlib.h>
#include <boost/interprocess/sync/named_upgradable_mutex.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <iostream>
#include <string>
#include <map>
#include <util/izene_serialization.h>
#include <glog/logging.h>

NS_IZENELIB_SF1R_BEGIN

#define SF1R_PROCESS_MUTEX_NAME "sf1r_named_mutex_for_process"
#define SF1R_PROCESS_SHARED_MEM_NAME "sf1r_shared_mem_for_process"
#define MAX_SHARED_SIZE 1024*32
#define SLOW_THRESHOLD 10
typedef std::map<std::string, size_t> Sf1rSharedSlowCounterT;


class SlowCounterMgr
{
public:
    static bool readSharedSlowCounter(Sf1rSharedSlowCounterT& ret)
    {
        using namespace boost::interprocess;
        try
        {
            named_upgradable_mutex mutex(open_only, SF1R_PROCESS_MUTEX_NAME);
            sharable_lock<named_upgradable_mutex> rlock(mutex, try_to_lock);

            if (!rlock.owns())
            {
                LOG(INFO) << "try lock failed.";
                return false;
            }
            shared_memory_object shm_obj(open_only, SF1R_PROCESS_SHARED_MEM_NAME, read_write);
            mapped_region region(shm_obj, read_write);

            ret.clear();
            void *addr = region.get_address();
            size_t data_size = (*(int*)addr);
            if (data_size == 0)
                return true;
            if (data_size >= MAX_SHARED_SIZE - sizeof(int))
            {
                LOG(ERROR) << "the data_size is larger than shared memory size :" << data_size;
                return false;
            }
            izenelib::util::izene_deserialization<Sf1rSharedSlowCounterT> izdeserial((const char*)addr + sizeof(int), data_size);
            izdeserial.read_image(ret);
        }
        catch(const std::exception& e)
        {
            LOG(ERROR) << "read shared memory failed : " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    static void clearSharedSlowCounter()
    {
        using namespace boost::interprocess;
        try
        {
            LOG(INFO) << "clearing shared memory.";
            named_upgradable_mutex mutex(open_only, SF1R_PROCESS_MUTEX_NAME);
            scoped_lock<named_upgradable_mutex> lock(mutex, try_to_lock);

            if (!lock.owns())
            {
                LOG(INFO) << "try lock failed.";
                return;
            }
            shared_memory_object shm_obj(open_only, SF1R_PROCESS_SHARED_MEM_NAME, read_write);
            mapped_region region(shm_obj, read_write);

            void *addr = region.get_address();
            (*(int*)addr) = 0;
            std::memset((char*)addr + sizeof(int), 0, region.get_size() - sizeof(int));
            region.flush(0, region.get_size());
        }
        catch(const std::exception& e)
        {
            LOG(ERROR) << "clear shared memory failed : " << e.what() << std::endl;
        }
    }

    static bool increSlowCounter(const std::string& path, Sf1rSharedSlowCounterT& ret_slow_counter)
    {
        using namespace boost::interprocess;
        try
        {
            named_upgradable_mutex mutex(open_only, SF1R_PROCESS_MUTEX_NAME);
            scoped_lock<named_upgradable_mutex> lock(mutex, try_to_lock);

            if (!lock.owns())
            {
                LOG(INFO) << "try lock failed.";
                return false;
            }
            shared_memory_object shm_obj(open_only, SF1R_PROCESS_SHARED_MEM_NAME, read_write);
            mapped_region region(shm_obj, read_write);

            Sf1rSharedSlowCounterT cur_shared_data;
            void *addr = region.get_address();
            size_t data_size = (*(int*)addr);
            if (data_size >= MAX_SHARED_SIZE - sizeof(int))
            {
                LOG(ERROR) << "the data_size is larger than shared memory size :" << data_size;
                return false;
            }
            if (data_size > 0)
            {
                izenelib::util::izene_deserialization<Sf1rSharedSlowCounterT> izdeserial((const char*)addr + sizeof(int), data_size);
                izdeserial.read_image(cur_shared_data);
            }
            std::pair<Sf1rSharedSlowCounterT::iterator,bool> ret;
            ret = cur_shared_data.insert(std::pair<std::string, size_t>(path, 0));
            if (ret.first->second <= SLOW_THRESHOLD + 1)
                (ret.first->second)++;
            if (ret.first->second >= SLOW_THRESHOLD)
                LOG(INFO) << "node : " << path << " , slow counter up to : " << ret.first->second; 
            char* buf;
            izenelib::util::izene_serialization<Sf1rSharedSlowCounterT> izs(cur_shared_data);
            izs.write_image(buf, data_size);
            if (data_size >= MAX_SHARED_SIZE - sizeof(int))
            {
                LOG(WARNING) << "shared data is too large to write." << data_size;
                return false;
            }
            (*(int*)addr) = (int)data_size;
            memcpy((char*)addr + sizeof(int), buf, data_size);
            region.flush(0, region.get_size());
            ret_slow_counter = cur_shared_data;
        }
        catch(const std::exception& e)
        {
            LOG(ERROR) << "write shared memory failed : " << e.what() << std::endl;
            return false;
        }
        return true;
    }
};

NS_IZENELIB_SF1R_END

#endif
