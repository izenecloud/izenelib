#include <util/kv2string.h>

#include <sstream>

#include <boost/lexical_cast.hpp>

namespace izenelib {
namespace util {

static std::string encode(const std::string& input_orig_value)
{
    std::string orig_value = input_orig_value;
    // replace % and $,#
    size_t i = 0; 
    while(i < orig_value.size())
    {
        if(orig_value[i] == '%')
        {
            orig_value.replace(i, 1, "%25");
            i += 3;
        }
        else if(orig_value[i] == '#')
        {
            orig_value.replace(i, 1, "%23");
            i += 3;
        }
        else if(orig_value[i] == '$')
        {
            orig_value.replace(i, 1, "%24");
            i += 3;
        }
        else
        {
            ++i;
        }
    }
    return orig_value;
}

static std::string decode(const std::string& input_orig_value)
{
    std::string orig_value = input_orig_value;
    // replace % and $,#
    size_t i = 0; 
    while(i < orig_value.size())
    {
        if(orig_value[i] == '%')
        {
            if(orig_value[i+1]=='2' && orig_value[i+2] == '5')
                orig_value.replace(i, 3, "%");
            else if(orig_value[i+1]=='2' && orig_value[i+2] == '3')
                orig_value.replace(i, 3, "#");
            else if(orig_value[i+1]=='2' && orig_value[i+2] == '4')
                orig_value.replace(i, 3, "$");
            else
            {
                std::cerr << "warning: unknow encode value string :" << orig_value.substr(i, 3).c_str() << std::endl;
                orig_value.replace(i, 3, "");
            }
        }
        ++i;
    }
    return orig_value;
}

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

std::string kv2string::getStrValue(const std::string& key) const
{
    map_const_iter_t it = kvMap_.find(key);
    if (it == kvMap_.end())
        return "";
    return it->second;
}

uint32_t kv2string::getUInt32Value(const std::string& key) const
{
    uint32_t value = 0;
    map_const_iter_t it = kvMap_.find(key);
    if (it != kvMap_.end())
    {
        std::istringstream iss(it->second);
        iss >> value;
    }

    return value;
}

uint64_t kv2string::getUInt64Value(const std::string& key) const
{
    uint64_t value = 0;

    map_const_iter_t it = kvMap_.find(key);
    if (it != kvMap_.end())
    {
        std::istringstream iss(it->second);
        iss >> value;
    }

    return value;
}

bool kv2string::getValue(const std::string& key, unsigned int& value) const
{
    value = 0;

    map_const_iter_t it = kvMap_.find(key);
    if (it != kvMap_.end())
    {
        try {
            value = boost::lexical_cast<unsigned int>(it->second);
            return true;
        } catch (std::exception& e) {
            return false;
        }
    }

    return false;
}

bool kv2string::getValue(const std::string& key, std::string& value) const
{
    map_const_iter_t it = kvMap_.find(key);
    if (it != kvMap_.end())
    {
        value = it->second;
        return true;
    }

    return false;
}

std::string kv2string::serialize(bool verbose) const
{
    std::ostringstream oss;

    map_const_iter_t it = kvMap_.begin();
    while (it != kvMap_.end())
    {
        oss << encode(it->first) << k_vDelim_ << encode(it->second);

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
            kvMap_[decode(key)] = decode(value);
            if (verbose)
                std::cout<<"[deserialize] "<< decode(key) <<" - "<< decode(value) <<std::endl;
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

    kvMap_[decode(key)] = decode(value);
    if (verbose)
        std::cout<<"[deserialize] "<< decode(key) <<" - "<< decode(value) <<std::endl;
}


}} // namespace






