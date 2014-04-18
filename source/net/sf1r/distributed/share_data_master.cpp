#include <boost/interprocess/sync/named_upgradable_mutex.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <string>
#include <map>
#include <util/izene_serialization.h>
#include <iostream>
#include <glog/logging.h>
#include <signal.h>

using namespace std;
using namespace boost;

#define SF1R_PROCESS_MUTEX_NAME "sf1r_named_mutex_for_process"
#define SF1R_PROCESS_SHARED_MEM_NAME "sf1r_shared_mem_for_process"
#define MAX_SHARED_SIZE 1024*32
#define SLOW_THRESHOLD 10
typedef std::map<std::string, size_t> Sf1rSharedSlowCounterT;

static volatile bool s_stop = false;
static sigset_t maskset;
static void* sig_thread(void *arg)
{
    sigset_t *set = (sigset_t *)arg;
    int s,sig;
    for(;;){
        s = sigwait(set, &sig);
        if(s != 0)
        {
            exit(1);
        }
        switch(sig)
        {
        case SIGINT:
        case SIGHUP:
        case SIGQUIT:
        case SIGABRT:
        case SIGTERM:
            std::cout << "got interrupt signal, begin quit" << std::endl;
            s_stop = true;
            break;
        default:
            break;
        }
    }
}

void init_sig_env()
{
    sigemptyset(&maskset);
    sigaddset(&maskset, SIGINT);
    sigaddset(&maskset, SIGHUP);
    sigaddset(&maskset, SIGQUIT);
    sigaddset(&maskset, SIGABRT);
    sigaddset(&maskset, SIGTERM);

    int ret;
    ret = pthread_sigmask(SIG_BLOCK, &maskset, NULL);
    if(ret != 0)
    {
        perror("failed to block signal");
        exit(1);
    }

    pthread_t thread;
    ret = pthread_create(&thread, NULL, &sig_thread, (void*)&maskset);
    if(ret != 0)
    {
        perror("failed to create signal handle thread");
        exit(1);
    }
}

void decreSlowCounterForAll()
{
    // decrease the slow counter period.
    using namespace boost::interprocess;
    bool is_locked = false;
    try
    {
        named_upgradable_mutex mutex(open_only, SF1R_PROCESS_MUTEX_NAME);
        LOG(INFO) << "write lock begin : ";
        scoped_lock<named_upgradable_mutex> lock(mutex);

        is_locked = true;
        LOG(INFO) << "write lock success.";
        shared_memory_object shm_obj(open_only, SF1R_PROCESS_SHARED_MEM_NAME, read_write);
        mapped_region region(shm_obj, read_write);

        Sf1rSharedSlowCounterT cur_shared_data;
        void *addr = region.get_address();
        size_t data_size = (*(int*)addr);
        if (data_size == 0)
            return;
        if (data_size >= MAX_SHARED_SIZE - sizeof(int))
        {
            LOG(ERROR) << "the data_size is larger than shared memory size :" << data_size;
            (*(int*)addr) = 0;
            std::memset((char*)addr + sizeof(int), 0, region.get_size() - sizeof(int));
            region.flush(0, region.get_size());
            return;
        }
        izenelib::util::izene_deserialization<Sf1rSharedSlowCounterT> izdeserial((const char*)addr + sizeof(int), data_size);
        izdeserial.read_image(cur_shared_data);
        Sf1rSharedSlowCounterT::iterator it;
        it = cur_shared_data.begin();
        while(it != cur_shared_data.end())
        {
            if (it->second > 0)
                (it->second)--;
            ++it;
        }
        LOG(INFO) << "current slow counter size :" << cur_shared_data.size();

        char* buf;
        izenelib::util::izene_serialization<Sf1rSharedSlowCounterT> izs(cur_shared_data);
        izs.write_image(buf, data_size);
        if (data_size >= MAX_SHARED_SIZE - sizeof(int))
        {
            LOG(ERROR) << "shared data is too large to write." << std::endl;
            return;
        }
        (*(int*)addr) = (int)data_size;
        memcpy((char*)addr + sizeof(int), buf, data_size);
        region.flush(0, region.get_size());
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << "write shared memory failed : " << e.what() << std::endl;
        try
        {
            if (!is_locked)
            {
                LOG(ERROR) << "the lock may corrupt!";
                named_upgradable_mutex::remove(SF1R_PROCESS_MUTEX_NAME);
                named_upgradable_mutex mutex(create_only, SF1R_PROCESS_MUTEX_NAME);
            }
            else
            {
                LOG(ERROR) << "the shared memory may corrupt!";
                shared_memory_object::remove(SF1R_PROCESS_SHARED_MEM_NAME);
                shared_memory_object shm_obj(create_only, SF1R_PROCESS_SHARED_MEM_NAME, read_write);
                shm_obj.truncate(MAX_SHARED_SIZE);
            }
        }catch(const std::exception& e2){
            LOG(ERROR) << "try fix shared memory failed: " << e2.what();
        }
        return;
    }
}

void checking_func()
{
    while(!s_stop)
    {
        struct timespec req, rem;
        req.tv_sec = 20;
        req.tv_nsec = 0;

        while(0 != nanosleep(&req, &rem))
        {
            if (errno == EINTR)
                req = rem;
            else
            {
                perror("sleep error.");
                break;
            }
        }
        LOG(INFO) << "decreasing slow counter period." << std::endl;
        decreSlowCounterForAll();
    }
}

int
main(int argc, char *argv[])
{
    init_sig_env();
    using namespace boost::interprocess;
    shared_memory_object::remove(SF1R_PROCESS_SHARED_MEM_NAME);
    named_upgradable_mutex::remove(SF1R_PROCESS_MUTEX_NAME);

    shared_memory_object shm_obj(create_only, SF1R_PROCESS_SHARED_MEM_NAME, read_write);
    shm_obj.truncate(MAX_SHARED_SIZE);
    
    {
        mapped_region region(shm_obj, read_write);
        LOG(INFO) << "init shared memory size : " << region.get_size();
    }
    named_upgradable_mutex mutex(create_only, SF1R_PROCESS_MUTEX_NAME);

    LOG(INFO) << "shared memory master started. ";
    boost::thread checking_thread(boost::bind(&checking_func));
    checking_thread.join();
    LOG(INFO) << "shared data master quiting." << std::endl;
    shared_memory_object::remove(SF1R_PROCESS_SHARED_MEM_NAME);
    named_upgradable_mutex::remove(SF1R_PROCESS_MUTEX_NAME);
    return 0;
}
