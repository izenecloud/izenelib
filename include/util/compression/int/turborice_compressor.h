#ifndef TURBORICE_COMPRESSOR_H_
#define TURBORICE_COMPRESSOR_H_

#include "coding.h"

namespace izenelib {
namespace util{
namespace compression {

class turborice_compressor : public coding {
public:
  turborice_compressor();
  int _compress(unsigned int* input, unsigned int* output, int size);
  int _decompress(unsigned int* input, unsigned int* output, int size);
private:
  int _compress_block(unsigned int* input, unsigned int* output, int size);
  int _decompress_block(unsigned int* input, unsigned int* output, int size);
  void turbo_rice_encode(unsigned int** w, unsigned int** buf, unsigned int bits);
  void turbo_rice_decode(unsigned int** w, unsigned int* buf, unsigned int bits, unsigned int flag);

  int b;
  int block_size;
  int cnum[17];
};

}}}
#endif /* turborice_coompressor_H_ */
