// tables.c: bitfields and their initialization procedures.
// Copyright 2009, 2010, 2014 Tom Plick (tomplick "AT" gmail.com).
// This code is released under the GPL version 3; see file LICENSE for info.

int128 G_Red, G_Blue;
int Tropism;
int128 Center6, Rim, FourPoints, Center8, Rim2, BoardField;
int128 SmallQuadrants[4];

char popcounts[65536], lowbits[65536] = {0};
extern void initEvalTable(void);
const int RowCentrism[10] = {0, 1, 2, 3, 4, 4, 3, 2, 1, 0};

#define SETSQUARE128(field, r, c) \
    do {field = or128(field, SingleBit[11*(r)+(c)]);} while (0)

void initializeTables()
{
    Zero.low = Zero.high = 0ULL;
    
    int i, j, t;
    SingleBit[0].low = 1ULL; SingleBit[0].high = 0ULL;
    for (i = 1; i < 128; i++){
        SingleBit[i] = shiftUp(SingleBit[i-1], 1);
    }
    
    for (i = 0; i < 65536; i++){
        t = 0;
        for (j = 0; j < 16; j++){
            if (i & (1 << j))
                t++;
        }
        popcounts[i] = t;
        
        for (j = 0; j < 16; j++){
            if (i & (1 << j)){
                lowbits[i] = j;
                break;
            }
        }
    }
    

    Center6 = Zero;
    for (i = 2; i < 8; i++){
        for (j = 2; j < 8; j++){
            Center6 = or128(Center6, SingleBit[11*i+j]);
        }
    }

    Center8 = Zero;
    for (i = 1; i < 9; i++){
        for (j = 1; j < 9; j++){
            Center8 = or128(Center8, SingleBit[11*i+j]);
        }
    }


    for (i = 0; i < 4; i++){
        SmallQuadrants[i] = Zero;
    }
    for (i = 0; i < 3; i++){
        for (j = 0; j < 3; j++){
            SETSQUARE128(SmallQuadrants[0], i, j);
            SETSQUARE128(SmallQuadrants[1], i, j+7);
            SETSQUARE128(SmallQuadrants[2], i+7, j);
            SETSQUARE128(SmallQuadrants[3], i+7, j+7);
        }
    }


    Rim = Zero;
    for (i = 0; i < 10; i++){
        Rim = or128(Rim, SingleBit[11*i+0]);
        Rim = or128(Rim, SingleBit[11*i+9]);
        Rim = or128(Rim, SingleBit[11*0+i]);
        Rim = or128(Rim, SingleBit[11*9+i]);
    }

    Rim2 = Zero;
    int adds[4] = {0, 1, 8, 9}, k;
    for (i = 0; i < 10; i++){
        for (k = 0; k < 4; k++){
            Rim2 = or128(Rim2, SingleBit[11*i+adds[k]]);
            Rim2 = or128(Rim2, SingleBit[11*adds[k]+i]);
        }
    }

    FourPoints = Zero;
    FourPoints = or128(FourPoints, SingleBit[36]);
    FourPoints = or128(FourPoints, SingleBit[39]);
    FourPoints = or128(FourPoints, SingleBit[72]);
    FourPoints = or128(FourPoints, SingleBit[69]);


    BoardField = Zero;
    for (i = 0; i < 10; i++){
        for (j = 0; j < 10; j++){
            BoardField = or128(BoardField, SingleBit[11*i+j]);
        }
    }
}

