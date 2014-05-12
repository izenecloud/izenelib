/***********************************************************************
 * Software License Agreement (BSD License)
 *
 * Copyright 2008-2009  Marius Muja (mariusm@cs.ubc.ca). All rights reserved.
 * Copyright 2008-2009  David G. Lowe (lowe@cs.ubc.ca). All rights reserved.
 *
 * THE BSD LICENSE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************/

#ifndef _IZENELIB_AM_INTERVAL_HEAP_HPP
#define _IZENELIB_AM_INTERVAL_HEAP_HPP

#include <assert.h>
#include <algorithm>
#include <vector>

NS_IZENELIB_AM_BEGIN

template <class T, class Compare = std::less<T> >
class interval_heap
{
public:
    typedef T value_type;
    typedef std::vector<std::pair<T, T> > container_type;
    typedef typename container_type::size_type size_type;

    /**
     * Constructor.
     *
     * Params:
     *     size = heap size
     */
    explicit interval_heap(size_type capacity, const Compare& compare = Compare())
        : compare_(compare)
        , container_((capacity + 1) / 2 + 1) // 1-based indexing
        , capacity_(capacity)
        , size_(0)
    {
    }

    /**
     * @return Heap size
     */
    size_type size() const
    {
        return size_;
    }

    /**
     * Tests if the heap is empty
     * @return true is heap empty, false otherwise
     */
    bool empty() const
    {
        return size_ == 0;
    }

    size_type capacity() const
    {
        return capacity_;
    }

    /**
     * Clears the heap.
     */
    void clear()
    {
        size_ = 0;
    }

    void reserve(size_type capacity)
    {
        if (capacity > capacity_)
        {
            container_.resize((capacity + 1) / 2 + 1);
            capacity_ = capacity;
        }
    }

    container_type& get_container()
    {
        return container_;
    }

    const container_type& get_container() const
    {
        return container_;
    }

    void insert(const value_type& value)
    {
        if (size_ == capacity_) return;

        // insert into the root
        if (size_ < 2)
        {
            if (size_ == 0)
            {
                container_[1].first = container_[1].second = value;
            }
            else
            {
                if (compare_(value, container_[1].first))
                {
                    container_[1].first = value;
                }
                else
                {
                    container_[1].second = value;
                }
            }
            ++size_;
            return;
        }

        size_type last_pos = (size_ + 1) / 2;
        bool min_heap;

        if (size_ % 2) // odd number of elements
        {
            min_heap = compare_(value, container_[last_pos].first);
        }
        else
        {
            min_heap = compare_(value, container_[++last_pos / 2].first);
        }

        if (min_heap)
        {
            size_type pos = last_pos;
            size_type par = pos / 2;
            while (pos > 1 && compare_(value, container_[par].first))
            {
                container_[pos].first = container_[par].first;
                pos = par;
                par /= 2;
            }
            container_[pos].first = value;

            if (++size_ % 2) // duplicate element in last position if size is odd
            {
                container_[last_pos].second = container_[last_pos].first;
            }
        }
        else
        {
            size_type pos = last_pos;
            size_type par = pos / 2;
            while (pos > 1 && compare_(container_[par].second, value))
            {
                container_[pos].second = container_[par].second;
                pos = par;
                par /= 2;
            }
            container_[pos].second = value;

            if (++size_ % 2) // duplicate element in last position if size is odd
            {
                container_[last_pos].first = container_[last_pos].second;
            }
        }
    }

    void replace_min(const value_type& value)
    {
        assert(size_ > 0);
        if (size_ == 1)
        {
            container_[1].first = container_[1].second = value;
            return;
        }

        size_type last_pos = (size_ + 1) / 2;
        value_type elem = value;

        if (compare_(container_[1].second, elem))
            std::swap(elem, container_[1].second);

        size_type current = 1; // root node
        size_type child = 2;

        while (child <= last_pos)
        {
            child += child < last_pos && compare_(container_[child + 1].first, container_[child].first);

            if (!compare_(container_[child].first, elem)) break;

            container_[current].first = container_[child].first;
            if (compare_(container_[child].second, elem))
                std::swap(elem, container_[child].second);

            current = child;
            child *= 2;
        }

        container_[current].first = (current <= size_ / 2) ? elem : container_[current].second;
    }

    void replace_max(const value_type& value)
    {
        assert(size_ > 0);
        if (size_ == 1)
        {
            container_[1].first = container_[1].second = value;
            return;
        }

        size_type last_pos = (size_ + 1) / 2;
        value_type elem = value;

        if (compare_(elem, container_[1].first))
            std::swap(elem, container_[1].first);

        size_type current = 1; // root node
        size_type child = 2;

        while (child <= last_pos)
        {
            child += child < last_pos && compare_(container_[child].second, container_[child + 1].second);

            if (!compare_(elem, container_[child].second)) break;

            container_[current].second = container_[child].second;
            if (compare_(elem, container_[child].first))
                std::swap(elem, container_[child].first);

            current = child;
            child *= 2;
        }

        container_[current].second = (current <= size_ / 2) ? elem : container_[current].first;
    }

    void pop_min()
    {
        assert(size_ > 0);
        size_type last_pos = (size_ + 1) / 2;
        value_type elem = container_[last_pos].first;

        if (size_ % 2) // odd number of elements
        {
            --last_pos;
        }
        else
        {
            container_[last_pos].first = container_[last_pos].second;
        }
        if (--size_ < 2) return;

        size_type current = 1; // root node
        size_type child = 2;

        while (child <= last_pos)
        {
            child += child < last_pos && compare_(container_[child + 1].first, container_[child].first);

            if (!compare_(container_[child].first, elem)) break;

            container_[current].first = container_[child].first;
            if (compare_(container_[child].second, elem))
                std::swap(elem, container_[child].second);

            current = child;
            child *= 2;
        }

        container_[current].first = elem;
    }

    void pop_max()
    {
        assert(size_ > 0);
        size_type last_pos = (size_ + 1) / 2;
        value_type elem = container_[last_pos].second;

        if (size_ % 2) // odd number of elements
        {
            --last_pos;
        }
        else
        {
            container_[last_pos].second = container_[last_pos].first;
        }
        if (--size_ < 2) return;

        size_type current = 1; // root node
        size_type child = 2;

        while (child <= last_pos)
        {
            child += child < last_pos && compare_(container_[child].second, container_[child + 1].second);

            if (!compare_(elem, container_[child].second)) break;

            container_[current].second = container_[child].second;
            if (compare_(elem, container_[child].first))
                std::swap(elem, container_[child].first);

            current = child;
            child *= 2;
        }

        container_[current].second = elem;
    }

    value_type& get_min()
    {
        assert(size_ > 0);
        return container_[1].first;
    }

    const value_type& get_min() const
    {
        assert(size_ > 0);
        return container_[1].first;
    }

    value_type& get_max()
    {
        assert(size_ > 0);
        return container_[1].second;
    }

    const value_type& get_max() const
    {
        assert(size_ > 0);
        return container_[1].second;
    }

private:
    /**
     * Storage array for the heap.
     * Type T must be comparable.
     */
    Compare compare_;
    container_type container_;
    size_type capacity_;
    size_type size_;
};


NS_IZENELIB_AM_END

#endif
