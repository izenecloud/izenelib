/* 
 * File:   Writer.hpp
 * Author: Paolo D'Apice
 *
 * Created on January 10, 2012, 11:10 AM
 */

#ifndef WRITER_HPP
#define	WRITER_HPP

#include <string>


namespace izenelib {
namespace net {
namespace sf1r {


/**
 * Interface for request body writers.
 */
class Writer {
public:
    
    /**
     * Adds SF1 header into the body of the request.
     * @param controller
     * @param action
     * @param tokens
     */
    virtual void setHeader(const std::string& controller, 
                           const std::string& action,
                           const std::string& tokens,
                           std::string& request) const = 0;
    
    /**
     * Checks if the data is valid.
     * @param data the data to be checked.
     * @return true if data is valid, false otherwise.
     */
    virtual bool checkData(const std::string& data) const = 0;
    
};


}}} /* namespace izenelib::net::sf1r */

#endif	/* WRITER_HPP */
