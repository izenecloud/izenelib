/*
 * File:   JsonWriter.cpp
 * Author: Paolo D'Apice
 *
 * Created on January 10, 2012, 11:18 AM
 */

#include "JsonWriter.hpp"
#include "Sf1Protocol.hpp"
#include <glog/logging.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>


NS_IZENELIB_SF1R_BEGIN


namespace rj = rapidjson;
using std::string;


JsonWriter::JsonWriter() {
    // nothing to do here
}


JsonWriter::~JsonWriter() {
    // nothing to do here
}


void
JsonWriter::setHeader(const string& ctr, const string& act, const string& tks,
                      string& request, string& coll) const {
    rj::Document document;

#if 0 // FIXME: error "Nothing should follow the root object or array."
    char buffer[request.length()];
    memcpy(buffer, request.c_str(), request.length());
    bool error = document.ParseInsitu<0>(buffer).HasParseError();
#else
    bool error = document.Parse<0>(request.c_str()).HasParseError();
#endif
    CHECK(not error) << ": JSON parse error: " << document.GetParseError();

    rj::Document::AllocatorType& allocator = document.GetAllocator();

    // get the header, if present
    rj::Value& header = document[SF1_HEADER];
    bool noHeader = false;
    if (header.IsNull()) {
        noHeader = true;
        header.SetObject();
    }

    // controller
    rj::Value controller(ctr.c_str(), ctr.length(), allocator);
    header.AddMember(SF1_HEADER_CONTROLLER, controller, allocator);

    // action
    if (not act.empty()) {
        rj::Value action(act.c_str(), act.length(), allocator);
        header.AddMember(SF1_HEADER_ACTION, action, allocator);
    }

    // tokens
    if (not tks.empty()) {
        rj::Value tokens(tks.c_str(), tks.length(), allocator);
        header.AddMember(SF1_HEADER_TOKENS, tokens, allocator);
    }

    // if not present, add the header
    if (noHeader) {
        document.AddMember(SF1_HEADER, header, allocator);
    }

    // write to string
    rj::StringBuffer stream;
    rj::Writer<rj::StringBuffer> writer(stream);
    document.Accept(writer);
    request.assign(stream.GetString(), stream.Size());

    // get the collection
    rj::Value& collection = document[SF1_HEADER_COLLECTION];
    if (not collection.IsNull())
        coll.assign(collection.GetString());
}


bool
JsonWriter::checkData(const string& data) const {
    rj::Document document;
    return not document.Parse<0>(data.c_str()).HasParseError();
};


NS_IZENELIB_SF1R_END
