#include <ZkDataPack.hpp>

#include <sstream>

using namespace zookeeper;

void ZkDataPack::setValue(const std::string& key, const std::string& value)
{
    kvMap_[key] = value;
}

void ZkDataPack::setValue(const std::string& key, const unsigned int value)
{
    std::ostringstream oss;
    oss << value;

    kvMap_[key] = oss.str();
}

std::string ZkDataPack::getValue(const std::string& key)
{
    return kvMap_[key];
}

bool ZkDataPack::getValue(const std::string& key, unsigned int& value)
{
    value = 0;

    std::istringstream iss(kvMap_[key]);
    iss >> value;

    return value;
}

bool ZkDataPack::getValue(const std::string& key, std::string& value)
{
    map_iter_t it = kvMap_.find(key);
    if (it != kvMap_.end())
    {
        value = it->second;
        return true;
    }

    return false;
}

std::string ZkDataPack::serialize(bool verbose)
{
    std::ostringstream oss;

    map_iter_t it = kvMap_.begin();
    while (it != kvMap_.end())
    {
        oss << it->first << K_V_Delimiter << it->second;

        it++;
        if (it != kvMap_.end())
        {
            oss << KV_KV_Delimiter;
        }
    }

    return oss.str();
}

void ZkDataPack::loadZkData(const std::string& data, bool verbose)
{
    deserialize(data, verbose);
}

void ZkDataPack::deserialize(const std::string& data, bool verbose)
{
    kvMap_.clear();

    std::string::const_iterator it = data.begin();
    std::string::const_iterator itEnd = data.end();;

    std::string key;
    std::string value;
    bool isKey = true;

    while (it != itEnd)
    {
        switch (*it)
        {
            case K_V_Delimiter:
            {
                isKey = false;
                break;
            }
            case KV_KV_Delimiter:
            {
                kvMap_[key] = value;
                if (verbose)
                    std::cout<<"[deserialize] "<<key<<" - "<<value<<std::endl;
                key.clear();
                value.clear();
                isKey = true;
                break;
            }
            default:
            {
                if (isKey)
                    key.push_back(*it);
                else
                    value.push_back(*it);
            }
        }

        it++;
    }

    kvMap_[key] = value;
    if (verbose)
        std::cout<<"[deserialize] "<<key<<" - "<<value<<std::endl;
}
