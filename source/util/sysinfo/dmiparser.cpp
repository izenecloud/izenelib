
#include <util/sysinfo/dmiparser.h>
#include <util/sysinfo/commandparser.h>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

NS_IZENELIB_UTIL_BEGIN

namespace sysinfo
{

DMIParser::DMIParser(Types type)
{
    type_ = type;
    currentFrame_ = -1;
}
DMIParser::~DMIParser()
{
}

void DMIParser::exec()
{
    bool readDescription = false;
    bool readFeatureList = true;
    std::string currentFeature = "";
    int currentIndex = -1;
    CommandParser parser;
    std::vector<std::string> lines = parser.parse( str( boost::format("sudo dmidecode --type %s") % type_ ) );
    std::vector<std::vector<std::string> > fields = parser.split(":");
    // Process the lines one by one
    for (unsigned int i = 0; i < lines.size() ; ++i)
    {
        if (readDescription)
        {
            frames_[currentIndex].Description = lines[i];
            readDescription = false;
            continue;
        }
        if (lines[i].substr(0,6) == "Handle")
        {
            // Start new Frame
            Frame f;
            f.Handle = lines[i];
            frames_.push_back(f);
            currentIndex++;
            readDescription = true;
            readFeatureList = false;
            continue;
        }
        if (currentIndex < 0)
            continue;
        if (lines[i].find(":") != std::string::npos)
        {
            readFeatureList = false;
        }
        else if (readFeatureList)
        {
            frames_[currentIndex].FeatureData[currentFeature].push_back(lines[i]);
        }
        if (fields[i].size() == 2)
        {
            // Simple field
            readFeatureList = false;
            frames_[currentIndex].Data[fields[i][0]] = fields[i][1];
        }
        else if (fields[i].size() == 1)
        {
            // Possible Feature list type field
            boost::trim(fields[i][0]);
            currentFeature = fields[i][0];
            readFeatureList = true;
        }

    }
}
void DMIParser::setCurrentFrame(int frame)
{
    currentFrame_ = frame;
}
int DMIParser::currentFrame()
{
    return currentFrame_;
}
std::string DMIParser::operator[](std::string index)
{
    if ((currentFrame_ < 0) || (currentFrame_ > ((int)frames_.size()-1)))
        return "";

    if (frames_[currentFrame_].Data.find(index) != frames_[currentFrame_].Data.end())
    {
        return frames_[currentFrame_].Data[index];
    }
    if (frames_[currentFrame_].FeatureData.find(index) != frames_[currentFrame_].FeatureData.end())
    {
        std::string s;
        std::vector<std::string>::iterator i = frames_[currentFrame_].FeatureData[index].begin();
        while (i != frames_[currentFrame_].FeatureData[index].end())
        {
            s = (*i) + "\n";
            ++i;
        }
        return s;
    }

    return "";

}
int DMIParser::frameCount()
{
    return frames_.size();
}

}

NS_IZENELIB_UTIL_END
