/* 
 * File:   Errors.hpp
 * Author: Paolo D'Apice
 *
 * Created on February 6, 2012, 3:55 PM
 */

#ifndef ERRORS_HPP
#define	ERRORS_HPP

#include "config.h"
#include <stdexcept>
#include <string>


NS_IZENELIB_SF1R_BEGIN

/**
 * Exception thrown on network communication errors.
 */
class NetworkError : public std::runtime_error {
public:
    NetworkError(const std::string& m = "") : std::runtime_error(m) {}
};


/**
 * Exception thrown by the Sf1Driver on server errors.
 * Server errors occur when:
 * - SF1 not reachable
 * - SF1 errors (e.g. unmatched sequence number, malformed response)
 */
class ServerError : public std::runtime_error {
public:
    ServerError(const std::string& m = "") : std::runtime_error(m) {}
};


/** 
 * Exception thrown by the Sf1Driver on client errors.
 * Client errors occur when:
 * - missing or wrong URI in request
 * - malformed request
 */
class ClientError : public std::runtime_error {
public:
    ClientError(const std::string& m = "") : std::runtime_error(m) {}
};


/**
 * Exception thrown on connection pool errors.
 */
class ConnectionPoolError : public std::runtime_error {
public:
    ConnectionPoolError(const std::string& m = "") : std::runtime_error(m) {}
};


/**
 * Exception thrown on routing errors.
 */
class RoutingError : public std::runtime_error {
public:
    RoutingError(const std::string& m = "") : std::runtime_error(m) {}
};


NS_IZENELIB_SF1R_END


#endif	/* ERRORS_HPP */
