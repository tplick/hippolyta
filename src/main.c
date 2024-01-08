// main.c
// Copyright 2009, 2010, 2014 Tom Plick (tomplick "AT" gmail.com).
// This code is released under the GPL version 3; see file LICENSE for info.


#define VERSION_STRING \
    "Hippolyta version 2.0 (December 14, 2014)"

#define COPYRIGHT_STRING \
    "Copyright 2009, 2010, 2014 Tom Plick (tomplick at gmail.com)"


typedef int bool;

#define true 1
#define false 0

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

unsigned long long int Nodes = 0;

bool strequal(char * a, char * b)
{
    return strcmp(a, b) == 0;
}

void shuffle(int array[], int n)
{
    int i, j, swap;
    for (i = 0; i < n - 1; i++){
        j = i + rand() % (n - i);

        swap = array[i];
        array[i] = array[j];
        array[j] = swap;
    }
}


#include "bits128.c"
#include "position.c"
#include "blur.c"
#include "tables.c"
#include "territory.c"
#include "hamilton.c"
#include "search.c"


Position initialPos()
{
    Position p;
    p.occ[0].low = 0ULL;
    p.occ[0].high = 0ULL;
    p.occ[1].low = 0ULL;
    p.occ[1].high = 0ULL;
    p.arrows.low = 18023198899569664ULL;
    p.arrows.high = 144080020892028930ULL;
    p.turn = 0;
    p.move.src = p.move.dst = p.move.target = 0;
    return p;
}

char * initial_setup =
    "0001001000000000000000000000001000000001000000000000000000002000000002000000000000000000000002002000";

Position initial_position;
Position empty_position;


// display functions
#include "display.c"


void set(Position * pos, int cell, int val)
{
    if (val == 3){
        pos->arrows = or128(pos->arrows, SingleBit[cell]);
    } else if (val){
        pos->occ[val-1] = or128(pos->occ[val-1], SingleBit[cell]);
    }
}


static int counter_countLegalMovesAlt;
void incrementCount(int src, int dst, int target)
{
    counter_countLegalMovesAlt++;
}
int countLegalMovesAlt(Position pos)
{
    counter_countLegalMovesAlt = 0;
    OVER_LEGAL_MOVES(pos, incrementCount);
    return counter_countLegalMovesAlt;
}


// the unit tests:
#include "tests.c"

// function showHelp:
#include "help.c"


void benchmark()
{
    time_t end_time = clock() + 60 * CLOCKS_PER_SEC;
    unsigned long long int iterations = 0;
    int evalSum = 0, i;

    printf("Running eval on starting position for one minute...\n");

    while (clock() < end_time){
        for (i = 0; i < 60; i++){
            evalSum += eval(&initial_position);
        }
        iterations += 60ULL;
    }

    printf("Ran %llu in one minute (%llu per second).\n", iterations * (evalSum ? 0 : 1), iterations / 60ULL);
    printf("Done.\n");
}

void terr_benchmark()
{
    time_t end_time = clock() + 60 * CLOCKS_PER_SEC;
    unsigned long long int iterations = 0;
    int evalSum = 0, i;

    printf("Running terr on starting position for one minute...\n");

    while (clock() < end_time){
        for (i = 0; i < 60; i++){
            evalSum += terr(initial_position.occ[0], initial_position.occ[1], initial_position.arrows);
        }
        iterations += 60ULL;
    }

    printf("Ran %llu in one minute (%llu per second).\n", iterations * (evalSum ? 0 : 1), iterations / 60ULL);
    printf("Done.\n");
}



