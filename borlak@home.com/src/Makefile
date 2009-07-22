CC      = gcc
C_FLAGS = -O -Wall -ggdb $(PROF) $(NOCRYPT)
L_FLAGS = -O -lcrypt -lpthread $(PROF)

O_FILES = \
	comm.o		\
	db.o		\
	io.o		\
	main.o		\
	socket.o	\
	util.o

mud: $(O_FILES)
	rm -f mud
	$(CC) $(L_FLAGS) -o mud $(O_FILES)

.c.o: mud.h
	$(CC) -c $(C_FLAGS) $<

clean:
	touch *.c *.h
	make

tar:
	tar czhf ../minimud.tgz *.[ch] Makefile



