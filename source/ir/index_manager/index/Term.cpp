#include <ir/index_manager/index/Term.h>

using namespace std;

using namespace izenelib::ir::indexmanager;

Term::Term(const char* field_) : field(field_)
{
}
Term::Term(const string& field_) : field(field_)
{
}
Term::Term(const char* field_, termid_t value_) : field(field_),value(value_)
{
}
Term::Term(const Term& term)
{
    field = term.field;
    value = term.value;
}

int32_t Term::compare(const Term* pOther)
{
    if (field == pOther->field)
        return (int32_t)(value - pOther->value);

    return field.compare(pOther->field);
}

