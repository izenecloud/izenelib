/**
 * @file core/driver/value/ValueTypeHelper.cpp
 * @author Ian Yang
 * @date Created <2010-06-07 17:18:12>
 */
#include <util/driver/value/ValueTypeHelper.h>
#include <algorithm>

namespace izenelib {
namespace driver {

ValueTypeHelper::ValueTypeHelper()
{}

int ValueTypeHelper::name2Type(const std::string& typeName) const
{
    return std::find(typeNames_, typeNames_ + kTypeCount, typeName) -
        typeNames_;
}

const std::string ValueTypeHelper::typeNames_[kTypeCount] = {
    IZENELIB_DRIVER_VALUE_NAME_STR_LIST(IZENELIB_DRIVER_VALUE_TYPES)
};

}} // namespace izenelib::driver


