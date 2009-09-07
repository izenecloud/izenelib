#ifndef AM_RAW_WRITE_IMAGE_H
#define AM_RAW_WRITE_IMAGE_H
/**
 * @file am/raw/WriteImage.h
 * @author Ian Yang
 * @date Created <2009-09-06 23:17:37>
 * @date Updated <2009-09-06 23:18:56>
 * @brief Write image to Buffer in izene_serialization
 */

#include "Buffer.h"

#include <util/izene_serialization.h>

namespace izenelib {
namespace am {
namespace raw {

template<typename T>
inline void write_image(
    izenelib::util::izene_serialization<T>& izs,
    Buffer& buf
)
{
    izs.write_image(buf.data_, buf.size_);
}

}}} // namespace izenelib::am::raw

#endif // AM_RAW_WRITE_IMAGE_H
