/* 
 * File:   Sf1Driver.cpp
 * Author: paolo
 * 
 * Created on January 10, 2012, 10:07 AM
 */

#include "net/sf1r/Sf1Driver.hpp"
#include "ConnectionPool.hpp"
#include "JsonWriter.hpp"
#include "RawClient.hpp"
#include "Utils.hpp"
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>


NS_IZENELIB_SF1R_BEGIN


using ba::ip::tcp;
using boost::system::system_error;
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


Sf1Driver::Sf1Driver(const string& host, const uint32_t& port, 
        const Sf1Config& parameters, const Format& fmt) throw(ServerError) 
        : Sf1DriverBase(parameters, fmt), resolver(service),
          query(host, boost::lexical_cast<string>(port)) {
    try {
        iterator = resolver.resolve(query);
        initPool(parameters);
        
        LOG(INFO) << "Driver ready.";
    } catch (system_error& e) {
        string message = e.what();
        LOG(ERROR) << message;
        throw ServerError(message);
    }
}


Sf1Driver::~Sf1Driver() {
    LOG(INFO) << "Driver closed.";
}


void
Sf1Driver::initPool(const Sf1Config& params) {
    pool.reset(new ConnectionPool(service, iterator, 
                params.initialSize, params.resize, params.maxSize));
}


string
Sf1Driver::call(const string& uri, const string& tokens, string& request) 
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
    LOG(INFO) << "Send " << getFormatString() << " request: " << request;
    
    // increment sequence
    if (++sequence == MAX_SEQUENCE) {
        sequence = 1; // sequence == 0 means server error
    }
    
    RawClient& client = pool->acquire();
    
    client.sendRequest(sequence, request);
    Response response = client.getResponse();
    uint32_t responseSequence = response.get<RESPONSE_SEQUENCE>();

    if (responseSequence == 0) {
        LOG(ERROR) << "Zero sequence";
        pool->release();
        throw ServerError("Zero Sequence");
    }

    if (sequence != responseSequence) {
        LOG(ERROR) << "Unmatched sequence number: "
                    << "in = [" << sequence << "] out = [" << responseSequence << "]";
        pool->release();
        throw ServerError("Unmatched sequence number");
    }

    string responseBody = response.get<RESPONSE_BODY>();

    if (not writer->checkData(responseBody)) { // This should never happen
        LOG(ERROR) << "Malformed response: [" << responseBody << "]";
        pool->release();
        throw ServerError("Malformed response");
    }

    pool->release();
    return responseBody;
}


inline size_t 
Sf1Driver::getPoolSize() const {
    return pool->getSize();
}


NS_IZENELIB_SF1R_END
