#ifndef IZENELIB_DRIVER_PARSERS_CONDITION_PARSER_H
#define IZENELIB_DRIVER_PARSERS_CONDITION_PARSER_H
/**
 * @file izenelib/driver/parsers/ConditionParser.h
 * @author Ian Yang
 * @date Created <2010-06-10 19:33:04>
 */
#include "../Value.h"
#include "../Parser.h"

namespace izenelib {
namespace driver {

class ConditionParser: public Parser
{
public:
    ConditionParser();

    std::size_t size() const
    {
        return array_.size();
    }

    const Value& operator()(std::size_t index) const;

    const std::string& property() const
    {
        return property_;
    }

    const std::string& op() const
    {
        return op_;
    }

    bool parse(const Value& condition);

    void swap(ConditionParser& other)
    {
        using std::swap;
        swap(array_, other.array_);
        swap(property_, other.property_);
        swap(op_, other.op_);
        swap(error(), other.error());
    }

private:
    Value::ArrayType array_;
    std::string property_;
    std::string op_;
};

inline void swap(ConditionParser& a, ConditionParser& b)
{
    a.swap(b);
}

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_PARSERS_CONDITION_PARSER_H
