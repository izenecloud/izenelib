/*
 * =====================================================================================
 *
 *       Filename:  asynchronization.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  04/15/2013 02:39:21 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kevin Hu (), kevin.hu@b5m.com
 *        Company:  iZeneSoft.com
 *
 * =====================================================================================
 */

#ifndef _IZENELIB_THREAD_ASYNCHRONIZATION_H_
#define _IZENELIB_THREAD_ASYNCHRONIZATION_H_

#include <types.h>
#include <vector>
#include <assert.h>
#include <iostream>
#include <list>
#include <pthread.h>

#include "util/kthread/mutex.h"
#include "util/kthread/condition.h"

namespace izenelib
{

class Asynchronization
{
    uint32_t pool_size_;
    std::list<pthread_t> pids_;
    Condition cond_;
    Mutex mtx_;

    void wait_()
    {
        for ( std::list<pthread_t>::const_iterator it=pids_.begin(); it!=pids_.end(); ++it)
        {
            pthread_join(*it, NULL);
            break;
        }
        pids_.pop_front();
    }

public:
    Asynchronization(uint32_t pool_size = 10)
        :pool_size_(pool_size)
    {
        cond_.set_mutex(&mtx_);
    }

    template<
    typename CLASS,
             typename FOO,
             typename PARAM
             >
    void new_batch_of_thread(CLASS* p, FOO fo, const PARAM& param)
    {
        while (pids_.size() < pool_size_)
            new_thread(p, fo, param);
    }

    template<
    typename CLASS,
             typename FOO,
             typename PARAM
             >
    static bool detach_thread(CLASS* p, FOO fo, const PARAM& param)
    {
        pthread_t pid;
        std::pair<CLASS*, std::pair<FOO,PARAM> >* data = new std::pair<CLASS*, std::pair<FOO,PARAM> >(p, std::pair<FOO,PARAM>(fo, param));
        if(pthread_create(&pid, NULL, Asynchronization::Fun<CLASS,FOO,PARAM>, (void*)data)!=0)
            return false;
        pthread_detach(pid);
        return true;
    }

    template<
             typename FOO,
             typename PARAM
             >
    void new_static_thread(FOO fo, const PARAM& param)
    {
        if (pids_.size() == pool_size_)
            wait_();
        pthread_t pid;
        std::pair<FOO,PARAM>* data = new std::pair<FOO,PARAM>(fo,param);
        pthread_create(&pid, NULL, Asynchronization::StaticFun<FOO, PARAM>, (void*)data);
        pids_.push_back(pid);
    }

    template<
    typename CLASS,
             typename FOO,
             typename PARAM
             >
    void new_thread(CLASS* p, FOO fo, const PARAM& param)
    {
        if (pids_.size() == pool_size_)
            wait_();
        pthread_t pid;
        std::pair<CLASS*, std::pair<FOO,PARAM> >* data = new std::pair<CLASS*, std::pair<FOO,PARAM> >(p, std::pair<FOO,PARAM>(fo, param));
        pthread_create(&pid, NULL, Asynchronization::Fun<CLASS,FOO,PARAM>, (void*)data);
        pids_.push_back(pid);
    }

    void join()
    {
        for ( std::list<pthread_t>::const_iterator it=pids_.begin(); it!=pids_.end(); ++it)pthread_join(*it, NULL);
        pids_.clear();
    }

    template<
    typename CLASS,
             typename FOO,
             typename PARAM
             >
    void new_thread_with_delete(CLASS* p, FOO fo, PARAM* param)
    {
        if (pids_.size() == pool_size_)
            wait_();
        pthread_t pid;
        std::pair<CLASS*, std::pair<FOO,PARAM*> >* data = new std::pair<CLASS*, std::pair<FOO,PARAM*> >(p, std::pair<FOO,PARAM*>(fo, param));
        pthread_create(&pid, NULL, Asynchronization::Fun_with_delete<CLASS,FOO,PARAM>, (void*)data);
        pids_.push_back(pid);
    }

    template<
             typename FOO,
             typename PARAM
             >
    static void *StaticFun(void *data)
    {
        std::pair<FOO,PARAM> param = *(std::pair<FOO,PARAM> *)data;
        (*param.first)(param.second);
        delete[] (std::pair<FOO,PARAM>*)data;
        return NULL;
    }

    template<
    typename CLASS,
             typename FOO,
             typename PARAM
             >
    static void *Fun(void *data)
    {
        std::pair<CLASS*, std::pair<FOO,PARAM> > param = *(std::pair<CLASS*, std::pair<FOO,PARAM> >*)data;
        CLASS* p = param.first;
        (p->*param.second.first)(param.second.second);
        delete[] (std::pair<CLASS*, std::pair<FOO,PARAM> >*)data;
        return NULL;
    }

    template<
    typename CLASS,
             typename FOO,
             typename PARAM
             >
    static void *Fun_with_delete(void *data)
    {
        std::pair<CLASS*, std::pair<FOO,PARAM*> > param = *(std::pair<CLASS*, std::pair<FOO,PARAM*> >*)data;
        CLASS* p = param.first;
        (p->*param.second.first)(param.second.second);
        delete[] (PARAM*)param.second.second;
        delete[] (std::pair<CLASS*, std::pair<FOO,PARAM> >*)data;
        return NULL;
    }
};

}
#endif

