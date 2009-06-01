/**
 * @file compression/qsufsort/qsufsortImpl.h
 * @author Ian Yang
 * @date Created <2009-06-01 10:03:23>
 * @date Updated <2009-06-01 15:53:05>
 */
#ifndef COMPRESSION_QSUFSORT_QSUFSORT_IMPL_H
#define COMPRESSION_QSUFSORT_QSUFSORT_IMPL_H

namespace izenelib {
namespace compression {
namespace qsufsort {

template<typename StringIteratorT,
         typename SAIteratorT,
         typename PositionIteratorT>
OutputIteratorT qsufsortImpl(
    StringIteratorT first,
    StringIteratorT last,
    SAIteratorT result,
    PositionIteratorT position
)
{

}

}}} // namespace izenelib::compression::qsufsort

#endif // COMPRESSION_QSUFSORT_QSUFSORT_IMPL_H
