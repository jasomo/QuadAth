#include <curses.h>
#include <menu.h>
#include <stdlib.h>
#include <string.h>

#define WHITEONRED 1
#define WHITEONBLUE 2
#define WHITEONBLACK 3
#define BLACKONWHITE 4
#define REDONWHITE 5
#define REDONBLUE 6
#define YELLOWONBLUE 7
#define BLUEONYELLOW 8
#define BLUEONGREEN 9

int getScreenX();
int getScreenY();
void setTitle(WINDOW *pwin, const char * title);
void drawWindowBackground(WINDOW * pwin);
void initColors();
int runMenu(WINDOW *wParent,int height,int width,int y,int x,char *choices[],int lastChoice);
void drawScreen();
void initGUI(int colorStyle);
void endGUI();