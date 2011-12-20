#include <util/kv2string.h>

#include <sstream>

#include <boost/lexical_cast.hpp>

namespace izenelib {
namespace util {

void kv2string::setValue(const std::string& key, const std::string& value)
{
    kvMap_[key] = value;
}

void kv2string::setValue(const std::string& key, const uint32_t value)
{
    std::ostringstream oss;
    oss << value;

    kvMap_[key] = oss.str();
}

void kv2string::setValue(const std::string& key, const uint64_t value)
{
    std::ostringstream oss;
    oss << value;

    kvMap_[key] = oss.str();
}

std::string kv2string::getStrValue(const std::string& key)
{
    return kvMap_[key];
}

uint32_t kv2string::getUInt32Value(const std::string& key)
{
    uint32_t value = 0;

    map_iter_t it = kvMap_.find(key);
    if (it != kvMap_.end())
    {
        std::istringstream iss(kvMap_[key]);
        iss >> value;
    }

    return value;
}

uint64_t kv2string::getUInt64Value(const std::string& key)
{
    uint64_t value = 0;

    map_iter_t it = kvMap_.find(key);
    if (it != kvMap_.end())
    {
        std::istringstream iss(kvMap_[key]);
        iss >> value;
    }

    return value;
}

bool kv2string::getValue(const std::string& key, unsigned int& value)
{
    value = 0;

    map_iter_t it = kvMap_.find(key);
    if (it != kvMap_.end())
    {
        //std::istringstream iss(kvMap_[key]);
        //iss >> value;

        try {
            value = boost::lexical_cast<unsigned int>(kvMap_[key]);
            return true;
        } catch (std::exception& e) {
            return false;
        }
    }

    return false;
}

bool kv2string::getValue(const std::string& key, std::string& value)
{
    map_iter_t it = kvMap_.find(key);
    if (it != kvMap_.end())
    {
        value = it->second;
        return true;
    }

    return false;
}

std::string kv2string::serialize(bool verbose)
{
    std::ostringstream oss;

    map_iter_t it = kvMap_.begin();
    while (it != kvMap_.end())
    {
        oss << it->first << k_vDelim_ << it->second;

        it++;
        if (it != kvMap_.end())
        {
            oss << kv_kvDelim_;
        }
    }

    return oss.str();
}

void kv2string::loadKvString(const std::string& data, bool verbose)
{
    deserialize(data, verbose);
}

void kv2string::deserialize(const std::string& data, bool verbose)
{
    kvMap_.clear();

    std::string::const_iterator it = data.begin();
    std::string::const_iterator itEnd = data.end();;

    std::string key;
    std::string value;
    bool isKey = true;

    while (it != itEnd)
    {
        if (*it == k_vDelim_)
        {
            isKey = false;
        }
        else if (*it ==  kv_kvDelim_)
        {
            kvMap_[key] = value;
            if (verbose)
                std::cout<<"[deserialize] "<<key<<" - "<<value<<std::endl;
            key.clear();
            value.clear();
            isKey = true;
        }
        else
        {
            if (isKey)
                key.push_back(*it);
            else
                value.push_back(*it);
        }

        it++;
    }

    kvMap_[key] = value;
    if (verbose)
        std::cout<<"[deserialize] "<<key<<" - "<<value<<std::endl;
}


}} // namespace






