# WebServer
閻⑩€�++鐎圭偟骞囬惃鍕彯閹嗗厴WEB閺堝秴濮熼崳顭掔礉缂佸繗绻僿ebbenchh閸樺濮忓ù瀣槸閸欘垯浜掔€圭偟骞囨稉濠佺閻ㄥ嚥PS

## 閸旂喕鍏�
* 閸掆晝鏁O婢跺秶鏁ら幎鈧張鐤巔oll娑撳海鍤庣粙瀣潨鐎圭偟骞囨径姘卞殠缁嬪娈慠eactor妤傛ê鑻熼崣鎴災侀崹瀣剁幢
* 閸掆晝鏁ゅ锝呭灟娑撳海濮搁幀浣规簚鐟欙絾鐎紿TTP鐠囬攱鐪伴幎銉︽瀮閿涘苯鐤勯悳鏉款槱閻炲棝娼ら幀浣界カ濠ф劗娈戠拠閿嬬湴閿涳拷
* 閸掆晝鏁ら弽鍥у櫙鎼存挸顔愰崳銊ョ殱鐟佸崒har閿涘苯鐤勯悳鎷屽殰閸斻劌顤冮梹璺ㄦ畱缂傛挸鍟块崠鐚寸幢
* 閸╄桨绨亸蹇旂壌閸棗鐤勯悳鎵畱鐎规碍妞傞崳顭掔礉閸忔娊妫寸搾鍛閻ㄥ嫰娼ú璇插З鏉╃偞甯撮敍锟�
* 閸掆晝鏁ら崡鏇氱伐濡€崇础娑撳酣妯嗘繅鐐烘Е閸掓鐤勯悳鏉跨磽濮濄儳娈戦弮銉ョ箶缁崵绮洪敍宀冾唶瑜版洘婀囬崝鈥虫珤鏉╂劘顢戦悩鑸碘偓渚婄幢
* 閸掆晝鏁AII閺堝搫鍩楃€圭偟骞囨禍鍡樻殶閹诡喖绨辨潻鐐村复濮圭媴绱濋崙蹇撶毌閺佺増宓佹惔鎾圭箾閹恒儱缂撶粩瀣╃瑢閸忔娊妫撮惃鍕磻闁库偓閿涘苯鎮撻弮璺虹杽閻滈绨￠悽銊﹀煕濞夈劌鍞介惂璇茬秿閸旂喕鍏橀妴锟�

* 婢х偛濮瀕ogsys,threadpool濞村鐦崡鏇炲帗(todo: timer, sqlconnpool, httprequest, httpresponse) 

## 閻滎垰顣ㄧ憰浣圭湴
* Linux
* C++14
* MySql

## 閻╊喖缍嶉弽锟�
```
.
閳规壕鏀㈤埞鈧� code           濠ф劒鍞惍锟�
閳癸拷   閳规壕鏀㈤埞鈧� buffer
閳瑰伄鐘咃拷 閳规壕鏀㈤埞鈧� config
閳瑰伄鐘咃拷 閳规壕鏀㈤埞鈧� http
閳瑰伄鐘咃拷 閳规壕鏀㈤埞鈧� log
閳瑰伄鐘咃拷 閳规壕鏀㈤埞鈧� timer
閳瑰伄鐘咃拷 閳规壕鏀㈤埞鈧� pool
閳瑰伄鐘咃拷 閳规壕鏀㈤埞鈧� server
閳瑰伄鐘咃拷 閳规柡鏀㈤埞鈧� main.cpp
閳规壕鏀㈤埞鈧� test           閸楁洖鍘撳ù瀣槸
閳癸拷   閳规壕鏀㈤埞鈧� Makefile
閳瑰伄鐘咃拷 閳规柡鏀㈤埞鈧� test.cpp
閳规壕鏀㈤埞鈧� resources      闂堟瑦鈧浇绁┃锟�
閳瑰伄鐘咃拷 閳规壕鏀㈤埞鈧� index.html
閳瑰伄鐘咃拷 閳规壕鏀㈤埞鈧� image
閳瑰伄鐘咃拷 閳规壕鏀㈤埞鈧� video
閳瑰伄鐘咃拷 閳规壕鏀㈤埞鈧� js
閳癸拷   閳规柡鏀㈤埞鈧� css
閳规壕鏀㈤埞鈧� bin            閸欘垱澧界悰灞炬瀮娴狅拷
閳癸拷   閳规柡鏀㈤埞鈧� server
閳规壕鏀㈤埞鈧� log            閺冦儱绻旈弬鍥︽
閳规壕鏀㈤埞鈧� webbench-1.5   閸樺濮忓ù瀣槸
閳规壕鏀㈤埞鈧� build          
閳癸拷   閳规柡鏀㈤埞鈧� Makefile
閳规壕鏀㈤埞鈧� Makefile
閳规壕鏀㈤埞鈧� LICENSE
閳规柡鏀㈤埞鈧� readme.md
```


