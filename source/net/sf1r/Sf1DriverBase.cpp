/* 
 * File:   Sf1DriverBase.cpp
 * Author: Paolo D'Apice
 *
 * Created on March 1, 2012, 10:23 AM
 */

#include "net/sf1r/Sf1DriverBase.hpp"
#include "ConnectionPool.hpp"
#include "JsonWriter.hpp"
#include "PoolFactory.hpp"
#include "RawClient.hpp"
#include "Utils.hpp"
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>


NS_IZENELIB_SF1R_BEGIN


using std::string;
using std::vector;


const string VERSION = "20120702";

Sf1DriverBase::Sf1DriverBase(const Sf1Config& parameters, const Format& fmt) 
        : sequence(1), format(fmt),
        factory(new PoolFactory(service, parameters)) {
    LOG(INFO) << "Sf1Driver-" << VERSION;
    setFormat();
    DLOG(INFO) << "initialized";
}


Sf1DriverBase::~Sf1DriverBase() {
    DLOG(INFO) << "destroyed.";
}


void
Sf1DriverBase::setFormat() {
    switch (format) {
    case JSON:
    default:
        writer.reset(new JsonWriter);
        LOG(INFO) << "Using JSON data format.";
    }
}


void
Sf1DriverBase::parseUri(const string& uri, string& controller, string& action) const {
    vector<string> elems;
    split(uri, '/', elems);
    
    // controller is mandatory
    if (elems.size() < 1) {
        LOG(ERROR) << "Require controller name: [" << uri << "]";
        throw ClientError("Require controller name");
    }
    
    controller = elems.at(0);
    DLOG(INFO) << "controller: " << controller;
    
    // action is optional
    if (elems.size() > 1) {
        action = elems.at(1);
    } 
    DLOG(INFO) << "action    : " << action;
}


void
Sf1DriverBase::preprocessRequest(const string& controller, const string& action,
        const string& tokens, string& request, string& collection) const{
    // check request
    if (not writer->checkData(request)) {
        LOG(ERROR) << "Malformed request: [" << request << "]";
        throw ClientError("Malformed request");
    }
    
    // process header: set action, controller, tokens and get collections
    writer->setHeader(controller, action, tokens, request, collection);
    DLOG(INFO) << "collection: " << collection;
}


void
Sf1DriverBase::sendAndReceive(RawClient& client, const string& request, 
        string& responseBody) {
    client.sendRequest(sequence, request);
    
    // Probing must be done here, after using the connection.
    DLOG(INFO) << "is connected? " << client.isConnected();
    
    Response response = client.getResponse();
    uint32_t responseSequence = response.get<RESPONSE_SEQUENCE>();

    if (responseSequence == 0) {
        LOG(ERROR) << "Zero sequence";
        throw ServerError("Zero Sequence");
    }

    if (sequence != responseSequence) {
        LOG(ERROR) << "Unmatched sequence number: "
                    << "in = [" << sequence << "] out = [" << responseSequence << "]";
        throw ServerError("Unmatched sequence number");
    }

    responseBody.assign(response.get<RESPONSE_BODY>());

    if (not writer->checkData(responseBody)) { // This should never happen
        LOG(ERROR) << "Malformed response: [" << responseBody << "]";
        throw ServerError("Malformed response");
    }
}


NS_IZENELIB_SF1R_END
