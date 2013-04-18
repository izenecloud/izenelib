/* 
 * File:   Sf1Node.cpp
 * Author: Paolo D'Apice
 * 
 * Created on February 20, 2012, 10:41 AM
 */

#include "net/sf1r/distributed/Sf1Node.hpp"
#include "ZooKeeperNamespace.hpp"
#include "util/kv2string.h"
#include "../Utils.hpp"

NS_IZENELIB_SF1R_BEGIN

using izenelib::util::kv2string;
using std::string;
using std::ostream;


Sf1Node::Sf1Node(const string& path_, const string& data) 
        : path(path_) {
    kv2string parser;
    parser.loadKvString(data);
    
    host = parser.getStrValue(HOST_KEY);
    port = parser.getUInt32Value(BAPORT_KEY);
    service_state = parser.getStrValue(SERVICE_STATE_KEY);
    
    split(parser.getStrValue(SearchService + COLLECTION_KEY), DELIMITER_CHAR, collections);
}


Sf1Node::Sf1Node(const string& path_, const string& host_,
            const uint32_t port_, const string& colls) 
        : path(path_), host(host_), port(port_) {
    split(colls, DELIMITER_CHAR, collections);
}


void 
Sf1Node::update(const string& data) {
    kv2string parser;
    parser.loadKvString(data);
    
    host = parser.getStrValue(HOST_KEY);
    port = parser.getUInt32Value(BAPORT_KEY);
    service_state = parser.getStrValue(SERVICE_STATE_KEY);
    
    collections.clear();
    split(parser.getStrValue(SearchService + COLLECTION_KEY), DELIMITER_CHAR, collections);
}


NS_IZENELIB_SF1R_END
