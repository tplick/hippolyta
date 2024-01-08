// display.c: display routines
// Copyright 2009, 2010, 2014 Tom Plick (tomplick "AT" gmail.com).
// This code is released under the GPL version 3; see file LICENSE for info.


static const char * displays = ".12X";
void printPosition(Position pos)
{
    int r, c;
    fprintf(stderr, "+----------+\n");
    for (r = 0; r < 10; r++){
        fprintf(stderr, "|");
        for (c = 0; c < 10; c++){
            fprintf(stderr, "%c", displays[at(pos, r, c)]);
        }
        fprintf(stderr, "|\n");
    }
    fprintf(stderr, "+----------+\n");
    fprintf(stderr, " %d to move\n", pos.turn + 1);
    fprintf(stderr, "val is %d\n", eval(&pos));
}

void squareName(int sq, char * s)
{
    int r = sq / 11, c = sq % 11;
    snprintf(s, 4, "%c%d", c + 'a', r + 1);
}

void printSquare(int sq)
{
    char s[10];
    squareName(sq, s);
    printf("%s\n", s);
}

