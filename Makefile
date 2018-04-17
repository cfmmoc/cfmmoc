CC = gcc
CFLAGS = -O2 -fPIC
LDFLAGS = -shared
INC = -I../include -I../Ogre -I../Ogre/include

install: all
	cp b.so ../bin/cfMMOC-back.so.1.9.0
	cp f.so ../bin/cfMMOC-fore.so.1.9.0
	cp browser ../bin/cfMMOC
.PHONY : install

all: b.so f.so browser
.PHONY : all

clean:
	rm -f *.o
	rm -f *.so
	rm -f browser
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
	$(CC) $(CFLAGS) $(LDFLAGS) $(INC) -o b.so bload.o bcount.o brender.o bmain.o ../bin/libRQTS.so

fload.o: ../src/fload.cpp ../include/fload.h
	$(CC) $(CFLAGS) $(INC) -o fload.o -c ../src/fload.cpp

frender.o: ../src/frender.cpp ../include/frender.h fload.o
	$(CC) $(CFLAGS) $(INC) -I../Ogre/Common/include -I../Ogre/Overlay/include -I/usr/include/OIS/ -o frender.o -c ../src/frender.cpp

fmain.o: ../src/fso.cpp ../include/frender.h fload.o
	$(CC) $(CFLAGS) $(INC) -I../Ogre/Common/include -I../Ogre/Overlay/include -I/usr/include/OIS/ -o fmain.o -c ../src/fso.cpp

f.so: fload.o frender.o fmain.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(INC) -o f.so fload.o frender.o fmain.o ../bin/libRQTS.so

curl.o: ../browser/cURLArchive.cpp ../browser/cURLArchive.h
	$(CC) $(CFLAGS) $(INC) -o curl.o -c ../browser/cURLArchive.cpp 

brow.o: ../browser/SampleBrowser.cpp ../browser/SampleBrowser.h ../browser/SimpleTextB.h
	$(CC) $(CFLAGS) $(INC) -I../Ogre/Common/include -I../Ogre/Overlay/include -I/usr/include/OIS/ -I../browser -o brow.o -c ../browser/SampleBrowser.cpp 

browser: curl.o brow.o
	$(CC) $(CFLAGS) $(INC) -o browser curl.o brow.o ../bin/libOgreMain.so.1.9.0 ../bin/libOgreOverlay.so.1.9.0 -lstdc++ -lm -lOIS -lcurl
