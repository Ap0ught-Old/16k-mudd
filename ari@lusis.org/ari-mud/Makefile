CXX = c++

all:	mud

mud:	mud.o
	$(CXX) -w -g -o mud mud.o

mud.o:	mud.cpp
	$(CXX) -w -g -c mud.cpp

clean:
	-rm -f core mud.o mud

