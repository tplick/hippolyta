// search.c
// Copyright 2009, 2010, 2014 Tom Plick (tomplick "AT" gmail.com).
// This code is released under the GPL version 3; see file LICENSE for info.

#include <time.h>
#include <string.h>

#define TABLE_SIZE (1 << 20)
Move RecTable[TABLE_SIZE] = {{0, 0, 0}};
Move Table[100] = {{0, 0, 0}};
Move PropTable[128][128][128] = {{{{0, 0, 0}}}};
Move PropTable2[128][128] = {{{0, 0, 0}}};

int getTableMove(int n)
{    
    int src = Table[n].src, dst = Table[n].dst, target = Table[n].target;
    return src + (dst << 8) + (target << 16);
}

int hashSlot(const Position * pos)
{
    return (pos->occ[0].low + pos->occ[0].high + pos->occ[1].low + pos->occ[1].high + pos->arrows.low + pos->arrows.high) 
                 % (TABLE_SIZE - 1) + pos->turn;
}

void shiftTableMoves()
{
    int i;
    for (i = 99; i > 0; i--){
        Table[i] = Table[i-1];
    }
}


#define NEGASCOUT_BODY \
  { \
    a = -minimax(&child, depth-1, -b, -alpha); \
    \
    if (a > alpha){ \
        Table[depth] = RecTable[slot] = *prop = *prop2 = child.move; \
        alpha = a; \
    } \
    \
    if (alpha >= beta){ \
        Table[depth] = RecTable[slot] = *prop = *prop2 = child.move; \
        return alpha; \
    } \
    \
    if (alpha >= b){ \
        alpha = -minimax(&child, depth-1, -beta, -alpha); \
        if (alpha >= beta){ \
            Table[depth] = RecTable[slot] = *prop = *prop2 = child.move; \
            return alpha; \
        } \
    } \
    \
    b = alpha + 1; \
  }

#define DL_NEGASCOUT_BODY \
  { \
    a = -depthless_minimax(&child, expand, -b, -alpha, level+1, &childBestLeaf, &childBestMove, pos); \
    \
    if (a > alpha){ \
        *bestMove = Table[0] = RecTable[slot] = *prop = *prop2 = child.move; \
        *bestLeaf = childBestLeaf; \
        alpha = a; \
    } \
    \
    if (alpha >= beta){ \
        *bestMove = Table[0] = RecTable[slot] = *prop = *prop2 = child.move; \
        *bestLeaf = childBestLeaf; \
        return alpha; \
    } \
    \
    if (alpha >= b){ \
        alpha = -depthless_minimax(&child, expand, -beta, -alpha, level+1, &childBestLeaf, &childBestMove, pos); \
        if (alpha >= beta){ \
            *bestMove = Table[0] = RecTable[slot] = *prop = *prop2 = child.move; \
            *bestLeaf = childBestLeaf; \
            return alpha; \
        } \
    } \
    \
    b = alpha + 1; \
  }

int Direcs[8] = {10, -10, 11, -11, 12, -12, 1, -1};


#define OVER_LEGAL_MOVES(pos, function) \
do { \
    int i, i1, i2, d1, d2, src, dst, target; \
    int128 sources = pos.occ[pos.turn]; \
    int128 occ = or128(or128(pos.occ[0], pos.occ[1]), pos.arrows); \
    \
    int Direcs1[8] = {10, -10, 11, -11, 12, -12, 1, -1}; \
    int Direcs2[8] = {10, -10, 11, -11, 12, -12, 1, -1}; \
    shuffle(Direcs1, 8); \
    shuffle(Direcs2, 8); \
    \
    for (i = 0; i < 4; i++){ \
        src = lowestBit128(sources); \
        if (i < 3) \
            sources = xor128(sources, SingleBit[src]); \
        \
        for (i1 = 0; i1 < 8; i1++){ \
            d1 = Direcs1[i1]; \
            for (dst = src + d1; dst >= 0 && !bitSet128(occ, dst); dst += d1){ \
                for (i2 = 0; i2 < 8; i2++){ \
                    d2 = Direcs2[i2]; \
                    for (target = dst + d2; target >= 0 && (target == src || !bitSet128(occ, target)); target += d2){ \
                        function(src, dst, target); \
                    } \
                } \
            } \
        } \
    } \
} while (0)

