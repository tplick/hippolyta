// help.c: the help screen
// Copyright 2014 Tom Plick (tomplick "AT" gmail.com).
// This code is released under the GPL version 3; see file LICENSE for info.


void showHelp(char * argv[])
{
    printf(VERSION_STRING
#ifdef ALT
    " [alternate]"
#endif
           "\n" 
           COPYRIGHT_STRING
           "\n"
           "\n"
           "Usage: %s [-p] [-s <sec>] [<position> | -t | -b | -v | -h] \n"
           "  <position>   The position to analyze. \n"
           "  -s <sec>     Think for <sec> seconds.  (default 60; decimals are allowed) \n"
           "  -p           Print board to stderr.  (for debugging purposes) \n"
           "  -t           Run unit tests. \n"
           "  -b           Run eval benchmark. \n"
           "  -v           Show version. \n"
           "  -h           Show this help screen. \n"
           "\n"
           "Position should be given as a string of 100 characters, \n"
           "indicating the contents of squares a1, a2, ..., j10. \n"
           "Use 0 for empty square, 3 for arrow, \n"
           "1 for first player's Amazon, 2 for second player's Amazon. \n"
           "If fewer than 100 characters are given, the remainder \n"
            "of the board will be filled with arrows. \n"
           "\n"
           "The engine's move is normally printed out on three lines: \n"
           "square to move from, square to move to, and arrow square. \n"
           "If the engine wants to resign, the first line of the output \n"
           "will be the word 'resign'. \n"
           "\n"
           "The fourth line of output, if non-empty, gives the engine's \n"
           "territory counts for itself and its opponent.  These are \n"
           "given only when the territories have been settled (that is, \n"
           "when each square is reachable by at most one player). \n"
           "\n"
           "This program is released under the GPL version 3; \n"
           "see <http://www.gnu.org/licenses/gpl-3.0.html> for the terms. \n",
    argv[0]);
}

