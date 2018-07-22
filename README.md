# Flappy Bird

在代码中都有注释详解, 在这里就不做多的解释, 分析一下代码用到的东西.

### 指定位置写入字符

首先是实现指定(x,y)区域写入字符的问题, 起初, 我用了一个gotoxy的函数, 函数如下,

```cpp
void gotoxy(int x, int y)
{
    COORD pos;
    pos.X = x - 1;
    pos.Y = y - 1;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
```

使用这个函数将光标移动至(x,y)处, 再进行写入命令行. 这样做有一个问题就是, 由于bird线程和map线程双线程同时进行. 可能出现, 在某个线程光标移动到某处后, 另一个线程移动了光标, 而前者线程写入字符时, 出现了写入异常的问题. 因此, 不推荐使用.

接着从CSDN上找到如下函数, 并没有移动光标这种操作, 所以在写入时并不会出现异常.

```cpp
void PrintChar(char *ch,UINT count,UINT x,UINT y) { //在坐标(x,y)处输出字符串ch,ch里有count个字符
	HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);		//获取控制台句柄
	COORD pos;
	ULONG unuse;
	pos.X=x;
	pos.Y=y;
	CONSOLE_SCREEN_BUFFER_INFO bInfo; // 窗口缓冲区信息
	GetConsoleScreenBufferInfo(h, &bInfo );
	WriteConsoleOutputCharacterA(h,ch,count,pos,&unuse);
}
```

由于我对win32API并不了解, 并且也不想花时间去了解这些.(毕竟win32没落是趋势)所以如上这些API我并不想去花时间了解弄懂. 所以在这里贴出来, 作为函数供复制粘贴使用.

### 设置命令行大小和标题

```cpp
system("mode con cols=150 lines=35");		//设置命令行大小 
SetConsoleTitle("Flappy Bird");				//命令行标题 
```



### 隐藏光标

```cpp
	//隐藏命令行光标 
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO CursorInfo;
	GetConsoleCursorInfo(handle, &CursorInfo);//获取控制台光标信息
	CursorInfo.bVisible = false; //隐藏控制台光标
	SetConsoleCursorInfo(handle, &CursorInfo);//设置控制台光标状态
```



### 线程

Flappy Bird游戏比起贪吃蛇这类游戏来说, 难度的提升并不大, 基本上只多了一个多线程处理. (其实也可以不用多线程, 但是不用多线程, map的速度和bird的速度就无法分别控制)

```cpp
pthread_t tBird;		//控制bird的线程 
pthread_t tMap;			//控制map的线程 

if(pthread_create(&tMap, NULL, map_solve, NULL) == -1) {
				puts("fail to create pthread t0");
				exit(1);
			}

			if(pthread_create(&tBird, NULL, bird_solve, NULL) == -1) {
				puts("fail to create pthread t1");
				exit(1);
			}

			// 让进程等待其他线程结束,防止进程退出导致线程结束 
			void * result;
			if(pthread_join(tMap, &result) == -1) {
				puts("fail to recollect t0");
				exit(1);
			}

			if(pthread_join(tBird, &result) == -1) {
				puts("fail to recollect t1");
				exit(1);
			}
```

上面的代码是线程的简单应用, 再尚未学习并发编程之时, 我认为只要能够懂得这两个操作即可.

``` 
pthread_create(&tMap, NULL, map_solve, NULL);
```

上述函数是创建线程, 第一个参数为线程的地址, 第三个参数为线程执行的函数.

```cpp
pthread_join(tMap, &result);
```

上述函数是让进程等待线程结束, 防止进程退出导致线程被迫结束.

除此之外, 线程执行的函数有需要的参数, 如下所示, 

```cpp
void * map_solve(void *a){}
```



接着, 分析一下键盘读入按键的问题.

### 键盘按键读入

使用```getch()```读取字符时，读取一次就行, 

而读取方向键和功能键是，需要读取两次.  （第一次的返回值为0或者224（方向键））

```getch()```函数在读取一个功能键或者箭头（方向）键盘时，函数会返回两次，第一次调用返回0或者0xE0，第二次调用返回实际的键值。  

所以在读入字符时候, 要明确清楚, 到底是要读入字符还是要读入功能键和方向机, 由此判定是读取一次或是两次.或者也可以做一个判断, 因为0和224都在ASCII范围之外, 可以分出两类分别处理.

另外说明一下, 本人实测, 当输出为□时, 编码占据两格, 虽然不知道为什么, 但是在处理时, 需要把其当做中文字符的宽度处理.

**0:30躺在床上突然想起自己还没更新README, 爬起来继续刚. **

**接下来准备要找一点课程学习一波网络编程知识, 也差不多准备退水C语言, 入驻Java/C++/Python, 我的考虑是Java, 毕竟书<Java编程思想>在身边, 233**

------

7/22更新,

***鉴于, 准备上手web端, 所以还是先上手Python比较划算, 书JavaScript大犀牛也在我身边, 或者直接上手node.js***