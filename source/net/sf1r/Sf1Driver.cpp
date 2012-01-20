/* 
 * File:   Sf1Driver.cpp
 * Author: paolo
 * 
 * Created on January 10, 2012, 10:07 AM
 */

#include "JsonWriter.hpp"
#include "net/sf1r/Sf1Driver.hpp"
#include "RawClient.hpp"
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>


namespace izenelib {
namespace net {
namespace sf1r {


using std::string;


/**
 * Max sequence number.
 * Valued 4294967294 as defined in the Ruby client:
 * \code 
 * MAX_SEQUENCE = (1 << 31) - 2
 * \endcode
 * It is also the upper limit of the number of requests in a batch request.
 * @see \ref limits
 */
const uint32_t MAX_SEQUENCE = std::numeric_limits<uint32_t>::max() - 1;


Sf1Driver::Sf1Driver(const string& host, const uint32_t& port, 
        const Format& format) throw(ServerError) : sequence(1) {
    try {
        client = new RawClient(host, boost::lexical_cast<string>(port));
        client->connect();
    
        setFormat(format);
    
        LOG(INFO) << "Driver ready";
    } catch (std::exception& e) {
        string message = e.what();
        LOG(ERROR) << message;
        throw ServerError(message);
    }
}


Sf1Driver::~Sf1Driver() {
    delete writer;
    
    client->close();
    delete client;
    
    LOG(INFO) << "Driver closed";
}


void
Sf1Driver::setFormat(const Format& format) {
    switch (format) {
    case JSON:
    default:
        writer = new JsonWriter;
        LOG(INFO) << "Using JSON data format";
    }
}


/// Split a string on delimiter character into a vector.
std::vector<string>&
split(const string &s, char delim, std::vector<string> &elems) {
    std::stringstream ss(s);
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


string
Sf1Driver::call(const string& uri, const string& tokens, string& request) 
throw(ClientError, ServerError) {
    // parse uri for controller and action
    std::vector<string> elems;
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
    
    writer->setHeader(controller, action, tokens, request);
    LOG(INFO) << "sending request: " << request;
    
    // increment sequence
    if (++sequence == MAX_SEQUENCE) {
        sequence = 1; // sequence == 0 means server error
    }
    
    try {
        client->sendRequest(sequence, request);
        std::pair<uint32_t, string> response = client->getResponse();

        if (sequence != response.first) {
            LOG(ERROR) << "Unmatched sequence number: "
                       << "in = [" << sequence << "] out = [" << response.first << "]";
            throw ServerError("Unmatched sequence number");
        }
        
        if (not writer->checkData(response.second)) { // This should never happen
            LOG(ERROR) << "Malformed response: [" << response.second << "]";
            throw ServerError("Malformed response");
        }

        return response.second;
    } catch (ClientError& e) {
        // do not intercept ClientErrors
        throw e;
    } catch (ServerError& e) {
        // do not intercept ServerErrors
        throw e;
    } catch (std::exception& e) {
        string message = e.what();
        LOG(ERROR) << message;
        throw e;
    }
}


}}} /* namespace izenelib::net::sf1r */
