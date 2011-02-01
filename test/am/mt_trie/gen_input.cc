#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <am/linear_hash_table/linearHashTable.hpp>
#include <util/ustring/UString.h>

#define ENCODE_TYPE UString::UTF_8//EUC_KR//GB2312//

using namespace izenelib::util;
using namespace std;
using namespace izenelib::am;

const int en_size = 26;
char en[en_size] = {97,	98,	99,	100,	101,	102,	103,
104,	105,	106,	107,	108,	109,	110,	111,
112,	113,	114,	115,	116,	117,	118,	119,
120,	121,	122};


/** * @class get_input gen_input.cc
 *  @brief Genarate random words with random k for sEdit-Distance.
 *
 **/
template<
  class STRING_TYPE
  >
class get_input
{
  static void setEncoding(std::string& str) {
  }

  static void setEncoding(izenelib::util::UString& str) {
	str.setSystemEncodingType(ENCODE_TYPE);
  }
public:
  /**
   *This is a static function to generate random query into 'filename' file.
   *@param size It indicate how many datas you want to generate. Default value is 100.
   *@param maxChars Maximum charactors of one words.Default value is 100.
   *@param filename The output file name. Default value is './input'.
   *
   **/
  static bool doit(const std::string& filename, unsigned long size=1000000, unsigned long maxChars=20)
  {
    std::ofstream of;
    of.open (filename.c_str(), std::ofstream::out|std::ofstream::binary);
    if (of.fail())
    {
      std::cout<<"Can't open the output file! Please check the file name: "<<filename<<std::endl;
      return 0;
    }

    for (unsigned long i = 0; i<size; i++)
    {
      STRING_TYPE str;
      setEncoding(str);
      unsigned long charCount = rand()%maxChars;
      while (charCount == 0)
        charCount = rand()%maxChars;

      for (unsigned long j=0; j<charCount; j++)
      {
        str += en[rand()%en_size];
      }

      int size = str.size();
      of.write((char*)&size, sizeof(int));
      of.write((char*)str.c_str(), size);

    }
    of.close();

    return true;

  }

};


int main (int argc,char **argv)
{
  if (argc ==1)
  {
    std::cout<<"\nUsage: ./gen-input  <size> <max chars of one word>\n";
    std::cout<<"\nDefault:\n <size>: 100 \n  <max chars of one word>:100" << std::endl
	<< "The output file name is './input-string and ./input-ustring\n";
    return -1;
  }
  *argv++;

  std::stringstream ss(std::stringstream::in | std::stringstream::out);
  ss<<*argv;

  unsigned long size=0;
  unsigned long maxChars = 0;
  argv++;

  ss>>size;
  ss.clear();
  ss <<*argv;
  ss >> maxChars;

  if (size ==0)
  {
    get_input<std::string>::doit("input-string");
    get_input<izenelib::util::UString>::doit("input-ustring");
  }

  else
  {
    if (maxChars==0) {
      get_input<std::string>::doit("input-string", size);
      get_input<izenelib::util::UString>::doit("input-ustring", size);
    } else {
      get_input<std::string>::doit("input-string", size, maxChars);
      get_input<izenelib::util::UString>::doit("input-ustring", size, maxChars);
    }

  }
}
