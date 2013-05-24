/*
 * =====================================================================================
 *
 *       Filename:  stage.hpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  12/28/2012 05:09:07 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kevin Hu (), kevin.hu@b5m.com
 *        Company:  iZeneSoft.com
 *
 * =====================================================================================
 */

#ifndef _IZENELIB_SEDA_STAGE_HPP_
#define _IZENELIB_SEDA_STAGE_HPP_

#include "types.h"
#include <vector>
#include <assert.h>
#include <cerrno>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ostream>
#include <sstream>

#include "util/kthread/thread.hpp"
#include "util/kthread/mutex.h"
#include "net/seda/queue.hpp"

namespace izenelib
{

class StageLog
{
    Mutex _mutx;

public:

    void logging(uint64_t eid, std::ostringstream& ss)
    {
        if (ss.tellp())
        {
            ScopedLock mu(&_mutx);
            std::cout<<"["<<eid<<"]\t"<<ss.str()<<std::endl;
        }
    }
};

template<
class PROC_TYPE,
      class INPUT_Q_TYPE,
      class OUTPUT_Q_TYPE,
      class LOG_TYPE = StageLog
      >
class Stage
{
    typedef Stage<PROC_TYPE, INPUT_Q_TYPE, OUTPUT_Q_TYPE> SelfT;
    PROC_TYPE* _proc;
    INPUT_Q_TYPE* _inq;
    OUTPUT_Q_TYPE* _outq;
    std::vector<Thread<SelfT>* > _thrds;
    volatile bool _stop;
    Mutex _mut4stop;

    LOG_TYPE _log;

public:
    Stage(PROC_TYPE* proc=NULL,INPUT_Q_TYPE* inq=NULL,
          OUTPUT_Q_TYPE* outq=NULL, uint32_t thr_num=2)
        : _proc(proc),_inq(inq),_outq(outq),_stop(false)
    {
        for ( uint32_t i=0; i<thr_num; ++i)
            _thrds.push_back(NULL);
    }

    void set_input_queue(INPUT_Q_TYPE* inq)
    {
        _inq = inq;
        _inq.add_ref();
    }

    void set_output_queue( OUTPUT_Q_TYPE* outq)
    {
        _outq = outq;
    }

    bool stoped()
    {
        ScopedLock mu(&_mut4stop);
        return _stop;
    }

    void reset_stop(bool f=false)
    {
        ScopedLock mu(&_mut4stop);
        _stop = f;
    }

    void *operator()(void *data)
    {
        if (_inq)_inq->reset_stop();
        while(1)
        {
            if (stoped() && (!_inq || _inq->empty()))
            {
                std::cout<<"Get Stop command!\n";
                break;
            }

            typename INPUT_Q_TYPE::event_t e;
            uint64_t eid = 0;
            if (_inq)
                if(!_inq->pop(e, eid))continue;

            typename OUTPUT_Q_TYPE::event_t oe;
            std::ostringstream ss;
            if(!_proc->doit(e, oe, ss))
            {
                _log.logging(eid, ss);
                continue;
            }

            if (eid == 0)
            {
                /* srand(time(NULL));*/ eid = rand();
                eid=eid<<32;
                eid += rand()+(uint64_t)(&eid);
            }
            _log.logging(eid, ss);
            if (_outq)_outq->push(oe, eid);
        }
        return NULL;
    }

    void start()
    {
        if (!_proc)
            throw std::runtime_error("Uninitialized stage.");
        reset_stop();
        for ( uint32_t i=0; i<_thrds.size(); ++i)
            _thrds[i] = new Thread<SelfT>(this);
    }

    void stop()
    {
        //if (_inq)_inq->reset_stop(true);
        reset_stop(true);
        for ( uint32_t i=0; i<_thrds.size(); ++i)if(_thrds[i])
            {
                _thrds[i]->join();
                delete _thrds[i];
                _thrds[i] = NULL;
            }
    }
}
;

}

#endif
