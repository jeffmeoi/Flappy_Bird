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

const int LINES = 35, COLS = 150;		//���������� 
const int startX = 50, startY = 10;		//bird����ʼ���� 
const int dist = 10, spaces = 5;		//distΪˮƽ����ÿ�������ܵ���ˮƽ����,spacesΪ��ֱ�����������ܵ�����ֱ���� 
//dis�������Ϊһ��������,������������ 
const int dis[] = {-1, -1, -1, 0, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 18, 20, 23, 26, 29, 32, 35};
//12��ASCII�ַ���Ϊ��,11��ASCII�ַ���Ϊ����(ע��11��ASCII�ַ�ռ������������), spaceΪ�ո� 
char cBird = 12, cMap = 11, space = ' ';

struct Bird {
	int x;
	int y;
};

pthread_t tBird;		//����bird���߳� 
pthread_t tMap;			//����map���߳� 

Bird bird = {startX, startY};		//��ʼ��������� 

int map[COLS][LINES];				//Ԥ�ڵĵ�ͼ 
int console[COLS][LINES];			//����̨����ʾ�ĵ�ͼ 
int heights[COLS];					//heightsΪ���ܵ��������µĵ�һ���ո��y���� 
int lastHeight = 0, nowHeight = 0;	//lastHeightΪ�����һ���ܵ���height����, nowHeightΪ���ڸùܵ���height���� 
int index = 0, times = 0;			//indexΪ����dis�е��±�;   times���ڼ���, ÿ��ƽ��3��, ���ж�����һ�� 
int money = 0;			//��¼׬�˶���Ǯ 
bool gameOver = false;				//Ĭ����Ϸδ����, ��Ϸ����ʱ��Ϊtrue 


void PrintChar(char *ch,UINT count,UINT x,UINT y) { //������(x,y)������ַ���ch,ch����count���ַ�
	HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos;
	ULONG unuse;
	pos.X=x;
	pos.Y=y;
	CONSOLE_SCREEN_BUFFER_INFO bInfo; // ���ڻ�������Ϣ
	GetConsoleScreenBufferInfo(h, &bInfo );
	WriteConsoleOutputCharacterA(h,ch,count,pos,&unuse);
}

int set_box(int x, int y) {				//��(x, y)�������ַ�cMap					

	PrintChar(&cMap, 1, x, y); 
	console[x][y] = 1;
	
	return 0;
}

int clear_pos(int x, int y) {			//��(x, y)�����ַ���Ϊ�ո� 
	
	PrintChar(&space, 1, x, y);

	return 0;

}

int bird_walk(int y) {				//bird�ƶ�һ�� 

	clear_pos(bird.x, bird.y);
	PrintChar(&cBird, 1, bird.x, y);
	console[bird.x][y] = 2;
	bird.y = y;
	return 0;
}

int getRand(int minn, int maxn) {		//��ȡ[minn, maxn)��Χ�ڵ������ 
	if(maxn >= minn)
		return rand()%(maxn-minn) + minn;
	else
		return 0;
}

int delay (int isBird) {				//�ӳٺ���, ��ֹ��Ϸ���й���, ������Ϸ�ٶ� 
	if(isBird)
		for(long i = 0; i <= pow(12,6); ++i);	//����ֱ�ٶ� 
	else
		for(long i = 0; i <= pow(8,6); ++i);	//��ͼ�����ƶ��ٶ� 
}

int show_map() {		//���µ�ͼ 
	
	for(int i = 0; i < COLS; i++)
		for(int j = 0; j <=25; j++)
			if(map[i][j])
				set_box(i, j);
			else if(console[i][j] == 1)
				clear_pos(i, j);
	return 0;
}

