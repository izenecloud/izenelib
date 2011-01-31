#ifndef IZENELIB_DRIVER_PARSERS_ORDER_ARRAY_PARSER_H
#define IZENELIB_DRIVER_PARSERS_ORDER_ARRAY_PARSER_H
/**
 * @file izenelib/driver/parsers/OrderArrayParser.h
 * @author Ian Yang
 * @date Created <2010-06-11 18:26:37>
 */
#include "../Value.h"
#include "../Parser.h"

#include "OrderParser.h"

#include <vector>

namespace izenelib {
namespace driver {

class OrderArrayParser : public Parser
{
public:
    OrderArrayParser();

    bool parse(const Value& orders);

    const std::vector<OrderParser>& parsedOrders() const
    {
        return parsers_;
    }

    const OrderParser& parsedOrders(std::size_t index) const
    {
        return parsers_[index];
    }

    std::size_t parsedOrderCount() const
    {
        return parsers_.size();
    }

private:
    std::vector<OrderParser> parsers_;
};

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_PARSERS_ORDER_ARRAY_PARSER_H
