/**
* @file        PostingWriter.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief posting builder
*/

#ifndef POSTING_WRITER_H
#define POSTING_WRITER_H


NS_IZENELIB_IR_BEGIN

namespace indexmanager{

template<typename CompressedPostingWriter>
class PostingWriter
{
public:
    void add(uint32_t docId, uint32_t pos)
    {
        downcast()->add(docId,pos);
    }

protected:
    inline CompressedPostingWriter * downcast()
    {
        return static_cast<CompressedPostingWriter *>(this);
    };
    inline const CompressedPostingWriter * downcast()const
    {
        return static_cast<const CompressedPostingWriter *>(this);
    };

};

}

NS_IZENELIB_IR_END


#endif
