/*
 * =====================================================================================
 *
 *       Filename:  queue.hpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  04/19/2011 01:54:45 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kevin Hu (), kevin.hu@b5m.com
 *        Company:  iZeneSoft.com
 *
 * =====================================================================================
 */

#ifndef _IZENELIB_SEDA_QUEUE_HPP_
#define _IZENELIB_SEDA_QUEUE_HPP_

#include "types.h"
#include <vector>
#include <assert.h>
#include <cerrno>
#include <vector>
#include <iostream>
#include <cstdio>
#include "util/kthread/mutex.h"
#include "util/kthread/condition.h"

namespace izenelib
{

template<
class EVENT_TYPE
>
struct QNode
{
    uint32_t _ref;
    const EVENT_TYPE* _ptr;
    uint64_t _eid;

    QNode(uint32_t ref=0, const EVENT_TYPE* ptr=NULL, uint64_t eid=0)
        :_ref(ref),_ptr(ptr),_eid(eid)
    {}
};

template<
class EVENT_TYPE
>
class EventQueue
{
    typedef struct QNode<EVENT_TYPE> node_t;
    std::vector<node_t > _q;
    volatile uint32_t _head_idx;
    volatile uint32_t _tail_idx;
    uint32_t _ref;
    Mutex _mut;
    Mutex _push_mtx;
    Mutex _pop_mtx;
    Condition _cond;
    bool _stop;
    Mutex _mut4stop;

public:
    typedef EVENT_TYPE event_t;

    EventQueue(uint32_t max_len=1000)
        :_stop(false)
    {
        _q.reserve(max_len);
        for ( uint32_t i=0; i<max_len; ++i)
            _q.push_back(QNode<EVENT_TYPE>());
        _head_idx = _tail_idx = 0;
        _cond.set_mutex(&_mut);
        _ref = 0;
    }

    ~EventQueue()
    {
    }

    EventQueue* add_ref()
    {
        _ref++;
        return this;
    }

    void push(const EVENT_TYPE& event, uint64_t eid)
    {
        ScopedLock scp(&_push_mtx);
        ScopedLock scp_mut(&_mut);

        uint32_t newidx = (_tail_idx+1)%_q.size();
        while(newidx == _head_idx) _cond.wait();

        node_t node(_ref, new EVENT_TYPE(event), eid);
        _q[_tail_idx] = node;
        _tail_idx++;
        _tail_idx %= _q.size();
        _cond.notify();
    }

    bool full()
    {
        ScopedLock scp_mut(&_mut);
        return ((_tail_idx+1)%_q.size() == _head_idx);
    }

    bool empty()
    {
        ScopedLock scp_mut(&_mut);
        return _head_idx == _tail_idx;
    }

    void reset_stop(bool f=false)
    {
        ScopedLock scp_mut(&_mut4stop);
        _stop = f;
    }

    bool stoped()
    {
        ScopedLock scp_mut(&_mut4stop);
        return _stop;
    }

    EVENT_TYPE top()
    {
        return *(EVENT_TYPE*)_q[_head_idx]._ptr;
    }

    bool pop(EVENT_TYPE& event, uint64_t& eid)
    {
        ScopedLock scp(&_pop_mtx);
        ScopedLock scp_mut(&_mut);
        while(_head_idx == _tail_idx && !stoped())
            _cond.wait(1);
        if (stoped())return false;

        event = *(EVENT_TYPE*)_q[_head_idx]._ptr;
        eid  = _q[_head_idx]._eid;
        if (_q[_head_idx]._ref > 1)
        {
            _q[_head_idx]._ref--;
            _cond.notify();
            return true;
        }
        delete (EVENT_TYPE*)_q[_head_idx]._ptr;
        _head_idx++;
        _head_idx %= _q.size();

        _cond.notify();
        return true;
    }
}
;

}

#endif
