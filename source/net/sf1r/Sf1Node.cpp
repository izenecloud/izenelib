/* 
 * File:   Sf1Node.cpp
 * Author: Paolo D'Apice
 * 
 * Created on February 20, 2012, 10:41 AM
 */

#include "net/sf1r/Sf1Node.hpp"
#include "Utils.hpp"

NS_IZENELIB_SF1R_BEGIN

using izenelib::util::kv2string;
using std::string;
using std::ostream;


namespace {
    const string       HOST_KEY = "host";
    const string     BAPORT_KEY = "baport";
    const string   DATAPORT_KEY = "dataport";
    const string MASTERPORT_KEY = "masterport";
    const string COLLECTION_KEY = "collection";
    const char   DELIMITER_CHAR = ',';
}

kv2string Sf1Node::parser;


Sf1Node::Sf1Node(const string& nodePath, const string& data) 
        : path(nodePath) {
    update(data);
}


Sf1Node::~Sf1Node() {}


void 
Sf1Node::update(const std::string& data) {
    parser.loadKvString(data);
    
    host = parser.getStrValue(HOST_KEY);
    baPort = parser.getUInt32Value(BAPORT_KEY);
    dataPort = parser.getUInt32Value(DATAPORT_KEY);
    masterPort = parser.getUInt32Value(MASTERPORT_KEY);
    
    collections.clear();
    split(parser.getStrValue(COLLECTION_KEY), DELIMITER_CHAR, collections);
}

NS_IZENELIB_SF1R_END
