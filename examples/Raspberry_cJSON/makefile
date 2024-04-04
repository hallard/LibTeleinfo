SHELL=/bin/sh

CC := g++

CFLAGS=-DRASPBERRY_PI -std=c++14

# cJSON
INCLUDES += -I/usr/include/cjson/
CXXFLAGS += -lcjson

# cURL
CXXFLAGS += -lcurl

# spdlog
#INCLUDES += -I/home/pi/GIT/spdlog/include
#CXXFLAGS += -pthread -lspdlog -DSPDLOG_COMPILED_LIB
#LDFLAGS	 += -L/home/pi/GIT/spdlog/build -lspdlog
INCLUDES += -I/usr/local/include/spdlog/
CXXFLAGS += -pthread -lspdlog -DSPDLOG_COMPILED_LIB
LDFLAGS	 += -L/usr/local/lib/ -lspdlog


# raspjson
all: raspjson_standard

# ===== Compile
LibTeleinfo.o: ../../src/LibTeleinfo.cpp
	$(CC) $(CFLAGS) -c ../../src/LibTeleinfo.cpp
  
raspjson.o: raspjson.cpp
	$(CC) $(CFLAGS) -c raspjson.cpp $(INCLUDES) $(CXXFLAGS) 

raspjson_stand.o: raspjson_stand.cpp
	$(CC) $(CFLAGS) -c raspjson_stand.cpp $(INCLUDES) $(CXXFLAGS)

raspjson_mqtt.o: raspjson_mqtt.cpp
	$(CC) $(CFLAGS) -g -c raspjson_mqtt.cpp $(INCLUDES) $(CXXFLAGS) -lmosquitto

# ===== Link
raspjson: raspjson.o LibTeleinfo.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o raspjson raspjson.o LibTeleinfo.o $(CXXFLAGS)

raspjson_standard: raspjson_stand.o LibTeleinfo.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o raspjson_standard raspjson_stand.o LibTeleinfo.o $(CXXFLAGS)

raspjson_mqtt: raspjson_mqtt.o LibTeleinfo.o
	$(CC) $(CFLAGS) $(LDFLAGS) -g -o raspjson_mqtt raspjson_mqtt.o LibTeleinfo.o $(CXXFLAGS) -lmosquitto

clean: 
	rm -f *.o raspjson raspjson_standard raspjson_mqtt
  
