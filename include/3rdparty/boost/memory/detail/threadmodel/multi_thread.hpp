//
//  boost/detail/threadmodel/multi_thread.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/detail/todo.htm for documentation.
//
#ifndef BOOST_DETAIL_THREADMODEL_MULTI_THREAD_HPP
#define BOOST_DETAIL_THREADMODEL_MULTI_THREAD_HPP

NS_BOOST_DETAIL_BEGIN

// -------------------------------------------------------------------------
// class refcount_mt

class refcount_mt
{
public:
    typedef long value_type;

private:
    value_type m_nRef;

public:
    refcount_mt(value_type nRef)
        : m_nRef(nRef)
    {
    }

    value_type BOOST_DETAIL_CALL acquire()
    {
        return InterlockedIncrement(&m_nRef);
    }

    value_type BOOST_DETAIL_CALL release()
    {
        return InterlockedDecrement(&m_nRef);
    }

    operator value_type()
    {
        return m_nRef;
    }
};


NS_BOOST_DETAIL_END

#endif /* BOOST_DETAIL_THREADMODEL_MULTI_THREAD_HPP */
