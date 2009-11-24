#ifndef UTIL_ID_MAPPER_H
#define UTIL_ID_MAPPER_H
/**
 * @file util/IdMapper.h
 * @author Ian Yang
 * @date Created <2009-10-21 14:35:13>
 * @date Updated <2009-11-24 11:04:55>
 * @brief light utility map entity to id
 */
#include <boost/unordered_map.hpp>

#include <map>
#include <utility>

namespace izenelib {
namespace util {

/**
 * @brief maps entity to unique id
 * @tparam T entity type
 * @tparam ID id type
 */
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

    /**
     * @brief find corresponding id by \a value
     * @return pointer to the id if found, \c zero pointer otherwise.
     */
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

    /**
     * @brief find corresponding value by \a id
     * @return pointer to the entity if found, \c zero pointer otherwise
     */
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

    /**
     * @brief gets max occupied id
     * @return max occupied id. 0 if none is occupied.
     */
    ID maxId()
    {
        if (id2value_.empty())
        {
            return 0;
        }

        return id2value_.rbegin()->first;
    }

private:

    value_id_map_type value2id_; /**< map from value to id */
    id_value_map_type id2value_; /**< map from id to value */
};

}} // namespace izenelib::util

#endif // UTIL_ID_MAPPER_H
