#include "util/util.h"

/*
--------------------------------------------------------------------
lookup8.c, by Bob Jenkins, January 4 1997, Public Domain.
hash(), hash2(), hash3, and mix() are externally useful functions.
Routines to test the hash are included if SELF_TEST is defined.
You can use this free for any purpose.  It has no warranty.

2009: This is obsolete.  I recently timed lookup3.c as being faster
at producing 64-bit results.
--------------------------------------------------------------------
*/

typedef unsigned long long ub8; /* unsigned 8-byte quantities */
typedef unsigned long int ub4; /* unsigned 4-byte quantities */
typedef unsigned char ub1;

#define hashsize(n) ((ub8)1 << (n))
#define hashmask(n) (hashsize(n) - 1)

// clang-format off
#define mix64(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>43); \
  b -= c; b -= a; b ^= (a<<9); \
  c -= a; c -= b; c ^= (b>>8); \
  a -= b; a -= c; a ^= (c>>38); \
  b -= c; b -= a; b ^= (a<<23); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>35); \
  b -= c; b -= a; b ^= (a<<49); \
  c -= a; c -= b; c ^= (b>>11); \
  a -= b; a -= c; a ^= (c>>12); \
  b -= c; b -= a; b ^= (a<<18); \
  c -= a; c -= b; c ^= (b>>22); \
}
// clang-format on

static ub8 Hash64(const ub1* k, ub8 length, ub8 level)
{
	ub8 a, b, c, len;

	/* Set up the internal state */
	len = length;
	a = b = level; /* the previous hash value */
	c = 0x9e3779b97f4a7c13LL; /* the golden ratio; an arbitrary value */

	/*---------------------------------------- handle most of the key */
	while (len >= 24)
	{
		// clang-format off
        a += (k[0] + ((ub8)k[1] << 8) + ((ub8)k[2] << 16) + ((ub8)k[3] << 24)
              + ((ub8)k[4] << 32) + ((ub8)k[5] << 40) + ((ub8)k[6] << 48) + ((ub8)k[7] << 56));
        b += (k[8] + ((ub8)k[9] << 8) + ((ub8)k[10] << 16) + ((ub8)k[11] << 24)
              + ((ub8)k[12] << 32) + ((ub8)k[13] << 40) + ((ub8)k[14] << 48) + ((ub8)k[15] << 56));
        c += (k[16] + ((ub8)k[17] << 8) + ((ub8)k[18] << 16) + ((ub8)k[19] << 24)
              + ((ub8)k[20] << 32) + ((ub8)k[21] << 40) + ((ub8)k[22] << 48) + ((ub8)k[23] << 56));
		// clang-format on
		mix64(a, b, c);
		k += 24;
		len -= 24;
	}

	/*------------------------------------- handle the last 23 bytes */
	c += length;
	switch (len) /* all the case statements fall through */
	{
	case 23:
		c += ((ub8)k[22] << 56);
	case 22:
		c += ((ub8)k[21] << 48);
	case 21:
		c += ((ub8)k[20] << 40);
	case 20:
		c += ((ub8)k[19] << 32);
	case 19:
		c += ((ub8)k[18] << 24);
	case 18:
		c += ((ub8)k[17] << 16);
	case 17:
		c += ((ub8)k[16] << 8);
		/* the first byte of c is reserved for the length */
	case 16:
		b += ((ub8)k[15] << 56);
	case 15:
		b += ((ub8)k[14] << 48);
	case 14:
		b += ((ub8)k[13] << 40);
	case 13:
		b += ((ub8)k[12] << 32);
	case 12:
		b += ((ub8)k[11] << 24);
	case 11:
		b += ((ub8)k[10] << 16);
	case 10:
		b += ((ub8)k[9] << 8);
	case 9:
		b += ((ub8)k[8]);
	case 8:
		a += ((ub8)k[7] << 56);
	case 7:
		a += ((ub8)k[6] << 48);
	case 6:
		a += ((ub8)k[5] << 40);
	case 5:
		a += ((ub8)k[4] << 32);
	case 4:
		a += ((ub8)k[3] << 24);
	case 3:
		a += ((ub8)k[2] << 16);
	case 2:
		a += ((ub8)k[1] << 8);
	case 1:
		a += ((ub8)k[0]);
		/* case 0: nothing left to add */
	}
	mix64(a, b, c);
	/*-------------------------------------------- report the result */
	return c;
}

blt::idstring blt::idstring_hash(const std::string& text)
{
	return Hash64((const unsigned char*)text.c_str(), text.length(), 0);
}
