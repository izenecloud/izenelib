/* 
 * File:   Sf1DriverBase.cpp
 * Author: Paolo D'Apice
 *
 * Created on March 1, 2012, 10:23 AM
 */

#include "net/sf1r/Sf1DriverBase.hpp"
#include "JsonWriter.hpp"
#include "RawClient.hpp"
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


Sf1DriverBase::Sf1DriverBase(const Sf1Config& parameters, 
        const Format& fmt) : sequence(1), config(parameters), format(fmt) {
    setFormat();
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


NS_IZENELIB_SF1R_END
