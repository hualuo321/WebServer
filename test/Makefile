CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g

TARGET = run_test
OBJS = ../code/log/*.cpp ../code/pool/*.cpp ../code/timer/*.cpp \
       ../code/http/*.cpp ../code/server/*.cpp \
       ../code/buffer/*.cpp ../test/test.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../build/$(TARGET)  -pthread -l mysqlclient

clean:
	rm -rf ../build/$(OBJS) $(TARGET)