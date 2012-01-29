/* 
 * File:   JsonWriter.hpp
 * Author: Paolo D'Apice
 *
 * Created on January 10, 2012, 11:18 AM
 */

#ifndef JSONWRITER_HPP
#define	JSONWRITER_HPP

#include "Writer.hpp"


namespace izenelib {
namespace net {
namespace sf1r {


/**
 * Class implementing the Writer interface for the JSON format.
 */
class JsonWriter : public Writer {
public:
    JsonWriter();
    virtual ~JsonWriter();

    void setHeader(const std::string& controller, 
                   const std::string& action,
                   const std::string& tokens,
                   std::string& request) const;

    bool checkData(const std::string& data) const;
    
};


}}} /* namespace izenelib::net::sf1r */


#endif	/* JSONWRITER_HPP */
