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

#include "s16_coding.h"
using namespace std;

const unsigned int s16_coding::cbits[16][28] = { {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                                                 {2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
                                                 {1,1,1,1,1,1,1,2,2,2,2,2,2,2,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
                                                 {1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,0,0,0,0,0,0,0},
                                                 {2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                                 {4,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                                 {3,4,4,4,4,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                                 {4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                                 {5,5,5,5,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                                 {4,4,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                                 {6,6,6,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                                 {5,5,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                                 {7,7,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                                 {10,9,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                                 {14,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                                 {28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} };

s16_coding::s16_coding() {
  cnum[0] = 28;
  cnum[1] = 21;
  cnum[2] = 21;
  cnum[3] = 21;
  cnum[4] = 14;
  cnum[5] = 9;
  cnum[6] = 8;
  cnum[7] = 7;
  cnum[8] = 6;
  cnum[9] = 6;
  cnum[10] = 5;
  cnum[11] = 5;
  cnum[12] = 4;
  cnum[13] = 3;
  cnum[14] = 2;
  cnum[15] = 1;

  coding_type = 5;
}

int s16_coding::get_type() {
  return coding_type;
}

void s16_coding::set_size(int size) {
}

int s16_coding::Compression(unsigned int* input, unsigned int* output, int size) {
  int left = size;
  unsigned int* tmp = output;
  int ret;
  while (left > 0) {
    ret = s16_encode(tmp, input, left);
    input += ret;
    left -= ret;
    tmp++;
  }

  return tmp - output;
}

int s16_coding::s16_encode(unsigned int* _w, unsigned int* _p, unsigned int m) {
  unsigned int _k, _j, _m, _o;

  for (_k = 0; _k < 16; _k++) {
    (*_w) = _k << 28;
    _m = (cnum[_k] < m) ? cnum[_k] : m;

    for (_j = 0, _o = 0; (_j < _m) && (*(_p + _j) < static_cast<unsigned int> (1 << cbits[_k][_j]));) {
      (*_w) += ((*(_p + _j)) << _o);
      _o += cbits[_k][_j];
      _j++;
    }
    if (_j == _m) {
      (_p) += _m;
      (_w)++;
      break;
    }
  }

  return _m;
}

int s16_coding::Decompression(unsigned int* input, unsigned int* output, int size) {
  _p = output;
  int num;
  unsigned int* tmp = input;

  int left = size;
  while (left > 0) {
    num = s16_decode(tmp);
    tmp++;
    left -= num;
  }

  return tmp - input;
}

int s16_coding::s16_decode(unsigned int* _w) {
  int _k = (*_w) >> 28;
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
    case 2:
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
      *_p = (*_w >> 7) & 3;
      _p++;
      *_p = (*_w >> 9) & 3;
      _p++;
      *_p = (*_w >> 11) & 3;
      _p++;
      *_p = (*_w >> 13) & 3;
      _p++;
      *_p = (*_w >> 15) & 3;
      _p++;
      *_p = (*_w >> 17) & 3;
      _p++;
      *_p = (*_w >> 19) & 3;
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
    case 3:
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
    case 4:
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
    case 5:
      *_p = (*_w) & 15;
      _p++;
      *_p = (*_w >> 4) & 7;
      _p++;
      *_p = (*_w >> 7) & 7;
      _p++;
      *_p = (*_w >> 10) & 7;
      _p++;
      *_p = (*_w >> 13) & 7;
      _p++;
      *_p = (*_w >> 16) & 7;
      _p++;
      *_p = (*_w >> 19) & 7;
      _p++;
      *_p = (*_w >> 22) & 7;
      _p++;
      *_p = (*_w >> 25) & 7;
      _p++;
      break;
    case 6:
      *_p = (*_w) & 7;
      _p++;
      *_p = (*_w >> 3) & 15;
      _p++;
      *_p = (*_w >> 7) & 15;
      _p++;
      *_p = (*_w >> 11) & 15;
      _p++;
      *_p = (*_w >> 15) & 15;
      _p++;
      *_p = (*_w >> 19) & 7;
      _p++;
      *_p = (*_w >> 22) & 7;
      _p++;
      *_p = (*_w >> 25) & 7;
      _p++;
      break;
    case 7:
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
    case 8:
      *_p = (*_w) & 31;
      _p++;
      *_p = (*_w >> 5) & 31;
      _p++;
      *_p = (*_w >> 10) & 31;
      _p++;
      *_p = (*_w >> 15) & 31;
      _p++;
      *_p = (*_w >> 20) & 15;
      _p++;
      *_p = (*_w >> 24) & 15;
      _p++;
      break;
    case 9:
      *_p = (*_w) & 15;
      _p++;
      *_p = (*_w >> 4) & 15;
      _p++;
      *_p = (*_w >> 8) & 31;
      _p++;
      *_p = (*_w >> 13) & 31;
      _p++;
      *_p = (*_w >> 18) & 31;
      _p++;
      *_p = (*_w >> 23) & 31;
      _p++;
      break;
    case 10:
      *_p = (*_w) & 63;
      _p++;
      *_p = (*_w >> 6) & 63;
      _p++;
      *_p = (*_w >> 12) & 63;
      _p++;
      *_p = (*_w >> 18) & 31;
      _p++;
      *_p = (*_w >> 23) & 31;
      _p++;
      break;
    case 11:
      *_p = (*_w) & 31;
      _p++;
      *_p = (*_w >> 5) & 31;
      _p++;
      *_p = (*_w >> 10) & 63;
      _p++;
      *_p = (*_w >> 16) & 63;
      _p++;
      *_p = (*_w >> 22) & 63;
      _p++;
      break;
    case 12:
      *_p = (*_w) & 127;
      _p++;
      *_p = (*_w >> 7) & 127;
      _p++;
      *_p = (*_w >> 14) & 127;
      _p++;
      *_p = (*_w >> 21) & 127;
      _p++;
      break;
    case 13:
      *_p = (*_w) & 1023;
      _p++;
      *_p = (*_w >> 10) & 511;
      _p++;
      *_p = (*_w >> 19) & 511;
      _p++;
      break;
    case 14:
      *_p = (*_w) & 16383;
      _p++;
      *_p = (*_w >> 14) & 16383;
      _p++;
      break;
    case 15:
      *_p = (*_w) & ((1 << 28) - 1);
      _p++;
      break;
  }
  return cnum[_k];
}
