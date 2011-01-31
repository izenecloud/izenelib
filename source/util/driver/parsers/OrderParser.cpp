/**
 * @file core/driver/parsers/OrderParser.cpp
 * @author Ian Yang
 * @date Created <2010-06-11 18:14:33>
 */
#include <util/driver/parsers/OrderParser.h>
#include <util/driver/Keys.h>

#include <boost/algorithm/string/case_conv.hpp>

namespace izenelib {
namespace driver {

bool OrderParser::parse(const Value& order)
{
    clearMessages();
    property_.clear();
    // default is ascendant
    ascendant_ = true;

    if (order.type() == Value::kObjectType)
    {
        property_ = asString(order[Keys::property]);
        std::string orderField = asString(order[Keys::order]);
        boost::to_lower(orderField);
        if (orderField == "desc")
        {
            ascendant_ = false;
        }
    }
    else
    {
        property_ = asString(order);
    }

    if (property_.empty())
    {
        error() = "Require property name in sort.";
        return false;
    }

    return true;
}

}} // namespace izenelib::driver
