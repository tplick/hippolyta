// blur.c
// Copyright 2009, 2010, 2014 Tom Plick (tomplick "AT" gmail.com).
// This code is released under the GPL version 3; see file LICENSE for info.


int128 blur(int128 field)
{
    int128 mask = or128(shiftUp(field, 1), shiftDown(field, 1));
    field = or128(field, mask);
    mask = or128(shiftUp(field, 11), shiftDown(field, 11));
    field = or128(field, mask);
    return field;
}


/* For the record, I named this function back in 2010...
   way before that song! */
int128 blurLines(int128 Red, int128 blockers)
{
    int iter, j, dir;
    int128 field, spread, agglom = Red;
    const int Dirs[4] = {1, 11, 10, 12};

    int maxiters = 9;

    for (j = 0; j < 4; j++){
        dir = Dirs[j];
        field = Red;
        for (iter = 0; iter < maxiters; iter++){
            spread = or128(shiftUp(field, dir), shiftDown(field, dir));
            spread = and128(spread, blockers);
            if (equal128(field, or128(field, spread)))
                break;
            field = or128(field, spread);
        }
        agglom = or128(agglom, field);
    }
    return agglom;
}

