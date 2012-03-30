/* 
 * File:   JsonWriter.hpp
 * Author: Paolo D'Apice
 *
 * Created on January 10, 2012, 11:18 AM
 */

#ifndef JSONWRITER_HPP
#define	JSONWRITER_HPP

#include "net/sf1r/config.h"
#include "Writer.hpp"


NS_IZENELIB_SF1R_BEGIN


/**
 * Class implementing the Writer interface for the JSON format.
 */
class JsonWriter : public Writer {
public:
    JsonWriter();
    ~JsonWriter();

    void setHeader(const std::string& controller, 
                   const std::string& action,
                   const std::string& tokens,
                   std::string& request,
                   std::string& collection) const;

    bool checkData(const std::string& data) const;
    
};


NS_IZENELIB_SF1R_END


#endif	/* JSONWRITER_HPP */
