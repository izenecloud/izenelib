/**
 * @file core/driver/parsers/PageInfoParser.cpp
 * @author Ian Yang
 * @date Created <2010-06-10 19:13:08>
 */
#include <util/driver/parsers/PageInfoParser.h>
#include <util/driver/Keys.h>

namespace izenelib {
namespace driver {

bool PageInfoParser::parse(const Value& value)
{
    clearMessages();

    if (value.hasKey(Keys::offset))
    {
        offset_ = asUint(value[Keys::offset]);
    }

    if (value.hasKey(Keys::limit))
    {
        limit_ = asUint(value[Keys::limit]);
    }

    return true;
}

}} // namespace izenelib::driver

