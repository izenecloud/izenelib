#include <document-manager/Document.h>

namespace sf1v5_dummy
{
    Document::Document()
    {
    }

    void Document::clear()
    {
        docId_ = 0;
        title_.clear();
        content_.clear();
    }

    bool Document::operator() ( const Document & i, const Document & j )
    {
        return i.getId() < j.getId();
    }
}
