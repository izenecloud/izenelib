#ifndef AVMAPPER_H_
#define AVMAPPER_H_

#include "IDMapper.hpp"
#include <types.h>
#include <utility>
#include <string>
#include <vector>
#include <3rdparty/json/json.h>
#include "SimpleSerialization.hpp"

namespace izenelib { namespace ir { namespace be_index {

class AVMapper {
public:
    AVMapper()
    {
    }

    ~AVMapper()
    {
    }

    std::pair<std::pair<uint32_t, bool>, std::pair<uint32_t, bool> > insert(const std::string & attr, const std::string & value)
    {
        std::pair<uint32_t, bool> attrResult = attrMapper.insert(attr);
        if (attrResult.second == true) {
            valueMapper.push_back(IDMapper());
        }
        std::pair<uint32_t, bool> valueResult = valueMapper[attrResult.first].insert(value);
        return std::make_pair(attrResult, valueResult);
    }

    std::pair<std::pair<uint32_t, bool>, std::pair<uint32_t, bool> > has(const std::string & attr, const std::string & value)
    {
        std::pair<uint32_t, bool> hasAttr = attrMapper.has(attr);
        if (hasAttr.second == false) {
            return std::make_pair(hasAttr, std::make_pair(0U, false));
        }
        return std::make_pair(hasAttr, valueMapper[hasAttr.first].has(value));
    }

    void toJson(Json::Value & root)
    {
        attrMapper.toJson(root["attrMapper"]);
        valueMapperToJson(root["valueMapper"]);
    }

    void valueMapperToJson(Json::Value & root)
    {
        for (std::size_t i = 0; i != valueMapper.size(); ++i) {
            valueMapper[i].toJson(root[i]);
        }
    }

    void fromJson(Json::Value & root)
    {
        attrMapper.fromJson(root["attrMapper"]);
        valueMapperFromJson(root["valueMapper"]);
    }

    void valueMapperFromJson(Json::Value & root)
    {
        valueMapper.resize(root.size());
        for (std::size_t i = 0; i != root.size(); ++i) {
            valueMapper[i].fromJson(root[i]);
        }
    }

    void save_binary(std::ostream & os)
    {
        attrMapper.save_binary(os);
        serialize(valueMapper.size(), os);
        for (std::size_t i = 0; i != valueMapper.size(); ++i) {
            valueMapper[i].save_binary(os);
        }
    }

    void load_binary(std::istream & is)
    {
        attrMapper.load_binary(is);
        std::size_t size;
        deserialize(is, size);
        valueMapper.resize(size);
        for (std::size_t i = 0; i != size; ++i) {
            valueMapper[i].load_binary(is);
        }
    }

private:
    IDMapper attrMapper;
    std::vector<IDMapper> valueMapper;
};

}}}

#endif
