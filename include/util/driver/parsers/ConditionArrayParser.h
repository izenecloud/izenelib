#ifndef IZENELIB_DRIVER_PARSERS_CONDITION_ARRAY_PARSER_H
#define IZENELIB_DRIVER_PARSERS_CONDITION_ARRAY_PARSER_H
/**
 * @file izenelib/driver/parsers/ConditionArrayParser.h
 * @author Ian Yang
 * @date Created <2010-06-10 20:07:36>
 */
#include "../Value.h"
#include "../Parser.h"

#include "ConditionParser.h"

#include <vector>

namespace izenelib {
namespace driver {

/// @addtogroup parsers
/// @{

/**
 * @brief Parse \b conditions field.
 */
class ConditionArrayParser : public Parser
{
public:
    ConditionArrayParser();

    bool parse(const Value& conditions);

    const std::vector<ConditionParser>& parsedConditions() const
    {
        return parsers_;
    }

    const ConditionParser& parsedConditions(std::size_t index) const
    {
        return parsers_[index];
    }

    std::size_t parsedConditionCount() const
    {
        return parsers_.size();
    }

private:
    std::vector<ConditionParser> parsers_;
};

/// @}

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_PARSERS_CONDITION_ARRAY_PARSER_H
