#ifndef SF1COMMON_SCDDOCUMENT_H
#define SF1COMMON_SCDDOCUMENT_H
#include "type_defs.h"
#include "ScdParser.h"
#include "Document.h"

namespace izenelib 
{
class ScdDocument : public Document
{
public:
    ScdDocument():Document(), type(NOT_SCD)
    {
    }
    ScdDocument(SCD_TYPE t):Document(), type(t)
    {
    }
    ScdDocument(const Document& d, SCD_TYPE t):Document(d), type(t)
    {
    }
    ScdDocument(const ScdDocument& d):Document(d), type(d.type)
    {
    }

    void merge(const ScdDocument& doc)
    {
        type=doc.type;
        if(type==NOT_SCD) clear();
        else
        {
            Document::merge(doc);
        }
    }

public:
  SCD_TYPE type;
};
}

#endif