## 妞ゅ湱娲伴崥顖氬З
闂団偓鐟曚礁鍘涢柊宥囩枂婵傝棄顕惔鏃傛畱閺佺増宓佹惔锟�
```bash
// 瀵よ櫣鐝泍ourdb鎼达拷
create database yourdb;

// 閸掓稑缂搖ser鐞涳拷
USE yourdb;
CREATE TABLE user(
    username char(50) NULL,
    password char(50) NULL
)ENGINE=InnoDB;

// 濞ｈ濮為弫鐗堝祦
INSERT INTO user(username, password) VALUES('name', 'password');
```

```bash
make
./bin/server
```

## makefile
```
target: prerequisites ...       # 閻╊喗鐖ｉ弬鍥︽: 娓氭繆绂嗛弬鍥︽
    command                     #   閹笛嗩攽閻ㄥ嫬鎳℃禒锟�

clean:                          # 閻€劍娼靛〒鍛存珟閹笛嗩攽閺傚洣娆㈤崪灞艰厬闂傚瓨鏋冩禒锟�
	rm -rf ...

$@: 閻╊喗鐖ｉ弬鍥︽閻ㄥ嫭鏋冩禒璺烘倳
$<: 缁楊兛绔存稉顏冪贩鐠ф牗鏋冩禒璺烘倳缁夛拷
$^: 閹碘偓閺堝绶风挧鏍ㄦ瀮娴犺泛鎮曠粔锟�
CXX: 閻€劋绨紓鏍槯 C++ 缁嬪绨惃鍕柤鎼村骏绱辨妯款吇 g++
CXXFLAGS: 閹绘劒绶电紒锟� C++ 缂傛牞鐦ч崳銊ф畱妫版繂顦婚弽鍥х箶
%: 闁岸鍘ょ粭锟�
``` 

## GDB 閸涙垝鎶�
```
g++ demo.cpp -o demo -g         # 閻㈢喐鍨氱敮锔芥箒鐠嬪啳鐦穱鈩冧紖閻ㄥ嫬褰查幍褑顢戠粙瀣碍
gdb demo                        # 閸氼垰濮� gdb 鐠嬪啳鐦�
quit                            # 閹恒劌鍤� gdb 鐠嬪啳鐦�
list                            # 閺屻儳婀呰ぐ鎾冲閺傚洣娆㈡禒锝囩垳
break n                         # 缁楋拷 n 鐞涘本澧﹂弬顓犲仯
run                             # 鏉╂劘顢�, 闁洤鍩岄弬顓犲仯閸嬫粍顒�
step                            # 閸氭垳绗呴崡鏇燁劄閹笛嗩攽, 闁洤鍩岄崙鑺ユ殶娴兼俺绻橀崗銉ュ毐閺侀缍�
next                            # 閸氭垳绗呴幍褑顢戞稉鈧悰灞煎敩閻拷, 闁洤鍩岄崙鑺ユ殶娑撳秳绱版潻娑樺弳閸戣姤鏆熸担锟�
continue                        # 閹笛嗩攽閸掗绗呮稉鈧稉顏呮焽閻愶拷
print                           # 閹垫挸宓冮崣姗€鍣烘穱鈩冧紖  
```

## GCC 閸欏倹鏆�
```
-E: 鏉╂稖顢戞０鍕槱閻烇拷, 閻㈢喐鍨氭０鍕槱閻炲棙鏋冩禒锟�      g++ -E demo.cpp -o demo.i
-S: 鏉╂稖顢戠紓鏍槯, 瀵版鍩屽Ч鍥╃椽娴狅絿鐖滈弬鍥︽      g++ -S demo.i -o demo.s
-c: 鏉╂稖顢戝Ч鍥╃椽, 瀵版鍩岄惄顔界垼閺傚洣娆�          g++ -c demo.s -o demo.o
闁剧偓甯存禒锝囩垳, 閻㈢喐鍨氶崣顖涘⒔鐞涘瞼娲伴弽鍥ㄦ瀮娴狅拷        g++ demo1.o demo2.o -o demo -l 鎼存挸鎮� -L 鎼存捁鐭惧锟� -I 婢跺瓨鏋冩禒鎯扮熅瀵帮拷
閻╁瓨甯撮悽鐔稿灇閻╊喗鐖ｉ弬鍥︽, 娴ｅ棔绻氶悾娆庤厬闂傚瓨鏋冩禒锟�    g++ demo.cpp -o demo -save-temps

-o: 閹稿洤鐣鹃弬鍥︽閻ㄥ嫯绶崙鍝勬倳
-L: 濞ｈ濮為柧鐐复鎼存挾娈戠捄顖氱窞
-l: 濞ｈ濮為柧鐐复鎼存挾娈戦崥宥囆� 
-I: 濞ｈ濮炴径瀛樻瀮娴犲墎娈戠捄顖氱窞
-g: 娴溠呮晸鐠嬪啫鍩楁穱鈩冧紖
-Wall: 閺勫墽銇氶幍鈧張澶庮劅閸涘﹣淇婇幁锟�
-w: 閸忔娊妫撮幍鈧張澶庮劅閸涘﹣淇婇幁锟�
-O[n]: 缂傛牞绶崳銊ф畱娴兼ê瀵茬痪褍鍩�
```

