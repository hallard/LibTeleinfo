SHELL=/bin/sh

CC := g++

CFLAGS=-DRASPBERRY_PI -std=c++14

# cJSON
INCLUDES += -I/usr/include/cjson/
CXXFLAGS += -lcjson

# cURL
CXXFLAGS += -lcurl

# raspjson
all: raspjson_standard

# ===== Compile
LibTeleinfo.o: ../../src/LibTeleinfo.cpp
	$(CC) $(CFLAGS)  -c ../../src/LibTeleinfo.cpp
  
raspjson.o: raspjson.cpp
	$(CC) $(CFLAGS)  -c raspjson.cpp $(CXXFLAGS) $(INCLUDES)

raspjson_stand.o: raspjson_stand.cpp
	$(CC) $(CFLAGS)  -c raspjson_stand.cpp $(CXXFLAGS) $(INCLUDES)

raspjson_mqtt.o: raspjson_mqtt.cpp
	$(CC) $(CFLAGS) -g -c raspjson_mqtt.cpp $(CXXFLAGS) $(INCLUDES) -lmosquitto

# ===== Link
raspjson: raspjson.o LibTeleinfo.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o raspjson raspjson.o LibTeleinfo.o $(CXXFLAGS)

raspjson_standard: raspjson_stand.o LibTeleinfo.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o raspjson_standard raspjson_stand.o LibTeleinfo.o $(CXXFLAGS)

raspjson_mqtt: raspjson_mqtt.o LibTeleinfo.o
	$(CC) $(CFLAGS) $(LDFLAGS) -g -o raspjson_mqtt raspjson_mqtt.o LibTeleinfo.o $(CXXFLAGS) -lmosquitto

clean: 
	rm -f *.o raspjson raspjson_standard raspjson_mqtt
  
