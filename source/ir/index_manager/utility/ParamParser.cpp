#include <ir/index_manager/utility/ParamParser.h>
#include <ir/index_manager/utility/StringUtils.h>

using namespace std;

using namespace izenelib::ir::indexmanager;

ParamParser::ParamParser()
{

}
ParamParser::~ParamParser()
{
    vector<Item*>::iterator iter = items.begin();
    while (iter != items.end())
    {
        delete (*iter);
        iter++;
    }
    items.clear();
}

bool ParamParser::parse(const char* szParamString,const char* szSep,const char* szEq)
{
    string sParam = szParamString;
    vector<string> params = split(sParam,szSep);
    if (params.size() <= 0)
        return false;
    bool bSuc = false;
    vector<string>::iterator iter = params.begin();
    while (iter != params.end())
    {
        vector<string> item = split(*iter,szEq);
        if (item.size() != 2)
        {
            iter++;
            continue;
        }
        bSuc = true;
        Item*  pItem = new Item(item[0],item[1]);
        items.push_back(pItem);
        iter++;
    }
    return bSuc;
}
bool ParamParser::parse2(const char* szParamString,const char* szSep)
{
    string sParam = szParamString;
    vector<string> params = split(sParam,szSep);
    if (params.size() <= 0)
        return false;
    vector<string>::iterator iter = params.begin();
    while (iter != params.end())
    {
        Item*  pItem = new Item(*iter,*iter);
        items.push_back(pItem);
        iter++;
    }
    return true;
}


bool ParamParser::getParam(const char* name,int32_t& value)
{
    Item* pItem = getItem(name);
    if (!pItem)
        return false;
    try
    {
        value = atoi(pItem->value.c_str());
    }
    catch (...)
    {
        return false;
    }
    return true;
}
bool ParamParser::getParam(const char* name,int64_t& value)
{
    Item* pItem = getItem(name);
    if (!pItem)
        return false;
    try
    {
        value = atoll(pItem->value.c_str());
    }
    catch (...)
    {
        return false;
    }
    return true;
}
bool ParamParser::getParam(const char* name,float& value)
{
    Item* pItem = getItem(name);
    if (!pItem)
        return false;
    try
    {
        value = (float)atof(pItem->value.c_str());
    }
    catch (...)
    {
        return false;
    }
    return true;
}
bool ParamParser::getParam(const char* name,string& value)
{
    Item* pItem = getItem(name);
    if (!pItem)
        return false;
    value = pItem->value;
    return true;
}
bool ParamParser::getParam(size_t _off,string& name,string& value)
{
    if (items.size() <= _off)
        return false;
    name = items[_off]->getName();
    value = items[_off]->getValue();
    return true;
}
bool ParamParser::getParam2(size_t _off,string& value)
{
    if (items.size() <= _off)
        return false;
    value = items[_off]->getName();
    return true;
}

ParamParser::Item* ParamParser::getItem(const char* name)
{
    vector<Item*>::iterator iter = items.begin();
    while (iter != items.end())
    {
        if (!strcasecmp(name,(*iter)->name.c_str()))
            return (*iter);
        iter++;
    }
    return NULL;
}

