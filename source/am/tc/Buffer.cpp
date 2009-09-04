/**
 * @file am/tokyo_cabinet/Buffer.cpp
 * @author Ian Yang
 * @date Created <2009-09-02 13:43:33>
 * @date Updated <2009-09-04 14:59:58>
 */
#include <am/tokyo_cabinet/Buffer.h>

namespace izenelib {
namespace am {
namespace tc {

Buffer& Buffer::operator=(const Buffer& rhs)
{
    if (this != &rhs)
    {
        if (data_ != rhs.data_ && deleter_)
        {
            (*deleter_)(data_);
        }

        char* data = static_cast<char*>(std::malloc(rhs.size_));
        if (data)
        {
            std::memcpy(data, rhs.data_, rhs.size_);
            data_ = data;
            size_ = rhs.size_;
            deleter_ = &std::free;
        }
    }

    return *this;
}


}}} // namespace izenelib::am::tc
