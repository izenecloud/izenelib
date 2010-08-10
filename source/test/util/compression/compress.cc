#include "Compressor.h"
#include  <ctime>

using namespace std;
using namespace izenelib::util::compression;


int main (int argc, char *argv[])
{
  try
  {
    /* check arguments */
    if (argc < 3)
    {
      CompressorException ex ("syntax: compress <input file> <output file>\n");
      throw ex;
    }

    clock_t start,end;
    /* init a compressor and do compression */
    Compressor c;
    string input_filename (argv[1]);
    string output_filename (argv[2]);
    c.displayPattern();
    start = clock();
    c.compressFile (input_filename, output_filename);
    end = clock();
    cout << "\tcompressed elapsed: " << (end - start) * 1.0 / CLOCKS_PER_SEC << " seconds\n";

    //c.decompressFile (output_filename, "dbg.out");
    
   // c.showCode(input_filename, "showcode.out");

    return 0;
  }

  catch (CompressorException ex)
  {
    cerr << ex.get_reason();
    return 1;
  }

  catch (...)
  {
    return 1;
  }
}
