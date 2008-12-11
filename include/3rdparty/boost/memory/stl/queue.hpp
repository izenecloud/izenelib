//
//  stl/queue.hpp (*)
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/memory/index.htm for documentation.
//
#ifndef BOOST_MEMORY_STL_QUEUE_HPP
#define BOOST_MEMORY_STL_QUEUE_HPP

#if !defined(_VECTOR_) && !defined(_VECTOR)
#include <vector> // std::vector
#endif

#if !defined(_FUNCTIONAL_) && !defined(_FUNCTIONAL)
#include <functional> // std::less
#endif

#if !defined(_ALGORITHM_) && !defined(_ALGORITHM)
#include <algorithm> // std::make_heap, etc
#endif

#ifndef NS_BOOST_MEMORY_BEGIN
#define NS_BOOST_MEMORY_BEGIN	namespace boost { namespace memory {
#define NS_BOOST_MEMORY_END		} }
#define NS_BOOST_MEMORY			boost::memory
#endif

NS_BOOST_MEMORY_BEGIN

// -------------------------------------------------------------------------

template <class Type, 
          class Sequence = std::vector<Type>,
          class Pred = std::less<Type> >
class ext_priority_queue
{
public:
  typedef typename Sequence::value_type      value_type;
  typedef typename Sequence::size_type       size_type;
  typedef          Sequence                  container_type;

  typedef typename Sequence::reference       reference;
  typedef typename Sequence::const_reference const_reference;

protected:
  Sequence m_coll;
  Pred m_pred;

public:
  ext_priority_queue() {}
  explicit ext_priority_queue(const Pred& x) :  m_coll(), m_pred(x) {}
  ext_priority_queue(const Pred& x, const Sequence& s) 
    : m_coll(s), m_pred(x) 
    { std::make_heap(m_coll.begin(), m_coll.end(), m_pred); }

  template <class InputIterator>
  ext_priority_queue(InputIterator first, InputIterator last) 
    : m_coll(first, last) { std::make_heap(m_coll.begin(), m_coll.end(), m_pred); }

  template <class InputIterator>
  ext_priority_queue(InputIterator first, 
                 InputIterator last, const Pred& x)
    : m_coll(first, last), m_pred(x) 
    { std::make_heap(m_coll.begin(), m_coll.end(), m_pred); }

  template <class InputIterator>
  ext_priority_queue(InputIterator first, InputIterator last,
                 const Pred& x, const Sequence& s)
  : m_coll(s), m_pred(x)
  {
    m_coll.insert(m_coll.end(), first, last);
    std::make_heap(m_coll.begin(), m_coll.end(), m_pred);
  }

  bool empty() const { return m_coll.empty(); }
  size_type size() const { return m_coll.size(); }
  const_reference top() const { return m_coll.front(); }
  void push(const value_type& x) {
    m_coll.push_back(x); 
	std::push_heap(m_coll.begin(), m_coll.end(), m_pred);
  }
  void pop() {
    std::pop_heap(m_coll.begin(), m_coll.end(), m_pred);
    m_coll.pop_back();
  }
  void clear() {
	m_coll.clear();
  }
  void swap(ext_priority_queue& o) {
	m_coll.swap(o.m_coll);
	std::swap(m_pred, o.m_pred);
  }
};

// -------------------------------------------------------------------------
// $Log: $

NS_BOOST_MEMORY_END

#endif /* BOOST_MEMORY_STL_QUEUE_HPP */
