/**
 * This is code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 * (c) Daniel Lemire, http://lemire.me/en/
 */

#ifndef PACKINGVECTORS_H_
#define PACKINGVECTORS_H_

#include "common.h"
#include "bitpacking.h"

#define _PACKSIZE 32

template<class STLContainer>
const uint32_t * unpackme(const uint32_t * in, STLContainer & out,
                          const uint32_t bit)
{
    const uint32_t size = *in;
    ++in;
    /*if (size == 0) {
        out.resize(0);
        return in;
    }*/
    out.resize((size + _PACKSIZE - 1) / _PACKSIZE * _PACKSIZE);
    for (uint32_t j = 0; j != out.size(); j += _PACKSIZE)
    {
        fastunpack(in, &out[j], bit);
        in += bit;
    }
    out.resize(size);
    return in;
}

template<class STLContainer>
uint32_t * packmeup(STLContainer & source, uint32_t * out, const uint32_t bit)
{
    const uint32_t size = source.size();
    *out = size;
    out++;
    if (source.size() == 0)
        return out;
    source.resize((source.size() + _PACKSIZE - 1) / _PACKSIZE * _PACKSIZE);
    for (uint32_t j = 0; j != source.size(); j += _PACKSIZE)
    {
        fastpack(&source[j], out, bit);
        out += bit;
    }
    source.resize(size);
    return out;
}

template<class STLContainer>
uint32_t * packmeupwithoutmask(STLContainer & source, uint32_t * out,
                               const uint32_t bit)
{
    const uint32_t size = source.size();
    *out = size;
    out++;
    if (source.size() == 0)
        return out;
    source.resize((source.size() + _PACKSIZE - 1) / _PACKSIZE * _PACKSIZE);
    for (uint32_t j = 0; j != source.size(); j += _PACKSIZE)
    {
        fastpackwithoutmask(&source[j], out, bit);
        out += bit;
    }
    source.resize(size);
    return out;
}

#endif /* PACKINGVECTORS_H_ */