## 闂堟瑦鈧礁绨�, 閸斻劍鈧礁绨�
```
# 瑜版挸澧犻張锟� 3 娑擃亝鏋冩禒锟�, 閸掑棗鍩嗛弰锟� demo.cpp, demo.h, main.cpp

# 閸掓稑缂撻棃娆愨偓浣哥氨
g++ -c demo.cpp -o demo.o
ar rcs libdemo.a demo.o

# 娴ｈ法鏁ら棃娆愨偓浣哥氨
g++ main.cpp -o main -I 婢跺瓨鏋冩禒鎯扮熅瀵帮拷 -l 鎼存挸鎮曠粔锟� -L 鎼存捁鐭惧锟�

# 閸掓稑缂撻崝銊︹偓浣哥氨, 瀵版鍩岄崪灞肩秴缂冾喗妫ら崗宕囨畱鐢箑鎮�
g++ -c -fpic demo.cpp -o demo.o
g++ -shared demo.o -o libdemo.so

# 娴ｈ法鏁ら崝銊︹偓浣哥氨
export LD_LIBRARY_PATH = 閸斻劍鈧礁绨辩捄顖氱窞
g++ main.cpp -o main -I 婢跺瓨鏋冩禒鎯扮熅瀵帮拷 -l 鎼存挸鎮曠粔锟�
```

## IO 閹垮秳缍�
```
# 闁俺绻� open 閹垫挸绱戦弬鍥︽
int fd  = open("file.txt", O_RDONLY);
if (fd == -1) perror("open");

# 鐠囪鍟撻幙宥勭稊
char buf[1024] = {0};
int len = 0;
while ((len = read(fd, buf, sizeof(buf))) > 0) {
    write(destfd, buf, len);
}
```

## 鐢摜鏁ゆ惔鎾冲毐閺侊拷
```
# open: 閹垫挸绱戠捄顖氱窞娑擄拷 path 閻ㄥ嫭鏋冩禒锟�, flags 娑撶儤澧﹀鈧Ο鈥崇础, mode 娑撶儤鏋冩禒鑸垫綀闂勶拷
int open(const char *path, int flags, mode_t mode);

# read: 娴狅拷 fd 娑擃叀顕伴崣鏍ㄦ殶閹诡喖鍩� buf, 閹稿洤鐣剧拠璇插絿 count 鐎涙濡� (鏉╂柨娲栧鑼额嚢閸欐牜娈戠€涙濡弫锟�)
ssize_t read(int fd, void *buf, size_t count);

# write: 鐏忥拷 buf 娑擃厾娈戦弫鐗堝祦閸愭瑥鍙嗛崚锟� fd 娑擄拷, 閹稿洤鐣鹃崘娆忓弳 count 鐎涙濡� (鏉╂柨娲栧鎻掑晸閸忋儳娈戠€涙濡弫锟�)
ssize_t write(int fd, const void *buf, size_t count);

# close: 閸忔娊妫� fd 閹碘偓閹稿洤鎮滈惃鍕瀮娴狅拷 (鏉╂柨娲栭弮璺衡偓娆愬灇閸旂喎鍙ч梻锟�)
int close(int fd);

# fcntl: 鐎碉拷 fd 鏉╂稖顢戦幒褍鍩楅幙宥勭稊, 閸忔湹鑵戦崣顖欎簰娑撳搫顦查崚锟� fd, 閼惧嘲褰囬悩鑸碘偓浣圭垼鐠囷拷(婵″倿妯嗘繅锟�/闂堢偤妯嗘繅鐐额攽娑擄拷)
int fcntl(int fd, int cmd, ... /* arg */);

# fopen: 鐠囪褰囬弬鍥︽閸氬秳璐� file 閻ㄥ嫭鏋冩禒锟�, 娴狅拷 mode 濡€崇础閹垫挸绱� (鏉╂柨娲栨稉鈧稉锟� FILE 缂佹挻鐎担鎾村瘹闁斤拷)
FILE *fopen(const char *file, const char *mode);

# fflush: 閸掗攱鏌� FILE 缂傛挸鐡ㄩ崠杞拌厬閻ㄥ嫬鍞寸€圭懓鍩岀拋鎯ь槵
int fflush(FILE *stream);

# access: 检查进程是否有权限访问一个文件
int access(const char *pathname, int mode);
```