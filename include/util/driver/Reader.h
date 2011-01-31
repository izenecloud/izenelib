#ifndef IZENELIB_DRIVER_READER_H
#define IZENELIB_DRIVER_READER_H
/**
 * @file core/driver/Reader.h
 * @author Ian Yang
 * @date Created <2010-06-09 16:09:59>
 */
#include "Value.h"
#include "Request.h"

#include <string>

namespace izenelib {
namespace driver {

class Reader
{
public:
    Reader();
    virtual ~Reader();

    /// @brief Parses document and put result in root.
    virtual bool read(const std::string& document, Value& value) = 0;

    const std::string& errorMessages() const
    {
        return error_;
    }

protected:
    std::string& error()
    {
        return error_;
    }
    const std::string& error() const
    {
        return error_;
    }

private:
    std::string error_;
};

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_READER_H
