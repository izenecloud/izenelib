#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <am/ustr-btrie/alphabet.hpp>
#include <am/linear_hash_table/linearHashTable.hpp>
#include <ustring/UString.h>

#define ENCODE_TYPE UString::UTF_8//EUC_KR//GB2312//

using namespace sf1lib;
using namespace std;

/** * @class get_input gen_input.cc
 *  @brief Genarate random words with random k for sEdit-Distance. 
 * 
 **/
class get_input
{
public:
  /**
   *This is a static function to generate random query into 'filename' file.
   *@param size It indicate how many datas you want to generate. Default value is 100.
   *@param maxChars Maximum charactors of one words.Default value is 100.
   *@param filename The output file name. Default value is './input'.
   *
   **/
  static bool doit(unsigned long size=1000000, unsigned long maxChars=20, const std::string& filename="./input")
  {
    std::ofstream of;
    of.open (filename.c_str(), std::ofstream::out);
    if (of.fail())
    {
      std::cout<<"Can't open the output file! Please check the file name: "<<filename<<std::endl;
      return 0;
      
    }
    UString str("", ENCODE_TYPE);
    UString line("\n", ENCODE_TYPE);
    
    for (unsigned long i = 0; i<size; i++)
    {
      unsigned long charCount = rand()%maxChars;
      while (charCount == 0)
        charCount = rand()%maxChars;
      
      for (unsigned long j=0; j<charCount; j++)
      {
        str += a2z[rand()%a2z_size];
      }

      str += (UCS2Char)'\n';
      
    }

    str.displayStringValue(ENCODE_TYPE, of);
    
    //of.write(str.c_str(), str.size());
    of.close();

    return true;
    
  }
  
};

  
int main (int argc,char **argv)
{
  
  if (argc ==1)
  {
    std::cout<<"\nUsage: ./gen-input <size> <max chars of one word>\n";
    std::cout<<"\nDefault:\n    <size>: 100 \n    <max chars of one word>:100\nThe output file name is './input\n";
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
    get_input::doit();
  }
  
  else
  {
    if (maxChars==0)
      get_input::doit(size);
    else
      get_input::doit(size, maxChars);
    
  }
  
  
  
  
}
