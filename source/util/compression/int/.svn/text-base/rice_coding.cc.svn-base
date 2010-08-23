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

#include "rice_coding.h"
using namespace std;

rice_coding::rice_coding() {
  coding_type = 0;
}

int rice_coding::get_type() {
  return coding_type;
}

void rice_coding::set_size(int size) {
  block_size = size;
}

int rice_coding::Compression(unsigned int* input, unsigned int* output, int size) {
  uint64_t avg = 0;
  // Find the average of the numbers we're encoding.
  for (int i = 0; i < size; ++i) {
    avg += input[i];
  }
  avg /= size;
  // Find the closest power of 2 to the average.
  b = 0;
  while ((avg >>= 1) > 0) {
    ++b;
  }

  unsigned int* aux1 = output;
  *aux1 = (unsigned int) b;
  aux1++;

  unsigned int bp = 0;
  for (int i = 0; i < size; ++i) {
    rice_encode(aux1, &bp, input[i], b);
  }

  int s;
  if ((bp & 0x1f) != 0)
    s = (1 + (bp >> 5));
  else
    s = (bp >> 5);
  s += 1;
  return s;
}

void rice_coding::rice_encode(unsigned int* buf, unsigned int* bp, unsigned int val, unsigned int bits) {
  unsigned int w;
  writeBits(buf, bp, val & MASK[bits], bits);
  for (w = (val >> bits); w > 0; --w) {
    writeBits(buf, bp, 1, 1);
  }
  writeBits(buf, bp, 0, 1);
}

int rice_coding::Decompression(unsigned int* input, unsigned int* output, int size) {
  int i;
  unsigned int bp;
  unsigned int* tmp = input;
  b = *tmp;
  tmp++;
  for (i = 0, bp = 0; i < size; ++i) {
    output[i] = rice_decode(tmp, &bp, b);
  }

  int s;
  if (bp & 0x1f)
    s = (1 + (bp >> 5));
  else
    s = (bp >> 5);
  s += 1;
  return s;
}

unsigned int rice_coding::rice_decode(unsigned int* buf, unsigned int* bp, unsigned int bits) {
  unsigned int v;
  v = readBits(buf, bp, bits);
  while (TESTBIT(buf, *bp)) {
    v += (1 << bits);
    (*bp)++;
  }

  (*bp)++;
  return v;
}