int draw_map() {		//���Ƶ�ͼ���� 
	
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

int map_init() {	//��ͼ��ʼ�� 

	memset(map, 0, sizeof(map));
	memset(heights, 0, sizeof(heights));

	for(int i = 0; i < 150; i++) {
		map[i][26] = cMap;
		if(i%2 == 0)
			set_box(i, 26);
	}
	char * help = "���������ʼ��Ϸ, �ո����Ծ. "; 
	PrintChar(help, strlen(help), 0, 27);
	return 0;
}

int gameover() {		//��Ϸ����ʱִ�� 
	system("cls");
	char cOver[] = "Game Over!";
	char cMoney[COLS];
	sprintf(cMoney, "money:%d!", money);
	PrintChar(cOver, strlen(cOver), (COLS-strlen(cOver))/2, LINES/2);
	PrintChar(cMoney, strlen(cMoney), (COLS-strlen(cMoney))/2, LINES/2 + 1);

	return 0;
}

int Init() {

	system("mode con cols=150 lines=35");		//�̶������д�С 
	SetConsoleTitle("Flappy Bird");				//�����б��� 

	//���������й�� 
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO CursorInfo;
	GetConsoleCursorInfo(handle, &CursorInfo);//��ȡ����̨�����Ϣ
	CursorInfo.bVisible = false; //���ؿ���̨���
	SetConsoleCursorInfo(handle, &CursorInfo);//���ÿ���̨���״̬

	//����������� 
	srand((unsigned)time(NULL));

	//��ʼ����ͼ���� 
	map_init();
	bird_walk(bird.y);
	return 0;
}

//��ͼ�̲߳��� 
void * map_solve(void *a) {

	lastHeight = getRand(3,20);		//�ȼٶ�һ��lastHeight 
	while(true) {
		if(true == gameOver){		//��Ϸ����ʱִ��gameover���� 
			gameover();
			return 0;
		}
		delay(false);
		for(int i = 0; i < COLS-2; i++) {	//����ͼ��ǰ�ƶ� 
			heights[i] = heights[i+1];
		}
		heights[COLS-2] = 0; 
		if(times % dist == 0) {				//ÿ�����ƶ�, ����һ�������ܵ� 
			times = 0;						//��ֹ������Թ�Ϊ0, ��ЧΪtime %= 3; 
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
//�����Ϸ��е�������Ϣ 
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

//���̲߳��� 
void * bird_solve(void *a) {
	index = 0;
	while(true) {
		
		if(true == gameOver){	//�����ڴ˴ν�����Ϸ��������, ��Ϊ��ʱ��ͼδ���Ѿ�ֹͣ�ƶ�, ������������ʧЧ. 
			return 0;
		}
		
		delay(true);
		int newY = bird.y + dis[index];		//���ƶ����µ�y���� 
		if(newY > 25) {
			newY = 25;
			gameOver = true;
			
		}
		if(newY < 0) {
			newY = 0;
			gameOver = true;
			
		}
		if(1 == map[bird.x][bird.y] || 1 == map[bird.x - 1][bird.y]){	//���������������Ϸ����, ֮�����������ж�����Ϊ��������������� 
			gameOver = true;
			continue;
		}
		bird_walk(newY);
		if(heights[bird.x] != 0){		//����������ܵ�,��money++; 
			money++;
		}
		index++;		//������һ���̶ȵ����� 
		if(up())		//����ո��,���¿�ʼ���� 
			index = 0;
	}

	return NULL;
}

int main(int argc, char* argv[]) {

	Init();
	
	while(true) {
		if(kbhit()) {	//�û��κμ���ʼ��Ϸ 
			//�����߳� 
			if(pthread_create(&tMap, NULL, map_solve, NULL) == -1) {
				puts("fail to create pthread t0");
				exit(1);
			}

			if(pthread_create(&tBird, NULL, bird_solve, NULL) == -1) {
				puts("fail to create pthread t1");
				exit(1);
			}

			// �ý��̵ȴ������߳̽���,��ֹ�����˳������߳̽��� 
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

	getchar();	//�س�������Ϸ 

	return 0;
}


