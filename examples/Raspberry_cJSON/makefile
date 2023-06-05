SHELL=/bin/sh

CC := g++

CFLAGS=-DRASPBERRY_PI -std=c++14

# cJSON
INCLUDES += -I/usr/include/cjson/
CXXFLAGS += -lcjson

# cURL
CXXFLAGS += -lcurl

# raspjson
all: raspjson

# ===== Compile
LibTeleinfo.o: ../../src/LibTeleinfo.cpp
	$(CC) $(CFLAGS)  -c ../../src/LibTeleinfo.cpp
  
raspjson.o: raspjson.cpp
	$(CC) $(CFLAGS)  -c raspjson.cpp $(CXXFLAGS) $(INCLUDES)

# ===== Link
raspjson: raspjson.o LibTeleinfo.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o raspjson raspjson.o LibTeleinfo.o $(CXXFLAGS)

clean: 
	rm -f *.o raspjson 
  
