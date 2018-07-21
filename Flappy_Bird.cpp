#include<cstdio>
#include<cstring>
#include<cmath>
#include<ctime>
#include<cstdlib>
#include<iostream>
#include<unistd.h>
#include<pthread.h>
#include<windows.h>
#include<conio.h>

using namespace std;

const int LINES = 35, COLS = 150;		//行数和列数 
const int startX = 50, startY = 10;		//bird的起始坐标 
const int dist = 10, spaces = 5;		//dist为水平方向每两条竖管道的水平距离,spaces为竖直方向上两条管道的竖直距离 
//dis数组近似为一个抛物线,即上升又下落 
const int dis[] = {-1, -1, -1, 0, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 18, 20, 23, 26, 29, 32, 35};
//12的ASCII字符作为鸟,11的ASCII字符作为方格(注意11的ASCII字符占了两个横坐标), space为空格 
char cBird = 12, cMap = 11, space = ' ';

struct Bird {
	int x;
	int y;
};

pthread_t tBird;		//控制bird的线程 
pthread_t tMap;			//控制map的线程 

Bird bird = {startX, startY};		//初始化鸟的坐标 

int map[COLS][LINES];				//预期的地图 
int console[COLS][LINES];			//控制台中显示的地图 
int heights[COLS];					//heights为竖管道从上往下的第一个空格的y坐标 
int lastHeight = 0, nowHeight = 0;	//lastHeight为左边上一个管道的height数据, nowHeight为现在该管道的height数据 
int index = 0, times = 0;			//index为数组dis中的下标;   times用于计算, 每左平移3次, 最有段新增一列 
int money = 0;			//记录赚了多少钱 
bool gameOver = false;				//默认游戏未结束, 游戏结束时改为true 


void PrintChar(char *ch,UINT count,UINT x,UINT y) { //在坐标(x,y)处输出字符串ch,ch里有count个字符
	HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos;
	ULONG unuse;
	pos.X=x;
	pos.Y=y;
	CONSOLE_SCREEN_BUFFER_INFO bInfo; // 窗口缓冲区信息
	GetConsoleScreenBufferInfo(h, &bInfo );
	WriteConsoleOutputCharacterA(h,ch,count,pos,&unuse);
}

int set_box(int x, int y) {				//在(x, y)处放置字符cMap					

	PrintChar(&cMap, 1, x, y); 
	console[x][y] = 1;
	
	return 0;
}

int clear_pos(int x, int y) {			//在(x, y)处将字符改为空格 
	
	PrintChar(&space, 1, x, y);

	return 0;

}

int bird_walk(int y) {				//bird移动一步 

	clear_pos(bird.x, bird.y);
	PrintChar(&cBird, 1, bird.x, y);
	console[bird.x][y] = 2;
	bird.y = y;
	return 0;
}

int getRand(int minn, int maxn) {		//获取[minn, maxn)范围内的随机数 
	if(maxn >= minn)
		return rand()%(maxn-minn) + minn;
	else
		return 0;
}

int delay (int isBird) {				//延迟函数, 防止游戏进行过快, 控制游戏速度 
	if(isBird)
		for(long i = 0; i <= pow(12,6); ++i);	//鸟竖直速度 
	else
		for(long i = 0; i <= pow(8,6); ++i);	//地图向左移动速度 
}

int show_map() {		//更新地图 
	
	for(int i = 0; i < COLS; i++)
		for(int j = 0; j <=25; j++)
			if(map[i][j])
				set_box(i, j);
			else if(console[i][j] == 1)
				clear_pos(i, j);
	return 0;
}

int draw_map() {		//重制地图数组 
	
	for(int i = 0; i < COLS; i++)
		for(int j = 0; j <=25; j++){
			map[i][j] = 0;
		}
	
	for(int i = 0; i < COLS; i++) {
		for(int j = 0; j <= 25; j++) {
			if(heights[i] && (j < heights[i] || j > heights[i] + spaces))
				map[i][j] = 1;
		}
	}
	
}