int minimax(Position * pos, const int depth, int alpha, const int beta)
{    
    Nodes++;

    if (depth == 0){
        return eval(pos);
    }
    const int slot = hashSlot(pos);
    
    int childeval = 0, bestVal = depth < 3 ? 0 : -eval(pos);
    int ppi = 0, i;
    
    int b = 0, a;
    
    int src, dst, target, i1, i2, d1, d2;
    
    Move * prop  = &PropTable[pos->move.src][pos->move.dst][pos->move.target];
    Move * prop2 = &PropTable2[pos->move.src][pos->move.dst];

    Move * first;
    Position child;
    
    first = &RecTable[slot];
    if (isLegalMove(pos, first->src, first->dst, first->target)){
        afterMove(&child, pos, first->src, first->dst, first->target);
        NEGASCOUT_BODY;
    }
    
    first = &Table[depth];
    if (isLegalMove(pos, first->src, first->dst, first->target)){
        afterMove(&child, pos, first->src, first->dst, first->target);
        NEGASCOUT_BODY;
    }
    
    first = prop2;
    if (isLegalMove(pos, first->src, first->dst, first->target)){
        afterMove(&child, pos, first->src, first->dst, first->target);
        NEGASCOUT_BODY;
    }
    
    first = prop;
    if (isLegalMove(pos, first->src, first->dst, first->target)){
        afterMove(&child, pos, first->src, first->dst, first->target);
        NEGASCOUT_BODY;
    }

    
    int128 occ = or128(pos->arrows, or128(pos->occ[0], pos->occ[1]));
    int128 sources = pos->occ[pos->turn];
    
    Position Postponed[4900];
    

    int srcs[4];
    for (i = 0; i < 4; i++){
        srcs[i] = lowestBit128(sources);
        if (i < 3)
            sources = xor128(sources, SingleBit[srcs[i]]);
    }
    shuffle(srcs, 4);


    for (i = 0; i < 4; i++){
        src = srcs[i];

        for (i1 = 0; i1 < 8; i1++){
            d1 = Direcs[i1];
            for (dst = src + d1; dst >= 0 && !bitSet128(occ, dst); dst += d1){
                for (i2 = 0; i2 < 8; i2++){
                    d2 = Direcs[i2];
                    for (target = dst + d2; target >= 0 && (target == src || !bitSet128(occ, target)); target += d2){
                        afterMove(&child, pos, src, dst, target);
                        if (depth <= 1 || (childeval = eval(&child)) < bestVal){
                            bestVal = childeval;
                            NEGASCOUT_BODY;
                        } else {
                            Postponed[ppi++] = child;
                        }
                    }
                }
            }
        }
    }
    
    for (i = 0; i < ppi; i++){
        child = Postponed[i];
        NEGASCOUT_BODY;
    }
    
    return alpha;
}


#define BIT_SET(field, n) \
    (field[n >> 5] & (1 << (n & 31)))
#define SET_BIT(field, n) \
    (field[n >> 5] |= (1 << (n & 31)))


