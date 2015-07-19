SHELL=/bin/sh

CFLAGS=-DRASPBERRY_PI

# raspjson
all: raspjson

# ===== Compile
LibTeleinfo.o: ../../LibTeleinfo.cpp
	$(CC) $(CFLAGS)  -c ../../LibTeleinfo.cpp
  
raspjson.o: raspjson.cpp
	$(CC) $(CFLAGS)  -c raspjson.cpp

# ===== Link
raspjson: raspjson.o LibTeleinfo.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o raspjson raspjson.o LibTeleinfo.o 

clean: 
	rm -f *.o raspjson 
  