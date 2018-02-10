CC = gcc
CFLAGS = -O2 -fPIC
LDFLAGS = -shared
INC = -I../include -I../Ogre -I../Ogre/include

all: b.so f.so
.PHONY : all

install: all
	cp b.so ../bin/cfMMOC-back.so.1.9.0
	cp f.so ../bin/cfMMOC-fore.so.1.9.0
.PHONY : install

clean:
	rm -f *.o
	rm -f *.so
.PHONY : clean

bload.o: ../src/bload.cpp ../include/bload.h
	$(CC) $(CFLAGS) $(INC) -o bload.o -c ../src/bload.cpp

bcount.o: ../src/bcount.cpp ../include/bcount.h
	$(CC) $(CFLAGS) $(INC) -I../Ogre/Common/include -I../Ogre/Overlay/include -I/usr/include/OIS/ -o bcount.o -c ../src/bcount.cpp

brender.o: ../src/brender.cpp ../include/brender.h bload.o bcount.o
	$(CC) $(CFLAGS) $(INC) -I../Ogre/Common/include -I../Ogre/Overlay/include -I/usr/include/OIS/ -o brender.o -c ../src/brender.cpp

bmain.o: ../src/bso.cpp ../include/brender.h bload.o bcount.o
	$(CC) $(CFLAGS) $(INC) -I../Ogre/Common/include -I../Ogre/Overlay/include -I/usr/include/OIS/ -o bmain.o -c ../src/bso.cpp

b.so: bload.o bcount.o brender.o bmain.o
	cp ../bin/libRQTS.so .
	$(CC) $(CFLAGS) $(LDFLAGS) $(INC) -o b.so bload.o bcount.o brender.o bmain.o libRQTS.so
	rm libRQTS.so

fload.o: ../src/fload.cpp ../include/fload.h
	$(CC) $(CFLAGS) $(INC) -o fload.o -c ../src/fload.cpp

frender.o: ../src/frender.cpp ../include/frender.h fload.o
	$(CC) $(CFLAGS) $(INC) -I../Ogre/Common/include -I../Ogre/Overlay/include -I/usr/include/OIS/ -o frender.o -c ../src/frender.cpp

fmain.o: ../src/fso.cpp ../include/frender.h fload.o
	$(CC) $(CFLAGS) $(INC) -I../Ogre/Common/include -I../Ogre/Overlay/include -I/usr/include/OIS/ -o fmain.o -c ../src/fso.cpp

f.so: fload.o frender.o fmain.o
	cp ../bin/libRQTS.so .
	$(CC) $(CFLAGS) $(LDFLAGS) $(INC) -o f.so fload.o frender.o fmain.o libRQTS.so
	rm libRQTS.so

