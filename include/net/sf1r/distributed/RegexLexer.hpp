/* 
 * File:   RegexLexer.hpp
 * Author: Paolo D'Apice
 *
 * Created on March 27, 2012, 3:07 PM
 */

#ifndef REGEXLEXER_HPP
#define	REGEXLEXER_HPP

#include "../config.h"
#include <boost/regex.hpp>
#include <string>

NS_IZENELIB_SF1R_BEGIN

/**
 * Lexer using regular expressions.
 */
class RegexLexer {
public:
    /// Constructor. Initialize a new lexer with the given regex.
    RegexLexer(const std::string& regex) : pattern(regex) {}

    /// Destructor.
    ~RegexLexer() {}
    
    /// Check for string matches.
    bool match(const std::string& input) const {
        return boost::regex_match(input, pattern);
    }
    
    /// Return the regex.
    std::string regex() const {
        return pattern.str();
    }
    
private:
    boost::regex pattern;
};

NS_IZENELIB_SF1R_END

#endif	/* REGEXLEXER_HPP */

