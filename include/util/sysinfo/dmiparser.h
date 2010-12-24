#ifndef __DMIPARSER_H__
#define __DMIPARSER_H__

#include <map>
#include <vector>
#include <string>

#include <types.h>

NS_IZENELIB_UTIL_BEGIN

namespace sysinfo
{

class DMIParser
{
public:
    typedef std::map<std::string, std::string> StringStringMap;
    typedef std::map<std::string, std::vector<std::string> > StringVectorMap;
    enum Types
    {
        Bios = 0,
        System = 1,
        MotherBoard = 2,
        Chassis = 3,
        OnBoard = 10,
        Memory = 17
    };
    struct Frame
    {
        std::string Description;
        std::string Handle;
        StringStringMap Data;
        StringVectorMap FeatureData;
    };


    DMIParser(Types type);
    ~DMIParser();

    void exec();
    std::string operator[](std::string index);
    void setCurrentFrame(int frame);
    int currentFrame();
    int frameCount();

private:
    Types type_;
    int currentFrame_;
    std::vector<Frame> frames_;
};

}

NS_IZENELIB_UTIL_END
#endif	// __DMIPARSER_H__

