#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <types.h>
#include <assert.h>

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
  static bool doit(uint32_t size=1000000000, uint32_t maxChars=80000, const std::string& filename="./input")
  {
    FILE* f = fopen (filename.c_str(), "w+");
    if (f == NULL)
    {
      std::cout<<"Can't open the output file! Please check the file name: "<<filename<<std::endl;
      return 0;
      
    }
    
    for (unsigned long i = 0; i<size; i++)
    {
      uint64_t charCount = rand()%maxChars;

      assert(fwrite(&charCount, sizeof(uint64_t),1, f)==1);
    }

    fclose(f);
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
