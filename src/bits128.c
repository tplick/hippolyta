// bits128.c: operations on 128-bit bitfields.
// Copyright 2009, 2010 Tom Plick (tomplick "AT" gmail.com).
// This code is released under the GPL version 3; see file LICENSE for info.

#include <stdint.h>

extern char popcounts[], lowbits[];

int POPCOUNT(uint64_t x)
{
    int i, t = 0;
    for (i = 0; i < 4; i++){
        t += popcounts[x & 65535];
        x >>= 16;
    }
    return t;
}

int LOWBIT(uint64_t x)
{
    int i = 0;
    while (!(x & 65535)){
        i += 16;
        x >>= 16;
    }
    return i + lowbits[x & 65535];
}


typedef struct {
    uint64_t low, high;
} int128;

int128 Zero, SingleBit[128];

int128 not128(int128 x)
{
    int128 y = {~x.low, ~x.high};
    return y;
}

int128 and128(int128 x, int128 y)
{
    int128 z = {x.low & y.low, x.high & y.high};
    return z;
}

int128 or128(int128 x, int128 y)
{
    int128 z = {x.low | y.low, x.high | y.high};
    return z;
}

int128 xor128(int128 x, int128 y)
{
    int128 z = {x.low ^ y.low, x.high ^ y.high};
    return z;
}

bool equal128(int128 x, int128 y)
{
    return x.low == y.low && x.high == y.high;
}

int count128(int128 x)
{
    return POPCOUNT(x.low) + POPCOUNT(x.high);
}

bool zero128(int128 x)
{
    return x.low == 0ULL && x.high == 0ULL;
}

int bitSet128(int128 x, int bit)
{
    if (bit < 64){
        return (x.low & (1ULL << bit)) != 0ULL;
    } else {
        return (x.high & (1ULL << (bit - 64))) != 0ULL;
    }
}

int128 shiftUp(int128 x, int d)
{
    int128 z = {x.low << d, x.high << d};
    z.high |= (x.low >> (64 - d));
    return z;
}

int128 shiftDown(int128 x, int d)
{
    int128 z = {x.low >> d, x.high >> d};
    z.low |= (x.high << (64 - d));
    return z;
}

int lowestBit128(int128 x)
{
    if (x.low)
        return LOWBIT(x.low);
    else
        return LOWBIT(x.high) + 64;
}


int128 fieldFromBits(int bits[], int n)
{
    int128 field = {0ULL, 0ULL};
    int i;

    for (i = 0; i < n; i++){
        field = or128(field, SingleBit[bits[i]]);
    }

    return field;
}

