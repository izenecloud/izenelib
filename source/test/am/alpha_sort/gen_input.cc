#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ostream>
/** * @class get_input gen_input.cc
 *  @brief Genarate random words with random k for Edit-Distance. 
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
  static bool doit(size_t size=1000000, unsigned short maxChars=100, const std::string& filename="./input")
  {
    FILE* f = fopen(filename.c_str(), "w+");
    unsigned short len = rand()%maxChars;
    while (len<5)len = rand()%maxChars;

    fwrite(&size, sizeof(size_t), 1, f);

    while (size>0)
    {
      fwrite(&len, sizeof(unsigned short), 1, f);
      unsigned int key = rand();
      char r[len];
      *(unsigned int*)r = key;
      for (int i=sizeof(unsigned int); i<len-1;i++)
        r[i] = 'a'+rand()%26;
      r[len-1] = '\0';
      
      fwrite(r, len, 1, f);

      //std::cout<<"["<<len<<","<<key<<","<<r+sizeof(unsigned int)<<"]\n";
      
      size--;
      len = rand()%maxChars;
      while (len<5)len = rand()%maxChars;
    }

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


