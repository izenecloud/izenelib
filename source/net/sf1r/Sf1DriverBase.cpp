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

/**
 * Max sequence number.
 * Valued 4294967294 as defined in the Ruby client:
 * \code 
 * MAX_SEQUENCE = (1 << 31) - 2
 * \endcode
 * @see \ref limits
 */
const uint32_t MAX_SEQUENCE = std::numeric_limits<uint32_t>::max() - 1;


Sf1DriverBase::Sf1DriverBase(const Sf1Config& parameters, const Format& fmt) 
        : resolver(service), sequence(1), config(parameters), format(fmt),
        factory(new PoolFactory(service, resolver, config)) {
    setFormat();
    DLOG(INFO) << "initialized";
}


Sf1DriverBase::~Sf1DriverBase() {
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


string
Sf1DriverBase::call(const string& uri, const string& tokens, string& request) 
throw(ClientError, ServerError, ConnectionPoolError) {
    // parse uri for controller and action
    vector<string> elems;
    split(uri, '/', elems);
    
    // controller is mandatory
    if (elems.size() < 1) {
        LOG(ERROR) << "Require controller name: [" << uri << "]";
        throw ClientError("Require controller name");
    }
    
    string controller = elems.at(0);
    DLOG(INFO) << "controller: " << controller;
    
    // action is optional
    string action;
    if (elems.size() > 1) {
        action = elems.at(1);
    } 
    DLOG(INFO) << "action    : " << action;
    
    // check request
    if (not writer->checkData(request)) {
        LOG(ERROR) << "Malformed request: [" << request << "]";
        throw ClientError("Malformed request");
    }
    
    // process header: set action, controller, tokens and get collections
    string collection;
    writer->setHeader(controller, action, tokens, request, collection);
    DLOG(INFO) << "collection: " << collection;
    
    LOG(INFO) << "Send " << getFormatString() << " request: " << request;
    
    // increment sequence
    if (++sequence == MAX_SEQUENCE) {
        sequence = 1; // sequence == 0 means server error
    }
    
    // get a connection
    RawClient& client = acquire(collection);
    
    // process request
    client.sendRequest(sequence, request);
    Response response = client.getResponse();
    uint32_t responseSequence = response.get<RESPONSE_SEQUENCE>();

    if (responseSequence == 0) {
        LOG(ERROR) << "Zero sequence";
        release(client);
        throw ServerError("Zero Sequence");
    }

    if (sequence != responseSequence) {
        LOG(ERROR) << "Unmatched sequence number: "
                    << "in = [" << sequence << "] out = [" << responseSequence << "]";
        release(client);
        throw ServerError("Unmatched sequence number");
    }

    string responseBody = response.get<RESPONSE_BODY>();

    if (not writer->checkData(responseBody)) { // This should never happen
        LOG(ERROR) << "Malformed response: [" << responseBody << "]";
        release(client);
        throw ServerError("Malformed response");
    }

    release(client);
    return responseBody;
}


NS_IZENELIB_SF1R_END
