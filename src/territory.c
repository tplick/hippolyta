// terr.c
// Copyright 2009, 2010, 2014 Tom Plick (tomplick "AT" gmail.com).
// This code is released under the GPL version 3; see file LICENSE for info.


int terr(int128 Red, int128 Blue, int128 arrows)
{
    arrows = not128(arrows);
    
    {
        int128 RedExt  = and128(blur( Red), arrows);
        int128 BlueExt = and128(blur(Blue), arrows);
        
        RedExt  = and128(RedExt,  not128(Blue));
        BlueExt = and128(BlueExt, not128(Red));
        
        Red  = or128(Red, RedExt);
        Blue = or128(Blue, BlueExt);
    }
    
    int tropism = Tropism = count128(Red) - count128(Blue);
    
    while (1){
        int128 RedExt  = and128(blur( Red), arrows);
        int128 BlueExt = and128(blur(Blue), arrows);
        
        RedExt  = and128(RedExt,  not128(Blue));
        BlueExt = and128(BlueExt, not128(Red));
        
        if (equal128(Red,  or128(Red,  RedExt)) &&
            equal128(Blue, or128(Blue, BlueExt)))
                break;
        
        Red  = or128(Red, RedExt);
        Blue = or128(Blue, BlueExt);
    }
    
    G_Red = Red;  G_Blue = Blue;
    G_Red  = and128(G_Red, not128(Blue));
    G_Blue = and128(G_Blue, not128(Red));


    int x = ((count128(Red) - count128(Blue)));
    return (x << 8) - tropism;
}

int tropism(int128 Red, int128 Blue, int128 arrows)
{
    arrows = not128(arrows);
    
    {
        int128 RedExt  = and128(blur( Red), arrows);
        int128 BlueExt = and128(blur(Blue), arrows);
        
        RedExt  = and128(RedExt,  not128(Blue));
        BlueExt = and128(BlueExt, not128(Red));
        
        Red  = or128(Red, RedExt);
        Blue = or128(Blue, BlueExt);
    }
    
    return count128(Red) - count128(Blue);
}


int sq(int n){ return n*n; }

int terrLines(int128 Red, int128 Blue, int128 arrows)
{
    int trop = tropism(Red, Blue, arrows);

    while (1){
        int128 RedExt  = blurLines(Red,  not128(or128(Blue, arrows)));
        int128 BlueExt = blurLines(Blue, not128(or128(Red,  arrows)));

        RedExt  = and128(RedExt,  not128(Blue));
        BlueExt = and128(BlueExt, not128(Red));

        if (equal128(Red,  or128(Red,  RedExt)) &&
                 equal128(Blue, or128(Blue, BlueExt)))
            break;

        Red  = or128(Red, RedExt);
        Blue = or128(Blue, BlueExt);
    }

    return ((count128(Red) - count128(Blue)) << 8) - trop;
}




int isGameStillHot(int128 Red, int128 Blue, int128 arrows)
{
    arrows = not128(arrows);
    int128 origRed = Red, origBlue = Blue;
    
    while (1){
        int128 RedExt  = and128(blur( Red), arrows);
        int128 BlueExt = and128(blur(Blue), arrows);
        
        RedExt  = and128(RedExt,  not128(Blue));
        BlueExt = and128(BlueExt, not128(Red));
        
        if (equal128(Red,  or128(Red,  RedExt)) &&
            equal128(Blue, or128(Blue, BlueExt)))
                break;
        
        Red  = or128(Red, RedExt);
        Blue = or128(Blue, BlueExt);
    }
    
    Red  = and128(blur( Red), arrows);
    Blue = and128(blur(Blue), arrows);
    int128 overlap = and128(Red, Blue);
    overlap = and128(overlap, not128(origRed));
    overlap = and128(overlap, not128(origBlue));
    return !equal128(Zero, overlap);
}


int numberOfAmazonsCordonedOff(Position * pos, int whose)
{
    int128 sources = and128(pos->occ[whose], Rim);
    int128 obstruction = not128(or128(pos->occ[whose], or128(pos->occ[whose ^ 1], pos->arrows)));
    int totalPenalty = 0;

    while (!equal128(sources, Zero)){
        int src = lowestBit128(sources);
        sources = xor128(sources, SingleBit[src]);

        // immediate neighborhood of this Amazon
        int128 field = SingleBit[src];
        field = blur(field);
        field = and128(field, obstruction);

        if (equal128(Zero, and128(field, not128(Rim)))){
            totalPenalty++;
        }
    }

    return totalPenalty;
}



unsigned long long int EvalCount = 0;
int eval(Position * pos)
{
#ifdef DEBUG
    EvalCount++;
#endif /* ifdef DEBUG */

    int baseEval = terrLines(pos->occ[pos->turn], pos->occ[pos->turn ^ 1], pos->arrows);
    int numberOfArrows = count128(pos->arrows) - 21;

    if (numberOfArrows >= 20){
        return baseEval;
    } else {
        return baseEval - 1000 * (20 - numberOfArrows) * (
                    numberOfAmazonsCordonedOff(pos, pos->turn) - numberOfAmazonsCordonedOff(pos, pos->turn ^ 1));
    }
}

// for testing
int evalStruct(Position pos)
{
    return eval(&pos);
}

void printEvalCount()
{
    fprintf(stderr, "eval was called %llu times.\n", EvalCount);
}

