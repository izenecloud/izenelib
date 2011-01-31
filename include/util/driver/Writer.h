#ifndef IZENELIB_DRIVER_WRITER_H
#define IZENELIB_DRIVER_WRITER_H
/**
 * @file core/driver/Writer.h
 * @author Ian Yang
 * @date Created <2010-06-09 16:14:35>
 */

#include "Value.h"

#include <string>

namespace izenelib {
namespace driver {

class Writer
{
public:
    Writer();
    virtual ~Writer();

    /// @brief Parses document and put result in root.
    virtual void write(const Value& value, std::string& document) = 0;
};

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_WRITER_H
