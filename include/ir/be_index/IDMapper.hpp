#ifndef IDMAPPER_H_
#define IDMAPPER_H_

#include <types.h>
#include <utility>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <boost/unordered_map.hpp>
#include <3rdparty/json/json.h>
#include "SimpleSerialization.hpp"

namespace izenelib { namespace ir { namespace be_index {

class IDMapper {
public:
    IDMapper()
    {
    }

    ~IDMapper()
    {
    }

    std::pair<uint32_t, bool> insert(const std::string & attr)
    {
        typedef boost::unordered_map<std::string, uint32_t> dictT;

        dictT::iterator i = dict.find(attr);
        if (i != dict.end()) {
            return std::make_pair(i->second, false);
        } else {
            uint32_t ret = dict.size();
            dict.insert(std::make_pair(attr, ret));
            return std::make_pair(ret, true);
        }
    }

    std::pair<uint32_t, bool> has(const std::string & attr)
    {
        typedef boost::unordered_map<std::string, uint32_t> dictT;

        dictT::iterator i = dict.find(attr);
        if (i != dict.end()) {
            return std::make_pair(i->second, true);
        } else {
            return std::make_pair(0U, false);
        }
    }

    uint32_t size()
    {
        return dict.size();
    }

    void toJson(Json::Value & root)
    {
        dictToJson(root["dict"]);
    }

    void dictToJson(Json::Value & root)
    {
        for (boost::unordered_map<std::string, uint32_t>::iterator i = dict.begin(); i != dict.end(); ++i) {
            root[i->first] = Json::Value::UInt(i->second);
        }
    }

    void fromJson(Json::Value & root)
    {
        dictFromJson(root["dict"]);
    }

    void dictFromJson(Json::Value & root)
    {
        for (Json::ValueIterator i = root.begin(); i != root.end(); ++i) {
            dict[i.key().asString()] = (*i).asUInt();
        }
    }

    void save_binary(std::ostream & os)
    {
        serialize(dict.size(), os);
        std::vector<std::pair<std::string, uint32_t> > temp(dict.begin(), dict.end());
        std::sort(temp.begin(), temp.end());
        for (std::size_t i = 0; i != temp.size(); ++i) {
            serialize(temp[i].first, os);
            serialize(temp[i].second, os);
        }
    }

    void load_binary(std::istream & is)
    {
        dict.clear();

        std::size_t size;
        deserialize(is, size);
        for (std::size_t i = 0; i != size; ++i) {
            std::string s;
            deserialize(is, s);
            uint32_t id;
            deserialize(is, id);
            dict[s] = id;
        }
    }

private:
    boost::unordered_map<std::string, uint32_t> dict;
};

}}}

#endif
