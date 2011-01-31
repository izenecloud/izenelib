#ifndef IZENELIB_DRIVER_VALUE_VALUE_TYPE_HELPER_H
#define IZENELIB_DRIVER_VALUE_VALUE_TYPE_HELPER_H
/**
 * @file izenelib/driver/value/ValueTypeHelper.h
 * @author Ian Yang
 * @date Created <2010-06-08 15:07:25>
 * @brief Type of the value
 */

#include "macros.h"
#include "types.h"

#include <boost/noncopyable.hpp>

namespace izenelib {
namespace driver {

class ValueTypeHelper : boost::noncopyable
{
public:
    static inline const ValueTypeHelper& instance()
    {
        static const ValueTypeHelper helper;
        return helper;
    }

    enum
    {
        /// @brief Total count of types.
        kTypeCount = BOOST_PP_SEQ_SIZE(IZENELIB_DRIVER_VALUE_TYPES)
    };

    bool validType(int typeId) const
    {
        return 0 <= typeId && typeId < kTypeCount;
    }

    const std::string& type2Name(int typeId) const
    {
        if (validType(typeId))
        {
            return typeNames_[typeId];
        }
        else
        {
            static std::string kUnknown = "Unknown";
            return kUnknown;
        }
    }

    int name2Type(const std::string& typeName) const;

private:
    ValueTypeHelper();

    static const std::string typeNames_[kTypeCount];
};

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_VALUE_VALUE_TYPE_HELPER_H
