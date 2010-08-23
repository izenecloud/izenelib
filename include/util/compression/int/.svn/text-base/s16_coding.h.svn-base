// Copyright (c) 2008, WEST, Polytechnic Institute of NYU
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of WEST, Polytechnic Institute of NYU nor the names
//     of its contributors may be used to endorse or promote products derived
//     from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author(s): Torsten Suel, Jiangong Zhang, Jinru He
//
// If you have any questions or problems with our code, please contact:
// jhe@cis.poly.edu
//
// S16 Coding:
// In each block, we do not need to pad the last word with zero entries,
// because the coding method knows the block size and in the implementation,
// the decompression method will know where to stop.
// Each encoded word has a 4-bit header indicating which case was used to
// encode it. Thus, this limits the maximum integer that can be encoded to
// (2**28)-1.

#ifndef S16_CODING_H_
#define S16_CODING_H_

#include "coding.h"

class s16_coding : public coding {
public:
  s16_coding();
  int Compression(unsigned int* input, unsigned int* output, int size);
  int Decompression(unsigned int* input, unsigned int* output, int size);
  int get_type();
  void set_size(int size);
private:
  int s16_encode(unsigned int* _w, unsigned int* _p, unsigned int m);
  int s16_decode(unsigned int* _w);

  unsigned int* _p;
  int coding_type;
  uint32_t cnum[16];

  static const unsigned int cbits[16][28];
};

#endif /* S16_CODING_H_ */
