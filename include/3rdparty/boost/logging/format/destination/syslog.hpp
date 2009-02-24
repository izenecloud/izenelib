// destination/syslog.hpp

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


#ifndef JT28092007_destination_syslog_HPP_DEFINED
#define JT28092007_destination_syslog_HPP_DEFINED

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/logging/detail/fwd.hpp>
#include <boost/logging/detail/manipulator.hpp>
#include <boost/logging/format/destination/convert_destination.hpp>
#include <boost/logging/format/destination/file.hpp>
#include <boost/logging/format/formatter/tags.hpp> // uses_tag 
#include <boost/logging/detail/level.hpp>
#include <syslog.h>

namespace boost { namespace logging { namespace destination {


/** 
    @brief Writes the string to syslog. Note: does not care about levels - always logs as LOG_INFO
*/
template<class convert_dest = do_convert_destination > struct syslog_no_levels_t : is_generic, boost::logging::op_equal::always_equal {

    template<class msg_type> void operator()(const msg_type & msg) const {
        syslog( LOG_INFO, msg.c_str() );
    }
};


/** @brief syslog_no_levels_t with default values. See syslog_no_levels_t

@copydoc syslog_no_levels_t
*/
typedef syslog_no_levels_t<> syslog_no_levels;






/** @brief Writes the string to syslog. It cares about levels

See @ref boost::logging::tag "how to use tags".
*/
struct syslog : is_generic, formatter::uses_tag< syslog, ::boost::logging::tag::level >, boost::logging::op_equal::always_equal  {
    template<class msg_type, class tag_type> void write_tag(msg_type & str, const tag_type & tag) const {
        ::syslog( level_to_syslog_level(tag.val) , as_string(str).c_str() );
    }

private:
    const hold_string_type& as_string(const hold_string_type & str) { return str; }

    int level_to_syslog_level( level::type level) {
        if ( level <= level::debug)
            return LOG_DEBUG;
        if ( level <= level::info)
            return LOG_INFO;
        if ( level <= level::warning)
            return LOG_WARNING;
        if ( level <= level::error)
            return LOG_ERR;
        if  ( level <= level::fatal)
            return LOG_CRIT;

        return LOG_EMERG;
    }
};







}}}

#endif

