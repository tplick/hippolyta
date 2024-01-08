// tests.c
// Copyright 2014 Tom Plick (tomplick "AT" gmail.com).
// This code is released under the GPL version 3; see file LICENSE for info.

#define TEST(name, expression) \
    do { \
        result = (expression); \
        printf("%d.  Testing %s...  %s.\n", ++index, name, (result ? "OK" : "Failed")); \
        if (!result)  failed++; \
    } while (0)

int testRecursiveSum(int n)
{
    if (n == 0){
        return n;
    } else {
        return n + testRecursiveSum(n-1);
    }
}

bool areMovesEqual(Move a, Move b)
{
    return a.src == b.src && a.dst == b.dst && a.target == b.target;
}

int run_tests(){
    int index = 0;
    int result;
    int failed = 0;


    TEST("number of arrows in boundary", count128(initialPos().arrows) == 21);
    TEST("number of Amazons on empty board",  count128(or128(initialPos().occ[0], initialPos().occ[1])) == 0);
    TEST("number of Amazons in starting position", count128(or128(initial_position.occ[0], initial_position.occ[1])) == 8);

    TEST("number of bits in single bit", count128(SingleBit[63]) == 1);
    TEST("number of bits in blurred single bit", count128(blur(SingleBit[63])) == 9);
    TEST("number of bits in doubly blurred single bit", count128(blur(blur(SingleBit[63]))) == 25);

    TEST("zero legal moves", countLegalMoves(positionFromString("1111222233333333333333")) == 0);
    TEST("one legal move", countLegalMoves(positionFromString("1111022223333333333333")) == 1);
    TEST("four legal moves 1", countLegalMoves(positionFromString("1111002222333333333333")) == 4);
    TEST("four legal moves 2", countLegalMovesAlt(positionFromString("1111002222333333333333")) == 4);

    TEST("thirty-six legal moves 1", countLegalMoves(positionFromString("111100000033333333332222")) == 36);
    TEST("thirty-six legal moves 2", countLegalMovesAlt(positionFromString("111100000033333333332222")) == 36);
    TEST("eighteen legal moves", countLegalMoves(positionFromString("000111100033333333332222")) == 18);
    TEST("three legal moves 1", countLegalMoves(positionFromString("0130131031333333333322220")) == 3);
    TEST("three legal moves 2", countLegalMovesAlt(positionFromString("0130131031333333333322220")) == 3);

    TEST("legal moves from starting position 1", countLegalMoves(positionFromString(initial_setup)) == 2176);
    TEST("legal moves from starting position 2", countLegalMovesAlt(positionFromString(initial_setup)) == 2176);

    TEST("territory at start", terr(initial_position.occ[0], initial_position.occ[1], initial_position.arrows) == 0);
    TEST("terrLines at start", terrLines(initial_position.occ[0], initial_position.occ[1], initial_position.arrows) == 0);
    TEST("territory 0", evalStruct(positionFromString("1111022223333333333333")) == 0);
    TEST("territory < 0", evalStruct(positionFromString("1111020222333333333333")) < 0);

    TEST("rim and arrow boundary do not overlap", count128(and128(Rim, initialPos().arrows)) == 0);
    TEST("number of squares on rim", count128(Rim) == 36);
    TEST("number of squares in Rim2", count128(Rim2) == 64);
    TEST("blurred rim includes arrow boundary", equal128(initialPos().arrows, and128(initialPos().arrows, blur(Rim))));
    TEST("blurred rim is Rim2", equal128(Rim2, xor128(blur(Rim), initial_position.arrows)));


    bool can_see_win;
    TEST("find win 1", (depthless_search_top(positionFromString("1111002222"), 1.0, &can_see_win),
                        can_see_win));
    TEST("find win 2", (depthless_search_top(positionFromString("1111022223"), 1.0, &can_see_win),
                        can_see_win));

    int8_t d1 = 3, g1 = 6;
    int8_t d7 = d1 + 66, g7 = g1 + 66;
    Move valid_move_1 = {d1, d7, g7};
    Move valid_move_2 = {g1, g7, d7};
    minimax(&initial_position, 2, -100000000, 100000000);
    Move gen_move = Table[2];
    TEST("minimax to depth 2 from starting position",
             (areMovesEqual(gen_move, valid_move_1) || areMovesEqual(gen_move, valid_move_2)));

    TEST("number of bits in Center6", count128(Center6) == 36);
    TEST("number of bits in Center8", count128(Center8) == 64);
    TEST("number of bits in small quadrant", count128(SmallQuadrants[2]) == 9);

    TEST("lowest bit 1",   lowestBit128(Center6) == 24);
    TEST("lowest bit 2",   lowestBit128(initial_position.arrows) == 10);

    TEST("number of arrows in filled board", count128(positionFromString("11112222").arrows) == 92 + 21);


    int  red_bits[4] = {3, 6, 33, 42};
    int blue_bits[4] = {108 - 3, 108 - 6, 108 - 33, 108 - 42};
    int128  red_field = fieldFromBits(red_bits, 4);
    int128 blue_field = fieldFromBits(blue_bits, 4);
    TEST("field from bits 1", equal128(initial_position.occ[0], red_field));
    TEST("field from bits 2", equal128(initial_position.occ[1], blue_field));
    int single_elt_array[1] = {7};
    TEST("field from bits 3", equal128(SingleBit[7], fieldFromBits(single_elt_array, 1)));


    // test the move generator
    Position testpos1 = positionFromString("1111222200000"),
             testpos2 = positionFromString("1002003330000001000023333100200003332000133"),
             testpos3 = positionFromString("0000100030320010333332200000110000032");
    TEST("equal numbers of moves 1", countLegalMoves(testpos1) == countLegalMovesAlt(testpos1));
    TEST("equal numbers of moves 2", countLegalMoves(testpos2) == countLegalMovesAlt(testpos2));
    TEST("equal numbers of moves 3", countLegalMoves(testpos3) == countLegalMovesAlt(testpos3));

    TEST("100 bits in BoardField", count128(BoardField) == 100);
    TEST("board field and initial arrows do not overlap", equal128(Zero, and128(BoardField, initial_position.arrows)));

    // this one is last because it may crash the program!
    TEST("deep recursion", testRecursiveSum(1999) == 1999000);


    if (failed){
        printf("%d test(s) failed.\n", failed);
    } else {
        printf("All tests passed successfully!\n");
    }

    return failed;
}

