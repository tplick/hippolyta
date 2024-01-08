// position.c
// Copyright 2009, 2010 Tom Plick (tomplick "AT" gmail.com).
// This code is released under the GPL version 3; see file LICENSE for info.

#include <stdint.h>
#include <string.h>

typedef struct {
    int8_t src, dst, target;
} Move;

typedef struct {
    int128 occ[2];
    int128 arrows;
    int8_t turn;
    Move move;
} Position;

int at(Position pos, int r, int c)
{
    int bit = 11*r + c;
    if (bitSet128(pos.occ[0], bit)) return 1;
    if (bitSet128(pos.occ[1], bit)) return 2;
    if (bitSet128(pos.arrows, bit)) return 3;
    return 0;
}

int max(int a, int b) { return (a >= b ? a : b); }

int areSquaresOnLine(int a, int b)
{
    if (a == b) return 0;
    int r1 = a/11, c1 = a%11, r2 = b/11, c2 = b%11;

    int rd = r2 - r1, cd = c2 - c1;
    if (rd == 0 || cd == 0 || abs(rd) == abs(cd))
        return 1;
    else
        return 0;
}

int areOnLine(Position pos, int a, int b)
{
    if (a == b) return 0;
    int r1 = a/11, c1 = a%11, r2 = b/11, c2 = b%11;

    int rd = r2 - r1, cd = c2 - c1;
    if (rd == 0 || cd == 0 || abs(rd) == abs(cd))
        ;
    else
        return 0;
    
    int dist = max(abs(rd), abs(cd));
    if (rd > 0) rd = 1;
    else if (rd < 0) rd = -1;
    if (cd > 0) cd = 1;
    else if (cd < 0) cd = -1;
    
    int r = r1 + rd, c = c1 + cd, d;
    int128 occ = or128(pos.arrows, or128(pos.occ[0], pos.occ[1]));
    for (d = 1; d <= dist; d++){
        if (bitSet128(occ, 11*r+c))
            return 0;
        r += rd; c += cd;
    }
    return 1;
}

int isLegalMove(Position * pos, int src, int dst, int target)
{
    if (!bitSet128(pos->occ[pos->turn], src))
        return 0;
    
    pos->occ[pos->turn] = xor128(pos->occ[pos->turn], SingleBit[src]);
    int valid = areOnLine(*pos, src, dst) && (target == -1 || areOnLine(*pos, dst, target));
    pos->occ[pos->turn] = xor128(pos->occ[pos->turn], SingleBit[src]);
    return valid;
}

int countLegalMoves(Position pos)
{
    int src, dst, target, c = 0;
    for (src = 0; src < 110; src++){
        for (dst = 0; dst < 110; dst++){
            for (target = 0; target < 110; target++){
                if (isLegalMove(&pos, src, dst, target))
                    c++;
            }
        }
    }
    return c;
}

void afterMove(Position * child, const Position * const par,
               int src, int dst, int target)
{
    *child = *par;
    int128 mask = or128(SingleBit[src], SingleBit[dst]);
    child->occ[par->turn] = xor128(child->occ[par->turn], mask);
    child->arrows = or128(child->arrows, SingleBit[target]);
    child->turn ^= 1;
    
    child->move.src = src;
    child->move.dst = dst;
    child->move.target = target;
}


Position initialPos(void);

Position positionFromString(const char * str1)
{
    char str[101];
    strncpy(str, str1, 101);
    int i;
    for (i = strlen(str); i < 100; i++)
        str[i] = '3';
    str[100] = '\0';

    Position pos = initialPos();
    for (i = 0; i < 100; i++){
        int j = 11 * (i / 10) + i % 10;
        switch (str[i]){
            case '1':  pos.occ[0] = or128(pos.occ[0], SingleBit[j]); break;
            case '2':  pos.occ[1] = or128(pos.occ[1], SingleBit[j]); break;
            case '3':  pos.arrows = or128(pos.arrows, SingleBit[j]); break;
            default:  break;
        }
    }

    pos.turn = (count128(pos.arrows) % 2 == 0);

    return pos;
}

