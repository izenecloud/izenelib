/* 
 * File:   JsonWriter.hpp
 * Author: Paolo D'Apice
 *
 * Created on January 10, 2012, 11:18 AM
 */

#ifndef JSONWRITER_HPP
#define	JSONWRITER_HPP

#include "Writer.hpp"

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
    
private:
    // Disable copy
    JsonWriter(const JsonWriter&);
    void operator=(const JsonWriter&);
};

#endif	/* JSONWRITER_HPP */
