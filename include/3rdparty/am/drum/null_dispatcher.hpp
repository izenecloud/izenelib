/*****************************************************************************
 The MIT License

 Copyright (c) 2009 Leandro T. C. Melo

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef IZENELIB_DRUM_NULL_DISPATCHER_HPP
#define IZENELIB_DRUM_NULL_DISPATCHER_HPP

#include "config.hpp"


DRUM_BEGIN_NAMESPACE

template <class key_t,
          class value_t,
          class aux_t>
class NullDispatcher
{
public:
    void UniqueKeyCheck(key_t const&, aux_t const&) const {}
    void DuplicateKeyCheck(key_t const&, value_t const&, aux_t const&) const {}

    void Update(key_t const&, value_t const&, aux_t const&) const {}
    void UniqueKeyUpdate(key_t const&, value_t const&, aux_t const&) const {}
    void DuplicateKeyUpdate(key_t const&, value_t const&, aux_t const&) const {}

    void Delete(key_t const&, aux_t const&) const {}
    void UniqueKeyDelete(key_t const&, aux_t const&) const {}
    void DuplicateKeyDelete(key_t const&, value_t const&, aux_t const&) const {}

    void UniqueKeyAppend(key_t const&, value_t const&, aux_t const&) const {}
    void DuplicateKeyAppend(key_t const&, value_t const&, aux_t const&) const {}

    void UniqueKeyExpel(key_t const&, value_t const&, aux_t const&) const {}
    void DuplicateKeyExpel(key_t const&, value_t const&, aux_t const&) const {}
};

DRUM_END_NAMESPACE

#endif //IZENELIB_DRUM_NULL_DISPATCHER_HPP
