#!/usr/bin/env python3

import sys, subprocess, time

sec = sys.argv[1]
cmdA = sys.argv[2]
cmdB = sys.argv[3]

def convert(s):
    col, row = s[0], s[1:]
    return 'abcdefghij'.index(col) + 10 * (int(row) - 1)

def print_board(board):
    for i in range(10):
        for j in range(10):
            sys.stdout.write({'0': '.', '3': 'X', '1': '1', '2': '2'}[board[10*i+j]])
        sys.stdout.write('\n')
    sys.stdout.flush()

def play_one():
  board = list("0001001000000000000000000000001000000001000000000000000000002000000002000000000000000000000002002000")

  while True:
    toMove = 1 if board.count('3') % 2 == 0 else 2
    cmd = [0, cmdA, cmdB][toMove]

    start_time = time.time()
    output = subprocess.check_output([cmd, "-s", sec, "".join(board)]).decode('utf8')
    end_time = time.time()
    src, dst, target, rest = output.split("\n", 3)

    if src == dst:
        print("%d wins." % (toMove ^ 3))
        winners[toMove^3] += 1
        break

    if 'resign' in src:
        print("%d resigns" % toMove)
        print("%d wins." % (toMove ^ 3))
        winners[toMove^3] += 1
        break

    a = convert(src)
    b = convert(dst)
    c = convert(target)    
    board[b] = board[a]
    board[a] = '0'
    board[c] = '3'

    print("%d moves %s-%s/%s  (took %0.2f sec)" % (toMove, src, dst, target, end_time - start_time))
    print_board(board)

winners = {1: 0, 2: 0}


while True:
    play_one()
    print("*** After %d, the first player has won %d, the second player %d." % (
                sum(winners.values()), winners[1], winners[2]))

