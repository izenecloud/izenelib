/* 
 * File:   Sf1Protocol.hpp
 * Author: Paolo D'Apice
 *
 * Created on January 6, 2012, 3:56 PM
 */

#ifndef SF1PROTOCOL_HPP
#define	SF1PROTOCOL_HPP

namespace izenelib {
namespace net {
namespace sf1r {
    

const char* SF1_HEADER                  = "header";
const char* SF1_HEADER_CONTROLLER       = "controller";
const char* SF1_HEADER_ACTION           = "action";
const char* SF1_HEADER_TOKENS           = "acl_tokens";
const char* SF1_HEADER_CHECKTIME        = "check_time";


}}} /* namespace izenelib::net::sf1r */

#endif	/* SF1PROTOCOL_HPP */
