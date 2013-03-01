//
//  boost/detail/threadmodel.hpp (*)
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/detail/todo.htm for documentation.
//
#ifndef BOOST_DETAIL_THREADMODEL_HPP
#define BOOST_DETAIL_THREADMODEL_HPP

#ifndef NS_BOOST_DETAIL_BEGIN
#define NS_BOOST_DETAIL_BEGIN	namespace boost { namespace detail {
#define NS_BOOST_DETAIL_END		} }
#define NS_BOOST_DETAIL			boost::detail
#endif

#ifndef BOOST_DETAIL_CALL
#define BOOST_DETAIL_CALL
#endif

// -------------------------------------------------------------------------

#include "threadmodel/pthread.hpp"
#include "threadmodel/atomic_builtins.hpp"
#include "threadmodel/single_thread.hpp"
#include "threadmodel/multi_thread.hpp"

NS_BOOST_DETAIL_BEGIN

// -------------------------------------------------------------------------
// class multi_thread

class multi_thread
{
public:
    typedef refcount_mt refcount;
};

// -------------------------------------------------------------------------
// class single_thread

class single_thread
{
public:
    typedef refcount_st refcount;
};

// -------------------------------------------------------------------------
// class default_threadmodel

#if defined(_MT)
typedef multi_thread default_threadmodel;
#else
typedef single_thread default_threadmodel;
#endif

// -------------------------------------------------------------------------
// $Log: $

NS_BOOST_DETAIL_END

#endif /* BOOST_DETAIL_THREADMODEL_HPP */
