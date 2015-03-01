#include "ESC.h"

#define GPIO			17
#define CHANNEL			0
#define SUBCYCLE_TIME	SUBCYCLE_TIME_US_DEFAULT // 20000 (20ms)

#define LOW_RANGE	70
#define	HIGH_RANGE	200

#define SIMULATION	0	// 1-NO  0-YES

int setWidth(int *width, int num)
{
	*width=num;
	if (*width > HIGH_RANGE)
	{
		*width=HIGH_RANGE;
	}
	if (*width < LOW_RANGE)
	{
		*width=LOW_RANGE;
	}
	SIMULATION && add_channel_pulse(CHANNEL, GPIO, 0, *width);
}

int incWidth(int *width, int num)
{
	*width+=num;
	if (*width > HIGH_RANGE)
	{
		*width=HIGH_RANGE;
	}
	if (*width < LOW_RANGE)
	{
		*width=LOW_RANGE;
	}
	SIMULATION && add_channel_pulse(CHANNEL, GPIO, 0, *width);
}

int decWidth(int *width, int num)
{
	*width-=num;
	if (*width > HIGH_RANGE)
	{
		*width=HIGH_RANGE;
	}
	if (*width < LOW_RANGE)
	{
		*width=LOW_RANGE;
	}
	SIMULATION && add_channel_pulse(CHANNEL, GPIO, 0, *width);
}

int quit()
{
	SIMULATION && clear_channel_gpio(CHANNEL, GPIO);
	shutdown();
}

int calculatePercent(int pulse)
{
	return (100*(pulse-LOW_RANGE)/(HIGH_RANGE-LOW_RANGE));
}

void printBox(int pulse)
{
	int boxSize;
	int i;
	boxSize=(30*(pulse-LOW_RANGE)/(HIGH_RANGE-LOW_RANGE));
	
	
	
	wattrset(stdscr, COLOR_PAIR(WHITEONRED));
	if (boxSize<21) wattrset(stdscr, COLOR_PAIR(BLUEONYELLOW));
	if (boxSize<11) wattrset(stdscr, COLOR_PAIR(BLUEONGREEN));
	
	wmove(stdscr,5,10);
	for (i=0;i<boxSize;i++)
	{
		waddch(stdscr, ' ');
	}
	
	wattrset(stdscr, COLOR_PAIR(BLACKONWHITE));
	for (i;i<30;i++)
	{
		mvwaddch(stdscr,5,10+i,' ');
	}
	
	wattrset(stdscr, COLOR_PAIR(WHITEONBLUE) | WA_BOLD);	
}

void printPulseWidth(int pulse)
{
	mvprintw(3,10,"Pulse WIDTH     ");
	
	printw("%d (%d%)  ",pulse,calculatePercent(pulse));
	printBox(pulse);

	refresh();
}

int main (int argc, char*argv[])
{
	int ch;
	int row,col;
	int width=0;
	int temp;

	!SIMULATION && printf("SIMULACION\n");
	
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();
	
	char option,lastoption,aux;
	int num;
	int initialWidth;
	
	SIMULATION && setup(PULSE_WIDTH_INCREMENT_GRANULARITY_US_DEFAULT, DELAY_VIA_PWM);
	SIMULATION && init_channel(CHANNEL, SUBCYCLE_TIME);
		
	if (argc==2)
	{
		if (strcmp(argv[1],"calibration")==0)
		{
			printw("CALIBRATION mode\n");
			printw("================\n");
			SIMULATION && print_channel(CHANNEL);
			printw("Disconnect Battery and press enter...");
			getch();
			printw("\n");
			setWidth(&width,HIGH_RANGE);
			printw("Width to the top. Connect ESC.\n");
			printw("Wait for a tone.\n");
			printw("Wait for a BEEP BEEP (means throttle at top) and press enter...");
			getch();
			printw("\n");
			setWidth(&width,LOW_RANGE);
			printw("Wait for a BEEP BEEP BEEP(means throttle at bottom)\n");
			printw("Wait for about 5s. BEEP and press enter...");
			getch();	
		}
		else
		{
			printw("NORMAL mode (for calibration mode use parameter 'calibration')\n");
			printw("==============================================================\n");
			setWidth(&width,LOW_RANGE);
			printw("Connect Battery and press enter...\n");
			getch();
		}
	}
	else
	{
		printw("NORMAL mode (for calibration mode use parameter 'calibration')\n");
		printw("==============================================================\n");
		setWidth(&width,LOW_RANGE);
		printw("Connect Battery and press enter...\n");
		getch();
	}
	
	initGUI(COLOR_PAIR(WHITEONBLUE) | WA_BOLD);
	drawScreen();
	setTitle(stdscr,"ESC Pulse modulation test");	
	
	
	char *choices[] =
	{
		"'i' inc +1",
		"'I' inc +10",
		"'d' dec -1",
		"'D' dec -10",
		"'q' quit",
		NULL
	};
	
	printPulseWidth(width);
	ch=0;
	while (ch!=4)
	{
		ch=runMenu(stdscr, 10-1, 20-1, getScreenY()-13, getScreenX()-20, choices, ch);	
		switch (ch)
		{
			case 0:		incWidth(&width,1);
						printPulseWidth(width);
						refresh();
						break;

			case 1:		incWidth(&width,10);
						printPulseWidth(width);
						break;
						
			case 2:		decWidth(&width,1);
						printPulseWidth(width);
						break;
						
			case 3:		decWidth(&width,10);
						printPulseWidth(width);
						break;
						
			case 'q':	quit();
		}
	}
	endGUI();
	return 0;
}
