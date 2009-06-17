#ifndef FORWARDINDEXREADER_H
#define FORWARDINDEXREADER_H

#include <ir/index_manager/store/Directory.h>
#include <ir/index_manager/store/IndexInput.h>

#include <ir/index_manager/index/ForwardIndex.h>

#include <3rdparty/am/rde_hashmap/hash_map.h>

#include <vector>


NS_IZENELIB_IR_BEGIN

namespace indexmanager{

typedef rde::hash_map<unsigned int, fileoffset_t> VocInfoMap;

class ForwardIndexReader
{
public:
    ForwardIndexReader(Directory* pDirectory);

    ~ForwardIndexReader();
public:
    ForwardIndexReader* clone();

    bool getTermOffset(unsigned int termId, docid_t docId, fieldid_t fid, std::vector<std::pair<unsigned int, unsigned int> >& offsetList);

    bool getTermOffsetList(const std::vector<unsigned int>& termIds, docid_t docId, fieldid_t fid, std::vector<std::vector<std::pair<unsigned int, unsigned int> > >& offsetList);

    bool getForwardIndexByDoc(docid_t docId, fieldid_t fid, ForwardIndex& forwardIndex);	

private:
    inline bool locateTermPosByDoc(docid_t docId, fieldid_t fid);

    inline void retrieve_voc_by_doc(VocInfoMap & vocInfo);

private:
    Directory* pDirectory_;

    IndexInput* pDOCInput_;

    IndexInput* pFDIInput_;

    IndexInput* pVOCInput_;

    IndexInput* pPOSInput_;
};

}
NS_IZENELIB_IR_END

#endif
