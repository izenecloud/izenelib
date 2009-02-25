// out_of_the_box_no_levels.hpp

// Boost Logging library
//
// Author: John Torjo, www.torjo.com
//
// Copyright (C) 2007 John Torjo (see www.torjo.com for email)
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org for updates, documentation, and revision history.
// See http://www.torjo.com/log2/ for more details


#ifndef JT28092007_out_of_the_box_no_levels_HPP_DEFINED
#define JT28092007_out_of_the_box_no_levels_HPP_DEFINED

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/logging/format/named_write.hpp>
typedef boost::logging::named_logger<>::type logger_type;
typedef boost::logging::filter::no_ts filter_type;

namespace boost { namespace logging {
    struct out_of_the_box_logger : logger_type {
        out_of_the_box_logger() {
            writer().write( BOOST_LOG_STR("%time%($yyyy-$MM-$dd $hh:$mm.$ss) [%idx%] |\n"), BOOST_LOG_STR("cout file(log.txt)"));
            mark_as_initialized();
        }
    };

    struct do_log {
        static out_of_the_box_logger* l_() { static out_of_the_box_logger i; return &i; }
        static filter_type* l_filter_() { static filter_type f; return &f; }
    };

}}

#define L_ BOOST_LOG_USE_LOG_IF_FILTER( ::boost::logging::do_log::l_(), ::boost::logging::do_log::l_filter_()->is_enabled() ) 

#endif

