CXX = g++									# C++ 缂栬瘧鍣�
CFLAGS = -std=c++14 -O2 -Wall -g 			# C 缂栬瘧鍣ㄩ€夐」
# -std: 鎸囧畾C鏂硅█
# -0: 缂栬瘧鍣ㄤ紭鍖栫骇鍒�
# -Wall: 鏄剧ず鍏ㄩ儴璀﹀憡淇℃伅
# -g: 鐢熸垚璋冭瘯淇℃伅
# -o: 缂栬瘧鎴愬彲鎵ц鏂囦欢

# 鍙橀噺 = 鍙橀噺鍊�
# $<: 绗竴涓緷璧栨枃浠�, $^: 鎵€鏈変緷璧栨枃浠�, $@: 鐩爣鏂囦欢鍚嶇О

TARGET = server
OBJS = ../code/log/*.cpp ../code/pool/*.cpp ../code/timer/*.cpp \
       ../code/http/*.cpp ../code/server/*.cpp \
       ../code/buffer/*.cpp ../code/main.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient

# 娓呴櫎涔嬪墠鐨勫璞℃枃浠跺拰鍙墽琛屾枃浠讹紝浣垮緱浣犲彲浠ヤ粠涓€涓共鍑€鐨勭姸鎬佸紑濮嬫柊鐨勭紪璇戣繃绋�
clean:
	rm -rf ../bin/$(OBJS) $(TARGET)