int map_init() {	//地图初始化 

	memset(map, 0, sizeof(map));
	memset(heights, 0, sizeof(heights));

	for(int i = 0; i < 150; i++) {
		map[i][26] = cMap;
		if(i%2 == 0)
			set_box(i, 26);
	}
	char * help = "按任意键开始游戏, 空格键跳跃. "; 
	PrintChar(help, strlen(help), 0, 27);
	return 0;
}

int gameover() {		//游戏结束时执行 
	system("cls");
	char cOver[] = "Game Over!";
	char cMoney[COLS];
	sprintf(cMoney, "money:%d!", money);
	PrintChar(cOver, strlen(cOver), (COLS-strlen(cOver))/2, LINES/2);
	PrintChar(cMoney, strlen(cMoney), (COLS-strlen(cMoney))/2, LINES/2 + 1);

	return 0;
}

int Init() {

	system("mode con cols=150 lines=35");		//固定命令行大小 
	SetConsoleTitle("Flappy Bird");				//命令行标题 

	//隐藏命令行光标 
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO CursorInfo;
	GetConsoleCursorInfo(handle, &CursorInfo);//获取控制台光标信息
	CursorInfo.bVisible = false; //隐藏控制台光标
	SetConsoleCursorInfo(handle, &CursorInfo);//设置控制台光标状态

	//撒随机数种子 
	srand((unsigned)time(NULL));

	//初始化地图和鸟 
	map_init();
	bird_walk(bird.y);
	return 0;
}

//地图线程操作 
void * map_solve(void *a) {

	lastHeight = getRand(3,20);		//先假定一个lastHeight 
	while(true) {
		if(true == gameOver){		//游戏结束时执行gameover函数 
			gameover();
			return 0;
		}
		delay(false);
		for(int i = 0; i < COLS-2; i++) {	//将地图向前移动 
			heights[i] = heights[i+1];
		}
		heights[COLS-2] = 0; 
		if(times % dist == 0) {				//每三次移动, 生成一整条竖管道 
			times = 0;						//防止溢出所以归为0, 等效为time %= 3; 
			do {
				nowHeight = lastHeight + getRand(-5, 5);
			} while(nowHeight < spaces || nowHeight > 25-spaces*2);
			heights[COLS-2] = nowHeight;
			lastHeight = nowHeight;
		}
		times++;
		draw_map();
		show_map();

	}

	return NULL;
}
//鸟向上飞行的输入信息 
int up() {
	int ch;
	if(kbhit()) {
		ch = getch();
		if(ch == ' ') {
			return 1;
		}
	}
	return 0;
}

//鸟线程操作 
void * bird_solve(void *a) {
	index = 0;
	while(true) {
		
		if(true == gameOver){	//不能在此次进行游戏结束函数, 因为此时地图未必已经停止移动, 清屏函数可能失效. 
			return 0;
		}
		
		delay(true);
		int newY = bird.y + dis[index];		//鸟移动后新的y坐标 
		if(newY > 25) {
			newY = 25;
			gameOver = true;
			
		}
		if(newY < 0) {
			newY = 0;
			gameOver = true;
			
		}
		if(1 == map[bird.x][bird.y] || 1 == map[bird.x - 1][bird.y]){	//如果鸟遇到方格游戏结束, 之所以有两个判断是因为方格有两个坐标宽 
			gameOver = true;
			continue;
		}
		bird_walk(newY);
		if(heights[bird.x] != 0){		//如果遇到竖管道,则money++; 
			money++;
		}
		index++;		//进行下一个程度的升降 
		if(up())		//点击空格后,重新开始升降 
			index = 0;
	}

	return NULL;
}

int main(int argc, char* argv[]) {

	Init();
	
	while(true) {
		if(kbhit()) {	//敲击任何键后开始游戏 
			//创建线程 
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
			break;
		}
	}

	getchar();	//回车结束游戏 

	return 0;
}


