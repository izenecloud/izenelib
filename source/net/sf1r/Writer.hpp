/* 
 * File:   Writer.hpp
 * Author: Paolo D'Apice
 *
 * Created on January 10, 2012, 11:10 AM
 */

#ifndef WRITER_HPP
#define	WRITER_HPP

#include <boost/noncopyable.hpp>
#include <string>


NS_IZENELIB_SF1R_BEGIN


/**
 * Interface for request body writers.
 */
class Writer : private boost::noncopyable {
public:
    
    /**
     * Adds SF1 header into the body of the request.
     * @param controller The controller of this request.
     * @param action The action of this request.
     * @param tokens Custom tokens.
     * @param request The body of this request to be modified.
     * @param collection [out] The collection on which the request refers to.
     */
    virtual void setHeader(const std::string& controller, 
                           const std::string& action,
                           const std::string& tokens,
                           std::string& request,
                           std::string& collection) const = 0;
    
    /**
     * Checks if the data is valid.
     * @param data the data to be checked.
     * @return true if data is valid, false otherwise.
     */
    virtual bool checkData(const std::string& data) const = 0;
    
};


NS_IZENELIB_SF1R_END

#endif	/* WRITER_HPP */
