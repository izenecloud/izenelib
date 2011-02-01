#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ostream>

#include <am/graph_index/dyn_array.hpp>

using namespace izenelib::am;
/**
 *This is a static function to generate random query into 'filename' file.
 *@param size It indicate how many datas you want to generate. Default value is 100.
 *@param maxChars Maximum charactors of one words.Default value is 100.
 *@param filename The output file name. Default value is './input'.
 *
 **/
void  doit(uint32_t total = 100,uint32_t type_num = 10, 
           uint32_t min_len=10, uint32_t max_len=500,
           const std::string& filename="./input")
{
  typedef DynArray<uint32_t> array_t;
  FILE* f = fopen(filename.c_str(), "w+");
  IASSERT(fwrite(&total, sizeof (uint32_t), 1, f)==1);

  for (uint32_t i=0; i<type_num; ++i)
  {
    array_t arr;
    uint32_t len = rand()%max_len + min_len;
    for (uint32_t j=0; j<len; ++j)
      arr.push_back(rand());

    arr.save(f);
    for (uint32_t j=0; j<total/type_num; ++j)
    {
      array_t ar = arr;
      ar[rand()%arr.length()] = rand();
      ar.save(f);
    }
  }
    
}


  
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
    doit();
  }
  
  else
  {
    if (maxChars==0)
      doit(size);
    else
      doit(size, maxChars);
    
  }
  
}


