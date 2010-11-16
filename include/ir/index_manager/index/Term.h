/**
* @file        Term.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief
*/

#ifndef TERM_H
#define TERM_H

#include <ir/index_manager/utility/system.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
///Term is an encapsulation of token id and field name
class Term
{
public:
    Term(const char* field);

    Term(const std::string& field);

    Term(const char* field,termid_t value);

    Term(const Term& term);

    ~Term() {}
public:
    const char* getField()const
    {
        return field.c_str();
    }

    void setField(const char* field)
    {
        this->field = field;
    }

    termid_t getValue()
    {
        return value;
    }

    void setValue(termid_t value)
    {
        this->value = value;
    }

    void copy(const Term& term)
    {
        field = term.field;
        value = term.value;
    }

    int64_t compare(const Term* pOther);

    Term* clone()
    {
        return new Term(*this);
    }

public:
    std::string field;
    termid_t value;
};
}

NS_IZENELIB_IR_END

#endif
