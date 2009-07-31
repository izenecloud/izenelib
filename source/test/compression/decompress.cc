/*
 * decompress.cc
 *
 * charles wang
 */

#include "Compressor.h"
#include  <ctime>

using namespace std;
using namespace ylib;

/** \file decompress.cc
 * \brief Command line driver for order preserving decompressor.
 *
 * Command line syntax: decompress <input filename> <output filename>
 *
 */

int main (int argc, char *argv[])
{
  try
  {
    /* check arguments */
    if (argc < 3)
    {
        CompressorException ex ("syntax: decompress <input file> <output file>\n");
        throw ex;
    }
    clock_t start,end;
    
    /* init a compressor to decompress the file */
    Compressor c;
    string input_filename (argv[1]);
    string output_filename (argv[2]);
    start = clock();
    c.decompressFile(input_filename, output_filename);
    end = clock();
      cout << "\tdecompress elapsed: " << (end - start) * 1.0 / CLOCKS_PER_SEC << " seconds\n";
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
