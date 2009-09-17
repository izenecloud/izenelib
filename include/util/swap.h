#ifndef UTIL_SWAP_H
#define UTIL_SWAP_H
/**
 * @file util/swap.h
 * @author Ian Yang
 * @date Created <2009-09-08 17:52:25>
 * @date Updated <2009-09-16 17:16:37>
 * @brief use swap to replace copy when the right we indeed just need "move"
 */

#include <utility> // for std::swap

namespace izenelib {
namespace util {

/**
 * @brief alternative of \code container.push_back(value) \endcode
 * @tparam C container supporting resize, size and back.
 * @tparam T swappable, default constructable
 * @param container to which the value will be swapped back
 * @param value element to be swapped back
 * @post value == T(), value has be swapped with a default constructed T
 */
template<typename C, typename T>
void swapBack(C& container, T& value)
{
    container.resize(container.size() + 1);

    using std::swap;
    swap(container.back(), value);
}

}} // namespace izenelib::util
#endif // UTIL_SWAP_H
