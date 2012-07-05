#include <string>
#include <fstream>
#include <iostream>
#include <am/trie/alphabet.hpp>

using namespace izenelib::util;
using namespace std;

int main (int argc,char **argv)
{
    std::cout<<"Gen-alphabet" << std::endl;
    std::cout<<"The output file name is alphabet_en.h and alphabet_cjk.h" << std::endl;

    /** create alphabet_en.h */
    AlphabetGenerator enAlphabet("en");
    enAlphabet.addCharsFromString(std::string("abcdefghijklmnopqrstuvwxyz"), UString::UTF_8);

    std::ofstream enf;
    enf.open ("alphabet_en.h", std::ofstream::out);
    if (enf.fail())
    {
        std::cout<<"Can't open the output file alphabet_en.h!"<<std::endl;
        return 0;
    }
    enf << enAlphabet;
    enf.close();

    /** create alphabet_cjk.h */
    std::string cjkname("cjk");
    AlphabetGenerator cjkAlphabet(cjkname);
    cjkAlphabet.addCharsFromRange(0x0000, 0x007f);
    cjkAlphabet.addCharsFromRange(0x2e00, 0x9fff);
    cjkAlphabet.addCharsFromRange(0xac00, 0xd7af);
    cjkAlphabet.addCharsFromRange(0xf900, 0xfaff);
    cjkAlphabet.addCharsFromRange(0xfe30, 0xfe4f);

    std::ofstream cjkf;
    cjkf.open ("alphabet_cjk.h", std::ofstream::out);
    if (cjkf.fail())
    {
        std::cout<<"Can't open the output file alphabet_cjk.h!"<<std::endl;
        return 0;
    }
    cjkf << cjkAlphabet;
    cjkf.close();

}

