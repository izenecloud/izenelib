/**
 * This is code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 * (c) Daniel Lemire, http://lemire.me/en/
 */
#ifndef BITPACKINGUNALIGNED
#define BITPACKINGUNALIGNED
#include <string.h>
#include <types.h>
#include <stdexcept>

using namespace std;
typedef uint8_t byte;

const uint8_t * fastunalignedunpack_8(const uint8_t *  __restrict__ in, uint32_t *  __restrict__  out, const uint32_t bit) ;
uint8_t * fastunalignedpackwithoutmask_8(const uint32_t *  __restrict__ in, uint8_t *  __restrict__  out, const uint32_t bit) ;
const uint8_t * fastunaligneduint8_tunpack_8(const uint8_t *  __restrict__ in, uint8_t *  __restrict__  out, const uint32_t bit) ;
uint8_t * fastunaligneduint8_tpackwithoutmask_8(const uint8_t *  __restrict__ in, uint8_t *  __restrict__  out, const uint32_t bit) ;
const uint8_t * fastunalignedunpack_16(const uint8_t *  __restrict__ in, uint32_t *  __restrict__  out, const uint32_t bit) ;
uint8_t * fastunalignedpackwithoutmask_16(const uint32_t *  __restrict__ in, uint8_t *  __restrict__  out, const uint32_t bit) ;
const uint8_t * fastunaligneduint8_tunpack_16(const uint8_t *  __restrict__ in, uint8_t *  __restrict__  out, const uint32_t bit) ;
uint8_t * fastunaligneduint8_tpackwithoutmask_16(const uint8_t *  __restrict__ in, uint8_t *  __restrict__  out, const uint32_t bit) ;

#endif // BITPACKINGUNALIGNED
