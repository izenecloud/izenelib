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

#include "vbyte_coding.h"
using namespace std;

vbyte_coding::vbyte_coding() {
  coding_type = 2;
}

void vbyte_coding::set_size(int size) {
}

int vbyte_coding::get_type() {
  return coding_type;
}

int vbyte_coding::Compression(unsigned int* input, unsigned int* output, int size) {
  unsigned char* curr_byte = reinterpret_cast<unsigned char*> (output);

  unsigned int bp = 0;  // Current byte pointer into the 'output' array which we use to set each word in the 'output' array to 0 before decoding to it.
                        // This which prevents uninitialized data errors in Valgrind.
  unsigned int n;
  for (int i = 0; i < size; ++i) {
    n = input[i];
    assert(n <= 0x10204080);  // Max integer we can encode using 5 bytes is ((128**4)+(128**3)+(128**2)+(128**1)).
    unsigned char _barray[5];
    for (int i = 0; i < 5; ++i) {
      _barray[i] = (n & 0x7F) << 1;
      n = n >> 7;
    }

    bool started = false;
    for (int i = 4; i > 0; --i) {
      if (_barray[i] != 0 || started == true) {
        started = true;
        if ((bp & 3) == 0)
          output[bp >> 2] = 0;
        *curr_byte = _barray[i] | 0x1;
        ++curr_byte;
        ++bp;
      }
    }

    if ((bp & 3) == 0)
      output[bp >> 2] = 0;
    *curr_byte = _barray[0];
    ++curr_byte;
    ++bp;
  }

  return (bp >> 2) + ((bp & 3) != 0 ? 1 : 0);
}

int vbyte_coding::Decompression(unsigned int* input, unsigned int* output, int size) {
  unsigned char* curr_byte = reinterpret_cast<unsigned char*> (input);
  unsigned int n;
  for (int i = 0; i < size; ++i) {
    n = ((*curr_byte >> 1));
    if ((*curr_byte & 0x1) != 0) {
      ++curr_byte;
      n = (n << 7) | (*curr_byte >> 1);
      if ((*curr_byte & 0x1) != 0) {
        ++curr_byte;
        n = (n << 7) | (*curr_byte >> 1);
        if ((*curr_byte & 0x1) != 0) {
          ++curr_byte;
          n = (n << 7) | (*curr_byte >> 1);
        }
      }
    }
    ++curr_byte;
    output[i] = n;
  }

  int num_bytes_consumed = (curr_byte - reinterpret_cast<unsigned char*> (input));
  return (num_bytes_consumed >> 2) + ((num_bytes_consumed & 3) != 0 ? 1 : 0);
}
