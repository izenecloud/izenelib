#ifndef IDMAPPER_H_
#define IDMAPPER_H_

#include <utility>
#include <string>
#include <iostream>
#include <boost/unordered_map.hpp>
#include <3rdparty/json/json.h>

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

private:
    boost::unordered_map<std::string, uint32_t> dict;
};

}}}

#endif
