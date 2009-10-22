#ifndef DOCIDMAP_H
#define DOCIDMAP_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/IndexInput.h>
#include <ir/index_manager/store/IndexOutput.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

class DocIdMap
{
public:
    DocIdMap(Directory* pDirectory);
    ~DocIdMap();
public:
    void add(docid_t docId, docid_t val);
	
    docid_t getDocId(docid_t docId);

    void load();

    void flush();

    void setDirty(bool dirty) { dirty_ = dirty; }

    bool isDirty() { return dirty_; }
private:
    docid_t maxDoc_;
    docid_t* idMap_;
    bool dirty_;
};

}

NS_IZENELIB_IR_END

#endif
