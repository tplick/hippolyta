# go.py: LittleGolem interface code
# Copyright Tom Plick 2010, 2014.
# This file is released under the GPL version 3; see LICENSE.


import urllib2, LOGIN, re, subprocess, time
import thread, traceback, urllib


import socket
socket.setdefaulttimeout(60)

def loadPage(url):
    if not url.startswith("http://"):
        url = "http://www.littlegolem.net/jsp/" + url
    print "LOADING URL", url
    req = urllib2.Request(url.replace(" ", "+"), headers = {"Cookie" : LOGIN.Cookie})
    return urllib2.urlopen(req).read()

def gamesWaiting():
    text = loadPage("game/index.jsp")
    # print text
    off1 = text.find("</i>On Move")
    off2 = text.find("</i>Opponent's Move")
    text = text[off1:off2]

    print re.findall(r"game.jsp\?gid=\d+", text)
    for s in re.findall(r"game.jsp\?gid=\d+", text):
        gameid = re.findall(r"\d+", s)[0]
        yield gameid

def handleGame(gameid):
    print "Handling game %s" % gameid

    text = loadPage("game/game.jsp?gid=" + gameid)
    titleoff = text.find("<title>")
    if text[titleoff:].startswith("<title>Shogi"):
        handleMiniShogiGame(gameid, text)
        return

    if re.search("""<h3 class="page-title">\s*Four in a row-Size 8x8""", text):
        four = True
    else:
        four = False

    L = []
    #text = text[text.find("<!--board-->") : text.find("<!--end game part-->")]
    if four:
        handleFourGame(gameid, text)
        return

    for s in re.findall(r"alt='\w*'", text):
        color = re.search(r"alt='(\w*)'", s).group(1)
        val = {"" : '0', "white" : '1', "black" : '2', "W" : '2', "B" : '1', "S" : '3'}[color]
        L.append(val)

    if len(L) == 64:
        handleBreakthroughGame(gameid, text, L)
    elif len(L) == 100:
        handleAmazonsGame(gameid, text, L)
    elif len(L) == 361:
        handleConnect6Game(gameid, text, L)
    elif len(L) == 81:
        handleSlitherGame(gameid, text, L)
    print L

def handleFourGame(gameid, text):
    L = re.findall("[byr]x?\.gif", text)
    turn = '1'

    # change to decent values
    for i, im in enumerate(L):
        if im.startswith("y"):
            L[i] = '1'
            if 'x' in im: turn = '2'
        elif im.startswith("r"):
            L[i] = '2'
            if 'x' in im: turn = '1'
        else:
            L[i] = ''

    # group into rows
    Rows = []
    for i in xrange(0, 64, 8):
        Rows.append(L[i:i+8])
    Rows = Rows[::-1]

    # change to columns
    Columns = zip(*Rows)
    s = "/".join("".join(c) for c in Columns)
    turn = '1' if s.count('1') == s.count('2') else '2'
    s = s.replace(turn, 'X')
    s = s.replace('1', 'O').replace('2', 'O')

    print "fir game: %s" % s

    move = ""
    proc = subprocess.Popen(["./four", s], stdout=subprocess.PIPE)
    for line in proc.stdout:
        if line[:1] in ("1", "2", "3", "4", "5", "6", "7", "8"):
            move = line[:1]
    loadPage("game/game.jsp?sendgame=%s&sendmove=%s" % (gameid, move))


def handleBreakthroughGame(gameid, text, L):
    # group into rows
    Rows = []
    for i in xrange(0, 64, 8):
        Rows.append(L[i:i+8])
    Rows = Rows[::-1]

    # if we are playing black, flip the rows AND each row
    if text.find("<b>h</b>") < text.find("<b>g</b>"):
        Rows = Rows[::-1]
        for i in xrange(8):
            Rows[i] = Rows[i][::-1]
        mycolor = "2"
    else:
        mycolor = "1"

    print Rows

    Rows = reduce(lambda x,y:x+y, Rows)
    proc = subprocess.Popen(["./break", mycolor, "".join(Rows)], stdout = subprocess.PIPE)
    line = proc.stdout.readline().strip()
    src, dst = [int(x) for x in line.split(" ")]

    textmove = squareCode(src) + squareCode(dst)
    loadPage("game/game.jsp?sendgame=%s&sendmove=%s" % (gameid, textmove))

