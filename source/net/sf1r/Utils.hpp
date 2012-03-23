/* 
 * File:   Utils.hpp
 * Author: Paolo D'Apice
 *
 * Created on February 20, 2012, 10:44 AM
 */

#ifndef UTILS_HPP
#define	UTILS_HPP

#include "net/sf1r/config.h"
#include <sstream>
#include <vector>

NS_IZENELIB_SF1R_BEGIN

namespace {

/**
 * Split a string on delimiter character into a vector.
 * @param input The input string.
 * @param delim The delimiter character.
 * @param elems The output vector.
 */
inline std::vector<std::string>&
split(const std::string& input, const char& delim, 
        std::vector<std::string>& elems) {
    std::stringstream ss(input);
    std::string item;
    while(getline(ss, item, delim)) {
        if (item.empty()) {
            // skip empty elements
            continue;
        }
        elems.push_back(item);
    }
    return elems;
}


}

NS_IZENELIB_SF1R_END

#endif	/* UTILS_HPP */

