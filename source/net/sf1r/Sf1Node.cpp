/* 
 * File:   Sf1Node.cpp
 * Author: Paolo D'Apice
 * 
 * Created on February 20, 2012, 10:41 AM
 */

#include "net/sf1r/Sf1Node.hpp"
#include "Utils.hpp"

NS_IZENELIB_SF1R_BEGIN

using std::string;
using std::ostream;

Sf1Node::Sf1Node(const string& h, const uint32_t& bp, 
        const uint32_t& dp, const uint32_t& mp, const string& coll)
        : host(h), dataPort(dp), baPort(bp), masterPort(mp) {
    split(coll, ',', collections);
}


Sf1Node::~Sf1Node() {}


NS_IZENELIB_SF1R_END
