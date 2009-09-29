#ifndef UTIL_GET_H
#define UTIL_GET_H
/**
 * @file util/get.h
 * @author Ian Yang
 * @date Created <2009-09-28 17:24:24>
 * @date Updated <2009-09-28 17:27:32>
 * @brief Get helper functions
 */

namespace izenelib {
namespace util {

template<typename MapT>
const typename MapT::mapped_type&
getOr(const MapT& m,
      const typename MapT::key_type& key,
      const typename MapT::mapped_type& def)
{
    typedef typename MapT::const_iterator iterator;
    iterator found = m.find(key);
    if (found != m.end())
    {
        return found->second;
    }

    return def;
}

}} // namespace izenelib::util

#endif // UTIL_GET_H
