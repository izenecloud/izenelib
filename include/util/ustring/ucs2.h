/*
 * Copyright (C) 1999-2001 Free Software Foundation, Inc.
 * This file is part of the GNU LIBICONV Library.
 *
 * The GNU LIBICONV Library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * The GNU LIBICONV Library is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the GNU LIBICONV Library; see the file COPYING.LIB.
 * If not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * UCS-2
 */

/* Here we accept FFFE/FEFF marks as endianness indicators everywhere
   in the stream, not just at the beginning. The default is big-endian. */
/* The state is 0 if big-endian, 1 if little-endian. */

#ifndef _UCS2_H_
#define _UCS2_H_


static int
ucs2_mbtowc (ucs4_t *pwc, const ucs2_t *s, int n)
{
  ucs4_t wc = *s;
  if (wc >= 0xd800 && wc < 0xe000) {
    return RET_ILSEQ;
  } else {
    *pwc = wc;
    return 1;
  }
  return RET_TOOFEW(0);
}

/* But we output UCS-2 in big-endian order, without byte-order mark. */
/* RFC 2152 says:
   "ISO/IEC 10646-1:1993(E) specifies that when characters the UCS-2 form are
    serialized as octets, that the most significant octet appear first." */
static int
ucs2_wctomb (ucs2_t *r, ucs4_t wc, int n)
{
  if (wc < 0x10000 && wc != 0xfffe && !(wc >= 0xd800 && wc < 0xe000)) {
    if (n >= 1) {
    //if (n >= 2) {
      *r = (ucs2_t) wc;
      //r[0] = (unsigned char) (wc >> 8);
      //r[1] = (unsigned char) wc;
      return 1;
      //return 2;
    } else
      return RET_TOOSMALL;
  } else
    return RET_ILUNI;
}


#endif
