default:
	cd src && make

alt:
	cd src && make alt

win:
	cd src && make win

all: default alt win
