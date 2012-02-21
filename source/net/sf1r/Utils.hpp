/* 
 * File:   Utils.hpp
 * Author: paolo
 *
 * Created on February 20, 2012, 10:44 AM
 */

#ifndef UTILS_HPP
#define	UTILS_HPP

#include "net/sf1r/config.h"
#include <sstream>
#include <vector>

NS_IZENELIB_SF1R_BEGIN

using std::string;
using std::stringstream;
using std::vector;

/**
 * Split a string on delimiter character into a vector.
 * @param input The input string.
 * @param delim The delimiter character.
 * @param elems The output vector.
 */
static vector<string>&
split(const string& input, const char& delim, vector<string>& elems) {
    stringstream ss(input);
    string item;
    while(getline(ss, item, delim)) {
        if (item.empty()) {
            // skip empty elements
            continue;
        }
        elems.push_back(item);
    }
    return elems;
}

NS_IZENELIB_SF1R_END

#endif	/* UTILS_HPP */

