/***************************************************************************
 *
 *  Copyright (C) 2002-2003 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#ifndef PAGER_H
#define PAGER_H

#include <list>
#include <vector>
#include <boost/noncopyable.h>
#include <boost/shared_ptr.hpp>

#include "blockmanager_fwd.h"

NS_IZENELIB_AM_BEGIN

//! \brief Pager with \b LRU replacement strategy
template <unsigned npages_>
class lru_pager : private boost::noncopyable
{
    typedef std::list<int_type> list_type;

    boost::shared_ptr<list_type> history;
    std::vector<list_type::iterator> history_entry;

public:
    enum { n_pages = npages_ };

    lru_pager() : history(new list_type), history_entry(npages_)
    {
        for (unsigned_type i = 0; i < npages_; i++)
            history_entry[i] = history->insert(history->end(), static_cast<int_type>(i));
    }
    ~lru_pager() { }
    int_type kick()
    {
        return history->back();
    }
    void hit(int_type ipage)
    {
        assert(ipage < (int_type)npages_);
        assert(ipage >= 0);
        history->splice(history->begin(), *history, history_entry[ipage]);
    }
    void swap(lru_pager & obj)
    {
        boost::shared_ptr<list_type> tmp = obj.history;
        obj.history = history;
        history = tmp;
        std::swap(history_entry, obj.history_entry);
    }
};


NS_IZENELIB_AM_END

#endif

