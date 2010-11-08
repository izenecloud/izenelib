#include <ir/index_manager/index/Term.h>

using namespace std;

using namespace izenelib::ir::indexmanager;

Term::Term(const char* field_) : field(field_), value(0)
{
}
Term::Term(const string& field_) : field(field_), value(0)
{
}
Term::Term(const char* field_, termid_t value_) : field(field_), value(value_)
{
}
Term::Term(const Term& term) : field(term.field), value(term.value)
{
}

int64_t Term::compare(const Term* pOther)
{
    if (field == pOther->field)
        return ((uint64_t)value - (uint64_t)(pOther->value));

    return field.compare(pOther->field);
}

