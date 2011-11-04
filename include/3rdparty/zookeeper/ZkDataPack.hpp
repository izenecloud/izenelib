/**
 * @file ZkDataPack.hpp
 * @author Zhongxia Li
 * @date Nov 3, 2011
 * @brief 
 */
#ifndef ZK_DATA_PACK_HPP_
#define ZK_DATA_PACK_HPP_

#include <iostream>
#include <map>
#include <string>

namespace zookeeper {

class ZkDataPack
{
    typedef std::map<std::string, std::string> map_t;
    typedef std::map<std::string, std::string>::iterator map_iter_t;
    map_t kvMap_;

    static const char KV_KV_Delimiter = '$';
    static const char K_V_Delimiter = '#';

public:
    void setValue(const std::string& key, const std::string& value);
    void setValue(const std::string& key, const unsigned int value);

    std::string getValue(const std::string& key);
    bool getValue(const std::string& key, std::string& value);
    bool getValue(const std::string& key, unsigned int& value);

    bool hasKey(const std::string& key) { return (kvMap_.find(key) != kvMap_.end()); }
    void clear() { kvMap_.clear(); }
    bool empty() const { return kvMap_.empty(); }
    size_t size() const { return kvMap_.size(); }

    /**
     * key-values to string
     */
    std::string serialize(bool verbose = false);

    /**
     * load key-values from string
     */
    void loadZkData(const std::string& data, bool verbose = false);
    void deserialize(const std::string& data, bool verbose = false);
};

}

#endif
