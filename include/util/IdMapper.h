#ifndef UTIL_ID_MAPPER_H
#define UTIL_ID_MAPPER_H
/**
 * @file util/IdMapper.h
 * @author Ian Yang
 * @date Created <2009-10-21 14:35:13>
 * @date Updated <2009-10-30 13:16:47>
 * @brief light utility map entity to id
 */
#include <boost/unordered_map.hpp>

#include <map>
#include <utility>

namespace izenelib {
namespace util {

template<typename T, typename ID = std::size_t>
class IdMapper
{
    typedef boost::unordered_map<T, ID> value_id_map_type;

    // store pointer because unordered_map guarantee that pointer to element
    // never invalidates.
    typedef std::map<ID, const T*> id_value_map_type;

public:
    IdMapper()
    : value2id_()
    , id2value_()
    {
    }

    /**
     * @brief insert \a value in map.
     *
     * If \a value exists, just return the id. Otherwise inserts \a value and
     * return the new allocated id.
     */
    ID insert(const T& value)
    {
        std::pair<typename value_id_map_type::iterator, bool>
            result = value2id_.insert(std::make_pair(value, maxId() + 1));

        if (result.second) // new value
        {
            id2value_[result.first->second] = &result.first->first;
        }

        return result.first->second;
    }

    /**
     * @brief insert \a value in map with specified \a id.
     * @return \c false if \a value exists.
     */
    bool insert(const T& value, ID id)
    {
        std::pair<typename value_id_map_type::iterator, bool>
            result = value2id_.insert(std::make_pair(value, id));

        if (result.second) // new value
        {
            id2value_[result.first->second] = &result.first->first;
            return true;
        }

        return false;
    }

    const ID* findIdByValue(const T& value) const
    {
        typename value_id_map_type::const_iterator
            found = value2id_.find(value);

        if (found != value2id_.end())
        {
            return &(found->second);
        }

        return 0;
    }

    const T* const findValueById(ID id) const
    {
        typename id_value_map_type::const_iterator
            found = id2value_.find(id);

        if (found != id2value_.end())
        {
            return found->second;
        }

        return 0;
    }

    ID maxId()
    {
        if (id2value_.empty())
        {
            return 0;
        }

        return id2value_.rbegin()->first;
    }

private:

    value_id_map_type value2id_;
    id_value_map_type id2value_;
};

}} // namespace izenelib::util

#endif // UTIL_ID_MAPPER_H
