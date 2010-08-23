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

#include "s9_coding.h"
using namespace std;

s9_coding::s9_coding() {
  csize[0] = 1;
  csize[1] = 2;
  csize[2] = 3;
  csize[3] = 4;
  csize[4] = 5;
  csize[5] = 7;
  csize[6] = 9;
  csize[7] = 14;
  csize[8] = 28;

  conum[0] = 28;
  conum[1] = 14;
  conum[2] = 9;
  conum[3] = 7;
  conum[4] = 5;
  conum[5] = 4;
  conum[6] = 3;
  conum[7] = 2;
  conum[8] = 1;

  coding_type = 4;
}

int s9_coding::get_type() {
  return coding_type;
}

void s9_coding::set_size(int size) {
}

int s9_coding::Compression(unsigned int* input, unsigned int* output, int size) {
  int left = size;
  int ret;
  unsigned* tmp = output;
  while (left > 0) {
    ret = s9_encode(tmp, input, left);
    input += ret;
    left -= ret;
    tmp++;
  }

  return tmp - output;
}

int s9_coding::s9_encode(unsigned int* _w, unsigned int* _p, int M) {
  int _j;
  int _m;
  for (int _k = 0; _k < 9; _k++) {
    *_w = _k << 28;
    _m = (conum[_k] < (M)) ? conum[_k] : (M);
    for (_j = 0; (_j < _m) && (*(_p + _j) < static_cast<unsigned> (1 << csize[_k])); _j++) {
      *_w |= (*(_p + _j)) << (csize[_k] * _j);
    }

    if (_j == _m) {
      _p += _m;
      _w++;
      break;
    }
  }

  return _m;
}

int s9_coding::Decompression(unsigned int* input, unsigned int* output, int size) {
  _p = output;
  int num;
  unsigned* tmp = input;

  int left = size;
  while (left > 0) {
    num = s9_decode(tmp);
    tmp++;
    left -= num;
  }

  return tmp - input;
}

int s9_coding::s9_decode(unsigned int* _w) {
  int _k;
  _k = (*_w) >> 28;
  switch (_k) {
    case 0:
      *_p = (*_w) & 1;
      _p++;
      *_p = (*_w >> 1) & 1;
      _p++;
      *_p = (*_w >> 2) & 1;
      _p++;
      *_p = (*_w >> 3) & 1;
      _p++;
      *_p = (*_w >> 4) & 1;
      _p++;
      *_p = (*_w >> 5) & 1;
      _p++;
      *_p = (*_w >> 6) & 1;
      _p++;
      *_p = (*_w >> 7) & 1;
      _p++;
      *_p = (*_w >> 8) & 1;
      _p++;
      *_p = (*_w >> 9) & 1;
      _p++;
      *_p = (*_w >> 10) & 1;
      _p++;
      *_p = (*_w >> 11) & 1;
      _p++;
      *_p = (*_w >> 12) & 1;
      _p++;
      *_p = (*_w >> 13) & 1;
      _p++;
      *_p = (*_w >> 14) & 1;
      _p++;
      *_p = (*_w >> 15) & 1;
      _p++;
      *_p = (*_w >> 16) & 1;
      _p++;
      *_p = (*_w >> 17) & 1;
      _p++;
      *_p = (*_w >> 18) & 1;
      _p++;
      *_p = (*_w >> 19) & 1;
      _p++;
      *_p = (*_w >> 20) & 1;
      _p++;
      *_p = (*_w >> 21) & 1;
      _p++;
      *_p = (*_w >> 22) & 1;
      _p++;
      *_p = (*_w >> 23) & 1;
      _p++;
      *_p = (*_w >> 24) & 1;
      _p++;
      *_p = (*_w >> 25) & 1;
      _p++;
      *_p = (*_w >> 26) & 1;
      _p++;
      *_p = (*_w >> 27) & 1;
      _p++;
      break;
    case 1:
      *_p = (*_w) & 3;
      _p++;
      *_p = (*_w >> 2) & 3;
      _p++;
      *_p = (*_w >> 4) & 3;
      _p++;
      *_p = (*_w >> 6) & 3;
      _p++;
      *_p = (*_w >> 8) & 3;
      _p++;
      *_p = (*_w >> 10) & 3;
      _p++;
      *_p = (*_w >> 12) & 3;
      _p++;
      *_p = (*_w >> 14) & 3;
      _p++;
      *_p = (*_w >> 16) & 3;
      _p++;
      *_p = (*_w >> 18) & 3;
      _p++;
      *_p = (*_w >> 20) & 3;
      _p++;
      *_p = (*_w >> 22) & 3;
      _p++;
      *_p = (*_w >> 24) & 3;
      _p++;
      *_p = (*_w >> 26) & 3;
      _p++;
      break;
    case 2:
      *_p = (*_w) & 7;
      _p++;
      *_p = (*_w >> 3) & 7;
      _p++;
      *_p = (*_w >> 6) & 7;
      _p++;
      *_p = (*_w >> 9) & 7;
      _p++;
      *_p = (*_w >> 12) & 7;
      _p++;
      *_p = (*_w >> 15) & 7;
      _p++;
      *_p = (*_w >> 18) & 7;
      _p++;
      *_p = (*_w >> 21) & 7;
      _p++;
      *_p = (*_w >> 24) & 7;
      _p++;
      break;
    case 3:
      *_p = (*_w) & 15;
      _p++;
      *_p = (*_w >> 4) & 15;
      _p++;
      *_p = (*_w >> 8) & 15;
      _p++;
      *_p = (*_w >> 12) & 15;
      _p++;
      *_p = (*_w >> 16) & 15;
      _p++;
      *_p = (*_w >> 20) & 15;
      _p++;
      *_p = (*_w >> 24) & 15;
      _p++;
      break;
    case 4:
      *_p = (*_w) & 31;
      _p++;
      *_p = (*_w >> 5) & 31;
      _p++;
      *_p = (*_w >> 10) & 31;
      _p++;
      *_p = (*_w >> 15) & 31;
      _p++;
      *_p = (*_w >> 20) & 31;
      _p++;
      break;
    case 5:
      *_p = (*_w) & 127;
      _p++;
      *_p = (*_w >> 7) & 127;
      _p++;
      *_p = (*_w >> 14) & 127;
      _p++;
      *_p = (*_w >> 21) & 127;
      _p++;
      break;
    case 6:
      *_p = (*_w) & 511;
      _p++;
      *_p = (*_w >> 9) & 511;
      _p++;
      *_p = (*_w >> 18) & 511;
      _p++;
      break;
    case 7:
      *_p = (*_w) & 16383;
      _p++;
      *_p = (*_w >> 14) & 16383;
      _p++;
      break;
    case 8:
      *_p = (*_w) & ((1 << 28) - 1);
      _p++;
      break;
  }

  return conum[_k];
}
