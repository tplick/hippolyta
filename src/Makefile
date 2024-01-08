default:
	gcc -Wall -O3 main.c -o hippolyta && strip hippolyta

alt:
	gcc -Wall -O3 -DALT main.c -o alt_hippolyta && strip alt_hippolyta

win:
	i586-mingw32msvc-gcc -Wl,--stack,4194304 -O3 main.c -o hippolyta.exe && i586-mingw32msvc-strip hippolyta.exe

