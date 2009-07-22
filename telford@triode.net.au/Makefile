sixteen: sixteen.c
	gcc -O2 -fomit-frame-pointer -o $@ $^ -lm -lgdbm

report: sixteen.c Makefile
	./16k.py -c -l -b $^ | less
