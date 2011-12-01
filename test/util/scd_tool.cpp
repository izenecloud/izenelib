#include <util/scd_parser.h>

#include <cstdlib>
#include <iostream>
#include <fstream>

using namespace izenelib::util;

void extract(const std::string& scdFile, const std::string& outputFile, const std::string& property);

size_t gMaxDoc = 0;

int main(int argc, char** argv)
{
    std::string inputScdFile;
    std::string outputFile;
    std::string command;
    std::string property;


    bool help = false;
    char optchar;
    while ((optchar = getopt(argc, argv, "hi:o:c:p:m:")) != -1)
    {
        switch (optchar) {
            case 'i':
                inputScdFile = optarg;
                break;
            case 'o':
                outputFile = optarg;
                break;
            case 'c':
                command = optarg;
                break;
            case 'p':
                property = optarg;
                break;
            case 'm':
                try {
                    gMaxDoc = atoi(optarg);
                } catch(std::exception&e) {}
                break;
            case 'h':
                help = true;
                break;
            default:
                std::cout << "Unrecognized flag " << optchar << std::endl;
                help = true;
                break;
        }
    }

    if (argc < 2 || help)
    {
        std::cout<<"Usage: "<<argv[0]<<" -i <input scd file> -o <ouput file> -c <command> [-m <maxdoc> -p <property>] " << std::endl;
        std::cout<<"    command: extract -p     extract property value"<< std::endl;
        return 0;
    }

    if (command == "extract")
    {
        if (property.empty())
        {
            std::cout<<"-p <property> needed for extract"<<std::endl;
            return 0;
        }

        extract(inputScdFile, outputFile, property);
    }


    return 0;
}

void extract(const std::string& scdFile, const std::string& outputFile, const std::string& property)
{
    ScdParser scdParser(izenelib::util::UString::UTF_8);
    if(!scdParser.load(scdFile.c_str()))
    {
        std::cerr<<"Failed to open scd file: "<<scdFile<<std::endl;
        return;
    }

    izenelib::util::UString uproperty = izenelib::util::UString(property, izenelib::util::UString::UTF_8);

    std::ofstream ofs;
    ofs.open(outputFile.c_str());
    if (!ofs.is_open())
    {
        std::cerr<<"Failed to open ouput file: "<<outputFile<<std::endl;
        return;
    }

    size_t docCount = 0;
    std::string tmp;
    for (ScdParser::iterator it = scdParser.begin(); it != scdParser.end(); it++)
    {
        SCDDocPtr pScdDoc = *it;
        for (SCDDoc::iterator propIt = pScdDoc->begin(); propIt != pScdDoc->end(); propIt++)
        {
            izenelib::util::UString& propName = propIt->first;
            if (propName == uproperty)
            {
                propIt->second.convertString(tmp, izenelib::util::UString::UTF_8);
                ofs << tmp <<std::endl;
            }
        }

        std::cout<<"\rprocessed documents "<<(++docCount)<<std::flush;

        if (gMaxDoc > 0 && docCount >= gMaxDoc)
            break;
    }
    std::cout<<std::endl;

    ofs.close();
}
