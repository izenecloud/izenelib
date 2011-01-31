#ifndef IZENELIB_DRIVER_PARSER_H
#define IZENELIB_DRIVER_PARSER_H
/**
 * @file izenelib/driver/Parser.h
 * @author Ian Yang
 * @date Created <2010-06-10 19:14:54>
 */
#include "Value.h"

#include <string>

namespace izenelib {
namespace driver {

class Parser
{
public:
    Parser();
    virtual ~Parser();

    /// @brief Parses document and put result in root.
    virtual bool parse(const Value& value) = 0;

    const std::string& errorMessage() const
    {
        return error_;
    }
    const std::string& warningMessage() const
    {
        return warning_;
    }

protected:
    void clearMessages()
    {
        error_.clear();
        warning_.clear();
    }

    std::string& error()
    {
        return error_;
    }
    const std::string& error() const
    {
        return error_;
    }
    std::string& warning()
    {
        return warning_;
    }
    const std::string& warning() const
    {
        return warning_;
    }

private:
    std::string error_;
    std::string warning_;
};

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_PARSER_H
