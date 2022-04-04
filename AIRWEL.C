#include<graphics.h>

/* Project Wel-Come Screen */
void welcome(void)
{
	int gd=DETECT, gm, errorcode;
	int i, j, toggle, x, y, skip;
	int tiles[] = { LIGHTGRAY,
					BLUE };
	char temp[2], *names[] = { "Prashanth MN"};

	initgraph(&gd, &gm, "e:\\tc\\bgi");

	/* read result of initialization */
	errorcode = graphresult();

	if (errorcode != grOk)  /* an error occurred */
	{
		printf("Graphics error: %s\n", grapherrormsg(errorcode));
		printf("Spcify path of graphics files in \"initgraph\".\n");
		printf("Press any key to halt:");
		getch();
		exit(1);             /* return with error code */
	}

	/* 1st screen */
	/* Tiles background */
	for(toggle=y=0; y < 470; y+=50)	{
		if (y % 100)
			toggle=1;
		for (x=0; x < 640; x+=50, toggle++)	{
			if (toggle == 2)
				toggle=0;
			rectangle(x, y, x+50, y+50);
			setfillstyle(6, tiles[toggle]);
			floodfill(x+1, y+1, WHITE);
		}
	}
	setcolor(LIGHTGREEN);
	settextstyle(0, HORIZ_DIR, 6);
	outtextxy(30,152,"Havi Airways");
	outtextxy(10,250,"International");
	getch();

	/* 2nd screen */
	cleardevice();
	setbkcolor(BLACK);
	setcolor(BLUE);
	rectangle(0,0,640,75);
	setfillstyle(1,BLUE);
	floodfill(4,4,BLUE);
	setcolor(WHITE);
	settextstyle(0, HORIZ_DIR, 1);
	outtextxy(10,10,"Wel-Come to");
	outtextxy(570,64,"Project");
	setcolor(LIGHTGREEN);
	settextstyle(0, HORIZ_DIR, 3);
	outtextxy(5,25,"AIRWAYS RESERVATION SYSTEM");
	setlinestyle(0, 1, 2); /* select the line style */
	line(5,50,628,50);
	setcolor(GREEN);
	rectangle(0,76,70,480);
	setfillstyle(1,WHITE);
	floodfill(4,90,GREEN);
	setcolor(MAGENTA);
	settextstyle(0, VERT_DIR, 3);
	outtextxy(45,70,"Havi Airways Intl");

	setcolor(WHITE);
	settextstyle(0, HORIZ_DIR, 1);
	outtextxy(90,90,"Project Developed By:");

	skip = 33;
	setcolor(CYAN);
	settextstyle(0, HORIZ_DIR, 2);
	temp[1] = '\0';
	for (i=0, y=120; i<1; i++, y+=skip)	{
		circle(85,y+7,4);
		setfillstyle(1,CYAN);
		floodfill(85,y+7,CYAN);
		for(x=100,j=0; names[i][j] != '\0'; j++)	{
			temp[0] = names[i][j];
			outtextxy(x,y,temp);
			x += textwidth(temp)+5;
			if (temp[0] == ' ')
				x += 5;
				delay(60);
		}
	}
	setcolor(WHITE);
	settextstyle(0, HORIZ_DIR, 1);
	outtextxy(220,330,"Under The Guidence Of:");
	setcolor(CYAN);
	settextstyle(0, HORIZ_DIR, 2);
	outtextxy(220,350,"Vivekananda Murthy");
	settextstyle(0, HORIZ_DIR, 1);
	outtextxy(220,375, "H.O.D., Dept. Of Info Science,");
	outtextxy(220,390,"KIT");
	outtextxy(220,405,"Tiptur");
	getch();

	closegraph();
	return ;
}