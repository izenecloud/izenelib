#ifndef IZENELIB_DRIVER_PARSERS_ORDER_PARSER_H
#define IZENELIB_DRIVER_PARSERS_ORDER_PARSER_H
/**
 * @file izenelib/driver/parsers/OrderParser.h
 * @author Ian Yang
 * @date Created <2010-06-11 18:10:38>
 */
#include "../Value.h"
#include "../Parser.h"

#include <string>

namespace izenelib {
namespace driver {

class OrderParser : public Parser
{
public:
    bool parse(const Value& order);

    const std::string& property() const
    {
        return property_;
    }
    bool ascendant() const
    {
        return ascendant_;
    }

private:
    std::string property_;
    bool ascendant_;
};

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_PARSERS_ORDER_PARSER_H
