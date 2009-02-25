// convert_destination.hpp

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


#ifndef JT28092007_convert_destination_HPP_DEFINED
#define JT28092007_convert_destination_HPP_DEFINED

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/logging/detail/fwd.hpp>

namespace boost { namespace logging { 
    
namespace tag {
    template<
            class string_ ,
            class param1 ,
            class param2 ,
            class param3 ,
            class param4 ,
            class param5 ,
            class param6 ,
            class param7 ,
            class param8 ,
            class param9 ,
            class param10> struct holder ;
}
    
namespace destination {

template<class t> struct into {};



/** 
@brief Allows writing messages to destinations

It has 2 function overloads:
- write(message, output) - writes the given message, to the given output
- do_convert(message, into<other_type>() );

FIXME
*/
namespace convert {
    template<class obj, class char_type, class char_traits> void write(const obj & m, std::basic_ostream<char_type, char_traits> & out) {
        out << m;
    }

    template<class string_, class p1, class p2, class p3, class p4, class p5, class p6, class p7, class p8, class p9, class p10, class char_type, class char_traits> void write(const ::boost::logging::tag::holder<string_,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10> & src, std::basic_ostream<char_type, char_traits> & out ) {
        typedef typename ::boost::logging::tag::holder<string_,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10>::string_type string_type;
        out << (const string_type&)src;
    }

    template<class char_traits, class char_type> void write(const char_type* m, std::basic_ostream<char_type, char_traits> & out) {
        out << m;
    }

    inline const char_type * do_convert(const char_type * c, const into<const char_type*> &) { return c; }
    inline const char_type * do_convert(const std::basic_string<char_type> & s, const into<const char_type* > &) { return s.c_str(); }

    inline const std::basic_string<char_type> & do_convert(const std::basic_string<char_type> & s, const into< std::basic_string<char_type> > &) {
        return s;
    }
}

struct do_convert_destination {
    template<class msg, class dest> static void write(const msg & m, dest & d) {
        convert::write(m, d);
    }

    template<class msg, class dest> static dest do_convert(const msg & m, const into<dest> &) {
        return convert::do_convert(m, into<dest>() );
    }

};

}}}

#endif

