#include <boost/algorithm/string.hpp>
#include <iostream>

#include <util/sysinfo/commandparser.h>
#include <cstdio>

NS_IZENELIB_UTIL_BEGIN

namespace sysinfo
{

CommandParser::CommandParser()
{
}

CommandParser::~CommandParser()
{
}

std::string CommandParser::exec(const std::string & cmd)
{
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe))
    {
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result;
}

std::vector<std::string> CommandParser::parse(std::string cmd)
{
    std::string s = exec( cmd );
    lines_.clear();
    // Split output in vector of lines
    boost::split(lines_, s, boost::is_any_of("\n\r"), boost::token_compress_on);
    for (unsigned int i = 0; i < lines_.size() ; ++i)
    {
        boost::trim(lines_[i]);
        // Skip empty lines
        if (lines_[i].empty())
            continue;
    }
    // remove empty lines
    std::remove(lines_.begin(), lines_.end(), "");

    return lines_;
}
std::vector<std::vector<std::string> > CommandParser::split(std::string splitString)
{
    fields_.clear();
    for (unsigned int i = 0; i < lines_.size() ; ++i)
    {
        std::vector<std::string> fields;
        boost::split(fields, lines_[i], boost::is_any_of(splitString), boost::token_compress_on);
        for (int j = 0; j < (int)fields.size(); ++j)
        {
            boost::trim(fields[j]);
        }
        fields_.push_back(fields);
    }
    return fields_;
}
void CommandParser::printLines()
{
    for (unsigned int i = 0; i < lines_.size() ; ++i)
    {
        std::cout << lines_[i] << std::endl;
    }
}
void CommandParser::printFields()
{
    for (unsigned int i = 0; i < fields_.size() ; ++i)
    {
        for (unsigned int j = 0; j < fields_[i].size() ; ++j)
        {
            std::cout << fields_[i][j] << ":";
        }
        std::cout << std::endl;
    }
}

}

NS_IZENELIB_UTIL_END

