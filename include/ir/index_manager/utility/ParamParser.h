/**
* @file       ParamParser.h
* @version     SF1 v5.0
* @brief
*/

#ifndef PARAMPARSER_H
#define PARAMPARSER_H

#include <ir/index_manager/utility/system.h>
#include <vector>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{
///Helper class for DBTIndexMerger's param parsing
class ParamParser
{
public:
    class Item
    {
    public:
        Item(const std::string& _name,const std::string& _value)
                :name(_name)
                ,value(_value)
        {}
    public:
        const char* getName()
        {
            return name.c_str();
        }
        const char* getValue()
        {
            return value.c_str();
        }
    public:
        std::string name;
        std::string value;
    };
public:
    ParamParser();
    ~ParamParser();
public:
    bool parse(const char* szParamString,const char* szSep = ";",const char* szEq = "=");
    bool parse2(const char* szParamString,const char* szSep = ":");
    size_t getParamCount()
    {
        return items.size();
    }
    bool getParam(const char* name,int32_t& value);
    bool getParam(const char* name,int64_t& value);
    bool getParam(const char* name,float& value);
    bool getParam(const char* name,std::string& value);
    bool getParam(size_t _off,std::string& name,std::string& value);
    bool getParam2(size_t _off,std::string& value);
protected:
    ParamParser::Item* getItem(const char* name);
protected:
    std::vector<Item*> items;
};


}

NS_IZENELIB_IR_END

#endif

