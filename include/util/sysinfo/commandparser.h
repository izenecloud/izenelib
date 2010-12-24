#ifndef __COMMANDPARSER_H__
#define __COMMANDPARSER_H__

#include <string>
#include <vector>

#include <types.h>

NS_IZENELIB_UTIL_BEGIN

namespace sysinfo
{

class CommandParser
{
public:
    CommandParser();
    ~CommandParser();
    std::string exec(const std::string & cmd);
    std::vector<std::string> parse(std::string cmd);
    std::vector<std::vector<std::string> > split(std::string splitString);
    void printLines();
    void printFields();

private:
    std::vector<std::string> lines_;
    std::vector<std::vector<std::string> > fields_;
};

}

NS_IZENELIB_UTIL_END

#endif	// __COMMANDPARSER_H__

