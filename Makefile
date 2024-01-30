CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g

TARGET = run_server
OBJS = ./code/log/*.cpp ./code/pool/*.cpp ./code/timer/*.cpp \
       ./code/http/*.cpp ./code/server/*.cpp \
       ./code/buffer/*.cpp ./code/main.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ./code/$(TARGET) -pthread -l mysqlclient

clean:
	rm -rf ./code/$(TARGET)

# -Wall: 显示所有警告
# -g: 提供调试信息
# -l: 提供库名
# -L: 提供库路径
# -I: 提供头文件路径