int countLegalMoves2Deep(Position pos)
{
    int src, dst, target, src2, dst2, target2, c = 0, a = 0;
    Position child;

    for (src = 0; src < 110; src++){
        if (bitSet128(pos.occ[pos.turn], src))
        for (dst = 0; dst < 110; dst++){
            if (areSquaresOnLine(src, dst))
            for (target = 0; target < 110; target++){
                if (areSquaresOnLine(dst, target))
                if (isLegalMove(&pos, src, dst, target)){
                    printf("  (%d)...\n", ++a);
                    afterMove(&child, &pos, src, dst, target);
                    for (src2 = 0; src2 < 110; src2++){
                        if (bitSet128(child.occ[child.turn], src2))
                        for (dst2 = 0; dst2 < 110; dst2++){
                            if (areSquaresOnLine(src2, dst2))
                            for (target2 = 0; target2 < 110; target2++){
                                if (areSquaresOnLine(dst2, target2))
                                if (isLegalMove(&child, src2, dst2, target2)){
                                    c++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return c;
}



Position makeRandomStartingPosition()
{
    Position pos = initial_position;
    int i, sq;

    while (1){
        pos.occ[0] = pos.occ[1] = Zero;

        for (i = 0; i < 4; i++){
            sq = 11 * (rand() % 10) + (rand() % 10);
            pos.occ[0] = or128(pos.occ[0], SingleBit[sq]);
            sq = 11 * (rand() % 10) + (rand() % 10);
            pos.occ[1] = or128(pos.occ[1], SingleBit[sq]);
        }

        if (count128(pos.occ[0]) == 4 && count128(pos.occ[1]) == 4
                && equal128(Zero, and128(pos.occ[0], pos.occ[1]))){
            return pos;
        }
    }
}



int main(int argc, char * argv[])
{
#ifdef DEBUG
    atexit(printEvalCount);
#endif

    srand(time(0));
    initializeTables();
    shuffle(Direcs, 8);
    initial_position = positionFromString(initial_setup);
    empty_position = positionFromString("");

    int argIndex = 1;
#define   NEXT_ARGUMENT()   (argIndex < argc ? argv[argIndex++] : NULL)

    float seconds = 60;
    bool anyArgs = false;
    Position p = initial_position;
    bool shouldPrintBoard = false;

    while (true){
        char * arg = NEXT_ARGUMENT();

        if (arg == NULL){
            break;
        } else if (strequal(arg, "-h")){
            showHelp(argv);
            return 0;
        } else if (strequal(arg, "-t")){
            printf("Running tests...\n");
            run_tests();
            return 0;
        } else if (strequal(arg, "-v")){
            printf("%s\n", VERSION_STRING
#ifdef ALT
            " [alternate]"
#endif
            );
            return 0;
        } else if (strequal(arg, "-b")){
            benchmark();
            return 0;
        } else if (strequal(arg, "-terr")){
            terr_benchmark();
            return 0;
        } else if (strequal(arg, "-s")){
            arg = NEXT_ARGUMENT();
            if (arg){
                seconds = strtod(arg, NULL);
            }
        } else if (strequal(arg, "-p")){
            shouldPrintBoard = true;
        } else if (arg[0] == '0' || arg[0] == '1' || arg[0] == '2' || arg[0] == '3'){
            p = positionFromString(arg);
        } else if (strequal(arg, "-1")){
            printf("Counting number of moves from position\n");
            printPosition(p);
            printf("%d\n", countLegalMoves(p));
            return 0;
        } else if (strequal(arg, "-2")){
            printf("Counting number of two-move chains from position\n");
            printPosition(p);
            printf("%d\n", countLegalMoves2Deep(p));
            return 0;
        } else if (strequal(arg, "-z")){
            int max = 0, m;
            while (1){
                p = makeRandomStartingPosition();
                m = countLegalMoves(p);

                if (m > max){
                    max = m;
                    printPosition(p);
                    printf("%d\n", max);
                }
            }
            return 0;
        } else {
            showHelp(argv);
            return 1;
        }

        anyArgs = true;
    }

    if (!anyArgs){
        showHelp(argv);
        return 1;
    }

    
    if (count128(p.occ[0]) != 4 || count128(p.occ[1]) != 4){
        printf("Bad position.  There must be exactly 4 Amazons of each color.\n");
        return 2;
    }

#ifdef DEBUG
    printPosition(p);
#endif
    if (shouldPrintBoard){
        printPosition(p);
    }


    int mine = 0, yours = 0, cold = 0;
    if (!isGameStillHot(p.occ[0], p.occ[1], p.arrows)){
        cold = 1;
        terr(p.occ[p.turn], p.occ[p.turn ^ 1], p.arrows);
        mine = count128(G_Red);
        yours = count128(G_Blue);
        if (mine - yours < -5){
            printf("resign\n\n\n\n");
            return 0;
        }
    }

    bool can_see_win = false;

    Move move = depthless_search_top(p, seconds, &can_see_win);
    if (shouldPrintBoard){
        printf("(saw %llu nodes)\n", Nodes);
    }
    int src = move.src, dst = move.dst, target = move.target;

    if (!can_see_win && src != dst){
        Move hamMove = getEndgameRecommendation(&p, src, seconds / 6.0);
        if (hamMove.src != hamMove.dst){
            move = hamMove;
            src = move.src;
            dst = move.dst;
            target = move.target;
        }
    }

    if (src == dst){
        printf("resign\n\n\n\n");
        return 0;
    }


    printSquare(src);
    printSquare(dst);
    printSquare(target);

    if (!cold){
        printf("\n");
        return 0;
    }


    Position child;
    afterMove(&child, &p, move.src, move.dst, move.target);
    terr(child.occ[p.turn], child.occ[p.turn ^ 1], child.arrows);
    mine = count128(G_Red);
    yours = count128(G_Blue);

    if (mine > 4 && yours > 4){
        printf("%d %d\n", mine - 4, yours - 4);
    } else {
        printf("\n");
    }

    return 0;
}

