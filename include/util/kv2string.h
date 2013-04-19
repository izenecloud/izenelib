#ifndef IZENELIB_UTIL_KV2STRING_H_
#define IZENELIB_UTIL_KV2STRING_H_

#include <iostream>
#include <map>
#include <string>

#include <boost/unordered_map.hpp>

namespace izenelib {
namespace util {

class kv2string
{
    //typedef std::map<std::string, std::string> map_t;
    //typedef std::map<std::string, std::string>::iterator map_iter_t;
    typedef boost::unordered_map<std::string, std::string> map_t;
    typedef map_t::const_iterator map_const_iter_t;
    typedef map_t::iterator map_iter_t;
    map_t kvMap_;

    // xxx, use as escape sequence for key,value
    static const char DEFAULT_KV_KV_DELIMITER = '$';
    static const char DEFAULT_K_V_DELIMITER = '#';

    char kv_kvDelim_;
    char k_vDelim_;

public:
    kv2string(
        const char kv_kvDelim=DEFAULT_KV_KV_DELIMITER,
        const char k_vDelim=DEFAULT_K_V_DELIMITER)
    : kv_kvDelim_(kv_kvDelim)
    , k_vDelim_(k_vDelim)
    {
    }

public:
    void setValue(const std::string& key, const std::string& value);
    void setValue(const std::string& key, const uint32_t value);
    void setValue(const std::string& key, const uint64_t value);

    std::string getStrValue(const std::string& key) const;
    unsigned int getUInt32Value(const std::string& key) const;
    uint64_t getUInt64Value(const std::string& key) const;
    bool getValue(const std::string& key, std::string& value) const;
    bool getValue(const std::string& key, unsigned int& value) const;

    bool hasKey(const std::string& key) const { return (kvMap_.find(key) != kvMap_.end()); }
    void clear() { kvMap_.clear(); }
    bool empty() const { return kvMap_.empty(); }
    size_t size() const { return kvMap_.size(); }

    /**
     * key-values to string
     */
    std::string serialize(bool verbose = false) const;

    /**
     * load key-values from string
     */
    void loadKvString(const std::string& data, bool verbose = false);
    void deserialize(const std::string& data, bool verbose = false);
};

}} // namespace

#endif