int depthless_minimax(Position * pos, uint32_t expand[], int alpha, const int beta, int level,
        Position * bestLeaf, Move * bestMove, Position * parent)
{    
    Nodes++;
    const int slot = hashSlot(pos);

    if (!BIT_SET(expand, slot)){
        *bestLeaf = *pos;
        if (level % 2 == 0){
            return minimax(pos, 2, alpha, beta);
        } else {
            return minimax(pos, 3, alpha, beta);
        }
    } else if (level >= 100){
        *bestLeaf = *pos;
        return eval(pos);
    }
    
    int childeval, bestVal = 0;
    int ppi = 0, i;
    
    int b = 0, a;
    
    int src, dst, target, i1, i2, d1, d2;
    
    Move * prop  = &PropTable[pos->move.src][pos->move.dst][pos->move.target];
    Move * prop2 = &PropTable2[pos->move.src][pos->move.dst];

    Move * first;
    Position child;
    Move childBestMove = {0, 0, 0};
    Position childBestLeaf;


    first = &RecTable[slot];
    if (isLegalMove(pos, first->src, first->dst, first->target)){
        afterMove(&child, pos, first->src, first->dst, first->target);
        DL_NEGASCOUT_BODY;
    }
    
    first = &Table[0];
    if (isLegalMove(pos, first->src, first->dst, first->target)){
        afterMove(&child, pos, first->src, first->dst, first->target);
        DL_NEGASCOUT_BODY;
    }
    
    first = prop2;
    if (isLegalMove(pos, first->src, first->dst, first->target)){
        afterMove(&child, pos, first->src, first->dst, first->target);
        DL_NEGASCOUT_BODY;
    }
    
    first = prop;
    if (isLegalMove(pos, first->src, first->dst, first->target)){
        afterMove(&child, pos, first->src, first->dst, first->target);
        DL_NEGASCOUT_BODY;
    }

    
    int128 occ = or128(pos->arrows, or128(pos->occ[0], pos->occ[1]));
    int128 sources = pos->occ[pos->turn];
    
    Position Postponed[4900];
    
    int srcs[4];
    for (i = 0; i < 4; i++){
        srcs[i] = lowestBit128(sources);
        if (i < 3)
            sources = xor128(sources, SingleBit[srcs[i]]);
    }
    shuffle(srcs, 4);


    for (i = 0; i < 4; i++){
        src = srcs[i];
        
        for (i1 = 0; i1 < 8; i1++){
            d1 = Direcs[i1];
            for (dst = src + d1; dst >= 0 && !bitSet128(occ, dst); dst += d1){
                for (i2 = 0; i2 < 8; i2++){
                    d2 = Direcs[i2];
                    for (target = dst + d2; target >= 0 && (target == src || !bitSet128(occ, target)); target += d2){
                        afterMove(&child, pos, src, dst, target);
                        if ((childeval = eval(&child)) < bestVal){
                            bestVal = childeval;
                            DL_NEGASCOUT_BODY;
                        } else {
                            Postponed[ppi++] = child;
                        }
                    }
                }
            }
        }
    }
    
    for (i = 0; i < ppi; i++){
        child = Postponed[i];
        DL_NEGASCOUT_BODY;
    }


    *bestLeaf = childBestLeaf;
    return alpha;
}


Move depthless_search_top(Position pos, float seconds, bool * can_see_win)
{
    uint32_t expand[1 << 15];
    memset(expand, 0, sizeof(expand));
    Position bestLeaf;
    Move savedBestMove = {0, 0, 0};
    *can_see_win = false;

    int forever = (getenv("FOREVER") != NULL);

    int j;
    time_t finishTime = clock() + seconds * CLOCKS_PER_SEC;
    for (j = 0; ; j++){
        shuffle(Direcs, 8);
        bestLeaf = pos;
        Move bestMove = {0, 0, 0};

        int value = depthless_minimax(&pos, expand, -100000000, 100000000, 0, &bestLeaf, &bestMove, &pos);
        if (value > -100000000){
            savedBestMove = bestMove;
        }
        SET_BIT(expand, hashSlot(&bestLeaf));

#ifdef DEBUG
        int ecount = 0, i;
        for (i = 0; i < (1<<15); i++){
            ecount += POPCOUNT(expand[i]);
        }

        fprintf(stderr,
                "Done %d... expand count is %d... seen %llu nodes... value is %d... got move (%d, %d, %d)\n",
                j+1, ecount, Nodes, value,
                bestMove.src, bestMove.dst, bestMove.target);
#endif /* ifdef DEBUG */

        if (!forever && j > 0 && clock() >= finishTime){
            return savedBestMove;
        }
        if (j > 0 && abs(value) >= 100000000){
            *can_see_win = true;
            return savedBestMove;
        }
    }
}



#define DETERMINISTIC_OVER_MOVES(pos, src, dst, target) \
    for (src = 0; src < 110; src++) \
        for (dst = 0; dst < 110; dst++) \
            for (target = 0; target < 110; target++) \
                if (isLegalMove(&pos, src, dst, target))

