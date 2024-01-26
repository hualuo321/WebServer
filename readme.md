# WebServer
用C++实现的高性能WEB服务器，经过webbenchh压力测试可以实现上万的QPS

## 功能
* 利用IO复用技术Epoll与线程池实现多线程的Reactor高并发模型；
* 利用正则与状态机解析HTTP请求报文，实现处理静态资源的请求；
* 利用标准库容器封装char，实现自动增长的缓冲区；
* 基于小根堆实现的定时器，关闭超时的非活动连接；
* 利用单例模式与阻塞队列实现异步的日志系统，记录服务器运行状态；
* 利用RAII机制实现了数据库连接池，减少数据库连接建立与关闭的开销，同时实现了用户注册登录功能。

* 增加logsys,threadpool测试单元(todo: timer, sqlconnpool, httprequest, httpresponse) 

## 环境要求
* Linux
* C++14
* MySql

## 目录树
```
.
├── code           源代码
│   ├── buffer
│   ├── config
│   ├── http
│   ├── log
│   ├── timer
│   ├── pool
│   ├── server
│   └── main.cpp
├── test           单元测试
│   ├── Makefile
│   └── test.cpp
├── resources      静态资源
│   ├── index.html
│   ├── image
│   ├── video
│   ├── js
│   └── css
├── bin            可执行文件
│   └── server
├── log            日志文件
├── webbench-1.5   压力测试
├── build          
│   └── Makefile
├── Makefile
├── LICENSE
└── readme.md
```


## 项目启动
需要先配置好对应的数据库
```bash
// 建立yourdb库
create database yourdb;

// 创建user表
USE yourdb;
CREATE TABLE user(
    username char(50) NULL,
    password char(50) NULL
)ENGINE=InnoDB;

// 添加数据
INSERT INTO user(username, password) VALUES('name', 'password');
```

```bash
make
./bin/server
```

## 单元测试
```bash
cd test
make
./test
```

## 压力测试
![image-webbench](https://github.com/markparticle/WebServer/blob/master/readme.assest/%E5%8E%8B%E5%8A%9B%E6%B5%8B%E8%AF%95.png)
```bash
./webbench-1.5/webbench -c 100 -t 10 http://ip:port/
./webbench-1.5/webbench -c 1000 -t 10 http://ip:port/
./webbench-1.5/webbench -c 5000 -t 10 http://ip:port/
./webbench-1.5/webbench -c 10000 -t 10 http://ip:port/
```
* 测试环境: Ubuntu:19.10 cpu:i5-8400 内存:8G 
* QPS 10000+

## TODO
* config配置
* 完善单元测试
* 实现循环缓冲区

## makefile
```
target: prerequisites ...       # 目标文件: 依赖文件
    command                     #   执行的命令

clean:                          # 用来清除执行文件和中间文件
	rm -rf ...

$@: 目标文件的文件名
$<: 第一个依赖文件名称
$^: 所有依赖文件名称
CXX: 用于编译 C++ 程序的程序；默认 g++
CXXFLAGS: 提供给 C++ 编译器的额外标志
%: 通配符
``` 

## GDB 命令
```
g++ demo.cpp -o demo -g         # 生成带有调试信息的可执行程序
gdb demo                        # 启动 gdb 调试
quit                            # 推出 gdb 调试
list                            # 查看当前文件代码
break n                         # 第 n 行打断点
run                             # 运行, 遇到断点停止
step                            # 向下单步执行, 遇到函数会进入函数体
next                            # 向下执行一行代码, 遇到函数不会进入函数体
continue                        # 执行到下一个断点
print                           # 打印变量信息  
```

## GCC 参数
```
-E: 进行预处理, 生成预处理文件      g++ -E demo.cpp -o demo.i
-S: 进行编译, 得到汇编代码文件      g++ -S demo.i -o demo.s
-c: 进行汇编, 得到目标文件          g++ -c demo.s -o demo.o
链接代码, 生成可执行目标文件        g++ demo1.o demo2.o -o demo -l 库名 -L 库路径 -I 头文件路径
直接生成目标文件, 但保留中间文件    g++ demo.cpp -o demo -save-temps

-o: 指定文件的输出名
-L: 添加链接库的路径
-l: 添加链接库的名称 
-I: 添加头文件的路径
-g: 产生调制信息
-Wall: 显示所有警告信息
-w: 关闭所有警告信息
-O[n]: 编辑器的优化级别
```

## 静态库, 动态库
```
# 当前有 3 个文件, 分别是 demo.cpp, demo.h, main.cpp

# 创建静态库
g++ -c demo.cpp -o demo.o
ar rcs libdemo.a demo.o

# 使用静态库
g++ main.cpp -o main -I 头文件路径 -l 库名称 -L 库路径

# 创建动态库, 得到和位置无关的带吗
g++ -c -fpic demo.cpp -o demo.o
g++ -shared demo.o -o libdemo.so

# 使用动态库
export LD_LIBRARY_PATH = 动态库路径
g++ main.cpp -o main -I 头文件路径 -l 库名称
```

## IO 操作
```
# 通过 open 打开文件
int fd  = open("file.txt", O_RDONLY);
if (fd == -1) perror("open");

# 读写操作
char buf[1024] = {0};
int len = 0;
while ((len = read(fd, buf, sizeof(buf))) > 0) {
    write(destfd, buf, len);
}
```

## 常用库函数
```
# open: 打开路径为 path 的文件, flags 为打开模式, mode 为文件权限
int open(const char *path, int flags, mode_t mode);

# read: 从 fd 中读取数据到 buf, 指定读取 count 字节 (返回已读取的字节数)
ssize_t read(int fd, void *buf, size_t count);

# write: 将 buf 中的数据写入到 fd 中, 指定写入 count 字节 (返回已写入的字节数)
ssize_t write(int fd, const void *buf, size_t count);

# close: 关闭 fd 所指向的文件 (返回时候成功关闭)
int close(int fd);

# fcntl: 对 fd 进行控制操作, 其中可以为复制 fd, 获取状态标识(如阻塞/非阻塞行为)
int fcntl(int fd, int cmd, ... /* arg */);


int fflush(FILE *stream);
```