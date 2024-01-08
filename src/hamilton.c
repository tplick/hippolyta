// hamilton.c: try to fill isolated territories along a Hamiltonian path.
// Copyright 2014 Tom Plick (tomplick "AT" gmail.com).
// This code is released under the GPL version 3; see file LICENSE for info.


// Return the number of cells that can be filled this way.
int tryToFillRoom(int128 room, int startingPoint, int * nextDst, time_t deadline, unsigned int * count, int * expired)
{
    if (*expired)
        return -1;

    if (((*count)++ & 1023) == 0 && clock() > deadline){
        *expired = 1;
        *nextDst = -1;
        return -1;
    }

    // for each adjacent destination:
    //      move to that destination
    //      verify that the territory is still connected
    //      check 1 + recursive call; if this is better than the max, choose this

    int bestTerritory = 0;
    *nextDst = -1;
    int childNextDst;
    int128 neighbors = xor128(and128(blur(SingleBit[startingPoint]), room),
                              SingleBit[startingPoint]);

    int sources[8];
    int n_sources = 0;

    while (!zero128(neighbors)){
        int nei = lowestBit128(neighbors);
        neighbors = xor128(neighbors, SingleBit[nei]);
        sources[n_sources++] = nei;
    }

    shuffle(sources, n_sources);

    int i;
    for (i = 0; i < n_sources; i++){
        int nei = sources[i];
        int childValue = tryToFillRoom(xor128(room, SingleBit[startingPoint]), nei, &childNextDst,
                    deadline, count, expired);
        if (1 + childValue > bestTerritory){
            bestTerritory = 1 + childValue;
            *nextDst = nei;
        }
    }

    return bestTerritory;
}

int128 getRoomContainingSquare(const Position * pos, int square)
{
    int128 territory = SingleBit[square];
    int128 obstruction = not128(or128(pos->occ[pos->turn ^ 1], pos->arrows));

    while (1){
        int128 new_territory = and128(blur(territory), obstruction);
        if (equal128(territory, new_territory))
            break;
        else
            territory = new_territory;
    }

    return territory;
}

bool isRoomSingleAndIsolated(const Position * pos, int square)
{
    int128 territory = getRoomContainingSquare(pos, square);

    if (count128(and128(territory, pos->occ[pos->turn])) != 1)
        return 0;

    // the halo of the room must consist entirely of arrows
    int128 halo = xor128(blur(territory), territory);
    return equal128(halo, and128(halo, pos->arrows));
}

Move getEndgameRecommendation(const Position * pos, int square, float seconds)
{
    Move move = {0, 0, 0};
    
    if (!isRoomSingleAndIsolated(pos, square))
        return move;

    int nextDst = -1;
    int128 room = getRoomContainingSquare(pos, square);
#ifdef DEBUG
    printf("Size of room is %d\n", count128(room));
#endif
    unsigned int count = 0;
    int expired = 0;
    int size = tryToFillRoom(room, square, &nextDst, clock() + seconds * CLOCKS_PER_SEC, &count, &expired);
#ifdef DEBUG
    printf("found Hamiltonian path of length %d\n", size);
#endif

    if (nextDst < 0 || size < 0 || count128(room) - 1 > size){
        return move;
    }

#ifdef DEBUG
    printf("Using it!\n");
#endif

    move.src = square;
    move.dst = nextDst;
    move.target = square;
    return move;
}