def squareCode(x):
    return str(x%8) + str(x//8)
def amazonsSquareCode(sq):
    sq = sq.lower()

    if "10" in sq:
        sq = sq.replace("10", "9")
    else:
        for i in xrange(10):
            sq = sq.replace(str(i), str(i-1))

    for i, ch in enumerate("abcdefghij"):
        sq = sq.replace(ch, str(i))
    return sq

def handleAmazonsGame(gameid, text, L):
    # group into rows
    Rows = []
    for i in xrange(0, 100, 10):
        Rows.append(L[i:i+10])
    Rows = Rows[::-1]
    L = reduce(lambda x,y:x+y, Rows)
    
    string = "".join(L)
    string = string.replace("1", "4").replace("2", "1").replace("4", "2")

    proc = subprocess.Popen(["./hippolyta", string], stdout = subprocess.PIPE)
    def line(): return proc.stdout.readline().strip()
    src = line()
    if "resign" in src:
        loadPage("game/game.jsp?sendgame=%s&sendmove=resign" % (gameid))
        return
    dst = line()
    target = line()
    balance = line()

    if balance:
        me, you = balance.split(" ")
        message = "&message=I count %s territory for me versus %s for you." % (me, you)
    elif string.count('3') < 2:
        version = subprocess.check_output(["./hippolyta", "-v"]).strip()
        message = "&message=greetings from %s" % urllib.quote(version)
    else:
        message = ""

    print "amazons moving ", src, dst, target
    src, dst, target = map(amazonsSquareCode, [src, dst, target])
    textmove = src + dst + target
    print "amazons moving ", textmove
    loadPage("game/game.jsp?sendgame=%s&sendmove=%s%s" % (gameid, textmove, message))


def handleSlitherGame(gameid, text, L):
    # group into rows
    Rows = []
    for i in xrange(0, 81, 9):
        Rows.append(L[i:i+9])
    Rows = Rows[::-1]
    L = reduce(lambda x,y:x+y, Rows)

    L = "".join(L).replace("1", "+").replace("2", "-")
    if L.count("+") == L.count("-"):
        turn = "1"
    else:
        turn = "-1"

    proc = subprocess.Popen(["./slither", turn, L], stdout = subprocess.PIPE)
    def line(): return proc.stdout.readline().strip()
    move = line()
    if move == "swap":
        move = line()
        loadPage("game/game.jsp?sendgame=%s&sendmove=swap" % (gameid))

    # replace the coords
    for i in xrange(13, 0, -1):
        move = move.replace(str(i), " abcdefghijklmnop"[i])
    loadPage("game/game.jsp?sendgame=%s&sendmove=%s" % (gameid, move))


def handleConnect6Game(gameid, text, L):
    s = "".join(L)
    print "c6 game:", s
    turn = "1" if s.count("1") < s.count("2") else "2"
    proc = subprocess.Popen(["./six", turn, s], stdout = subprocess.PIPE)
    move = proc.stdout.readline().strip()
    print "sending move", move
    loadPage("game/game.jsp?sendgame=%s&sendmove=%s" % (gameid, move))



ShogiPieceMap = {'00': 'K', '01': 'R', '02': 'B', '03': 'G', '04': 'S', '07': 'P',
                 '10': 'k', '11': 'r', '12': 'b', '13': 'g', '14': 's', '17': 'p',
                 '20': 'K', '21': '+R', '22': '+B', '24': '+S', '27': '+P',
                 '30': 'k', '31': '+r', '32': '+b', '34': '+s', '37': '+p'}

LetterFlip = {'a':'e', 'b':'d', 'c':'c', 'd':'b', 'e':'a'}

def handleMiniShogiGame(gameid, text):
    D = {}
    flip = False
    exp = r"""left: (\d+)px;top: (\d+)px;background-image: url\("/images/shogi/shogi(\d+).png"\)"""
    for (left, top, index) in re.findall(exp, text):
        col = ["20", "67", "115", "162", "210"].index(left)
        row = ['20', '70', '120', '170', '220'].index(top)
        piece = ShogiPieceMap[index]
        D[4-row, col] = piece

        if index == '20':
            flip = True

    s = ""
    for r in xrange(5):
        for c in xrange(5):
            s += D.get((r,c), "0")

    exp = r"""left:\d+;top:\d+;background-image: url\("/images/shogi/shogi(\d+).png"\)"""
    for index in re.findall(exp, text):
        piece = ShogiPieceMap[index]
        s += piece

    print "shogi position: %s" % s

    proc = subprocess.Popen(["./minishogi", '1', s], stdout = subprocess.PIPE)
    move = proc.stdout.readline().strip()
    if flip:
        move2 = ""
        for ch in move:
            move2 += LetterFlip.get(ch, ch)
    else:
        move2 = move
    print "sending move", move2
    loadPage("game/game.jsp?sendgame=%s&sendmove=%s" % (gameid, move2))




def answerInvitation():
    text = loadPage("invitation/index.jsp")
    if "Refuse</a>" not in text:
        return

    text = text[text.find("<td>Your decision</td>") : text.find("Refuse</a>")]
    invid = re.search(r'invid=(\d+)">Accept</a>', text).group(1)
    print "INVITATION", invid

    if   "<td>Breakthrough :: Size 8</td>" in text:
        response = "refuse"
    elif "<td>Amazons :: Size 10</td>" in text:
        response = "accept"
    elif "<td>Connect6 :: Connect6</td>" in text:
        response = "refuse"
    elif "<td>Shogi :: Mini Shogi</td>" in text:
        response = "refuse"
    elif "<td>Four in a row :: Size 8x8</td>" in text:
        response = "refuse"
    else:
        response = "refuse"

    loadPage("http://www.littlegolem.net/ng/a/Invitation.action?%s=&invid=%s" % (response, invid))



def main():
    end = time.time() + 55

    extraSleep = 240 # 600 # 60 # 300
    try:
        for gameid in gamesWaiting():
            handleGame(gameid)
            extraSleep = 0
        answerInvitation()
    except Exception:
        traceback.print_exc()
        extraSleep = 240 # 600 # 60 # 300
        pass

    end += extraSleep
    while time.time() < end:
        time.sleep(5)

def quitAfterAWhile():
    time.sleep(6000)
    sys.exit(0)

thread.start_new_thread(quitAfterAWhile, ())
main()

