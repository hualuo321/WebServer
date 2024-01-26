CXX = g++									# C++ 编译器
CFLAGS = -std=c++14 -O2 -Wall -g 			# C 编译器选项
# -std: 指定C方言
# -0: 编译器优化级别
# -Wall: 显示全部警告信息
# -g: 生成调试信息
# -o: 编译成可执行文件

# 变量 = 变量值
# $<: 第一个依赖文件, $^: 所有依赖文件, $@: 目标文件名称

TARGET = server
OBJS = ../code/log/*.cpp ../code/pool/*.cpp ../code/timer/*.cpp \
       ../code/http/*.cpp ../code/server/*.cpp \
       ../code/buffer/*.cpp ../code/main.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient

# 清除之前的对象文件和可执行文件，使得你可以从一个干净的状态开始新的编译过程
clean:
	rm -rf ../bin/$(OBJS) $(TARGET)

