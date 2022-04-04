/* Reservation Cum Waiting */
#include<stdio.h>
#include<dos.h>    /* for date and time structures */
#include"defs.h"   /* local file containing define statements & function declrations */

#define PNRLT		15
#define DATELT		11
#define TIMELT		6
#define TOT_SEATLT	2
#define PHONELT	   	8
#define NAMELT		26
#define AGELT		4
#define FARELT		10

#define ATTB		0X60 /* normal attribute */
#define BATTB		0x1B /* block attribute */
#define HATTB		0x5E /* highlight attribute */

int waitf(void);
void waitdata(void);
int waiting(void);
static void clrblocks(void);
static void cells(void);
static void vertline(int c);
static void print(void);

/* The constants FLINOLT, SECTORLT used here are defined in defs.h for
Flight Details Entry */
extern struct airflight flight[]; /* defined in flight.c */
extern struct res_manifesto enq[]; /* defined in enquiry.c */
extern int eindex;

FILE *wait_fp;
int windex; /* next free position in structure */
struct waits wait[MAXWAITS];

int waiting()
{
	int i, j, itembut, attbbut, hattbbut, rowbut, colbut, skip;
	int printed = 0;

	char *but[] = { "F2  Print ",
					"F10  Exit " };
	char *help[] = {"Prints the Ticket.",
					"Press ENTER or ESC to exit" };

	/* Screen Design */
	scrfil(' ', ATTB);
	box(0,0,79,24,0x6B,-1);
	pstr("Reservation Cum Waiting",2,27,0x74);
	pstr("Date:",3,4,0x6F);
	pstr("Mode:",3,60,0x6F);
	textline(4,1,78,0x6B);
	pchar(4,0,'Ã',0x6B);
	pchar(4,79,'´ ',0x6B);
	pstr("PNR No",6,2,ATTB);
	pstr("Flight No",6,28,ATTB);
	pstr("Sector",6,51,ATTB);
	pstr("Departure Date",8,2,ATTB);
	pstr("Departure Time",8,51,ATTB);
	pstr("Total No of Seat",10,2,ATTB);
	pstr("Telephone",10,51,ATTB);
	ipblock(10,61,PHONELT,BATTB);
	cells();
	dispdate(3,10,0x6F);

	attbbut = 0x0A;
	hattbbut = 0x20;
	rowbut = 22;
	colbut = 20;
	skip = 28;
	for (i=0, j=colbut; i<=1; i++, j+=skip)
		pstr(but[i], rowbut, j, attbbut);

	if (waitf() == 0) /* if ESC hit in middle of Wait Entry */
		return 0;

	itembut = 0;
	pstr(but[itembut], rowbut, colbut, hattbbut); /* highlight first button */
	statbar(help[itembut]);
	pstr("YOU SHOULD PRESS THIS BUTTON FOR THIS RECORD TO UPDATE",23,25,0x9E);

	for (;;)	{
		switch (scan())	{
		case RIGHT:
		case TAB:
			pstr(but[itembut], rowbut, colbut, attbbut);
			if (itembut == 1)	/* if at last button */
				itembut=0, colbut=20;
			else
				itembut++, colbut += skip;
			pstr(but[itembut], rowbut, colbut, hattbbut);
			statbar(help[itembut]);
			if (itembut == 0)
				pstr("YOU SHOULD PRESS THIS BUTTON FOR THIS RECORD TO UPDATE",23,25,0x9E);
			break;
		case LEFT:
		case SHTAB:
			pstr(but[itembut], rowbut, colbut, attbbut);
			if (itembut == 0)	/* if at first button */
				itembut=1, colbut=48;
			else
				itembut--, colbut -= skip;
			pstr(but[itembut], rowbut, colbut, hattbbut);
			statbar(help[itembut]);
			if (itembut == 0)
				pstr("YOU SHOULD PRESS THIS BUTTON FOR THIS RECORD TO UPDATE",23,25,0x9E);
			break;
		case F2:
			pstr(but[itembut], rowbut, colbut, attbbut);
			print();
			printed = 1; /* print job completed */
			pstr(but[itembut], rowbut, colbut, hattbbut);
			break;
		case F10:
		case ESC:
			pstr("      ", 3, 67, 0x1F);
			return printed;
		case ENTER:
			switch(itembut)	{
				case 0:
					pstr(but[itembut], rowbut, colbut, attbbut);
					print();
					printed = 1; /* print job completed */
					pstr(but[itembut], rowbut, colbut, hattbbut);
					break;
				case 1:
					pstr("      ", 3, 67, 0x1F);
					return printed;
			}
			break;
		}
	}
}


/* waitf: returns 0, if job breaks in middle, 1 if job completes successfully */
int waitf(void)
{
	char c, temp[8];
	int i, j, attb, row;
	float tot_fare = 0.0;

	pstr("WAIT",3,67,0x6F);

	for (i=0; i < MAXWAITS; i++)
		if (wait[i].status == 0)
			break;
	if (i == MAXWAITS)	{
		msgbox("WAIT.DAT Full. No more entries possible", 0);
		return 0;
	}
	windex=i; /* first free record */

	/* copy flino and PNR No. from structure enq to sturcture wait */
	strcpy(wait[windex].pnr, enq[eindex].pnr);
	wait[windex].flino = enq[eindex].flino;
	/* copy departure date */
	wait[windex].depdate.day = enq[eindex].depdate.day;
	wait[windex].depdate.month = enq[eindex].depdate.month;
	wait[windex].depdate.year = enq[eindex].depdate.year;
	/* copy departure time */
	wait[windex].deptime = enq[eindex].deptime;

	pstr(wait[windex].pnr, 6, 9, BATTB);
	pstr(flight[wait[windex].flino].flino,6,38,BATTB);
	pstr(flight[enq[eindex].flino].sector,6,58,BATTB);
	textattr(BATTB);
	gotoxy(18,9);
	cprintf("%d/%d/%d", enq[eindex].depdate.day, enq[eindex].depdate.month, \
	enq[eindex].depdate.year);
	gotoxy(67,9);
	cprintf("%.2f",enq[eindex].deptime);
	textattr(HATTB);

	/* get total no of seats */
	for(;;)  {
		pchar(10,19,' ',HATTB);
		statbar("Enter Total No of Seats. It should be less than 4.");
		gotoxy(20,11);
		if ((c = getch()) == ESC)
			if ((msgbox("Do you want to leave?",1)) == 'Y')
				return 0;
			else
				continue;
		switch(c)	{
			case '1': wait[windex].totseat = 1; break;
			case '2': wait[windex].totseat = 2; break;
			case '3': wait[windex].totseat = 3; break;
			case '4': wait[windex].totseat = 4; break;
			default: wait[windex].totseat = 0; break;
		}
		if (wait[windex].totseat < 1 || wait[windex].totseat > 4)
			msgbox("Total seats should be in between 1 and 4",0);
		else
			break;
	}
	pchar(10,19,c,BATTB);

	/* get telephone no */
	for(;;) 	{
		ipblock(10,61,PHONELT,HATTB);
		statbar("Enter Passenger's Telephone No.");
		if (!(getip(10,61,wait[windex].phone,PHONELT)))
			if ((msgbox("Do you want to leave?",1)) == 'Y')
				return 0;
			else
				continue;
		break;
	}
	ipblock(10,61,PHONELT,BATTB);
	pstr(wait[windex].phone,10,61,BATTB);

	for (j=0, row=16; j < wait[windex].totseat; j++, row++)	{
		pstr("     ³       ³                         ³   ³   ³      ³   ³         ",row,6,HATTB);

		/* get class */
		do	{
			pchar(row,8,' ',HATTB);
			statbar("F - First Class       B - Business Class       E - Economy Class");
			gotoxy(9,row+1);
			if ((c = toupper(getch()) ) == ESC)
				if ((msgbox("Do you want to leave?",1)) == 'Y')
					return 0;
		}while (c != 'F' && c != 'B' && c != 'E');
		pchar(row,8,c,HATTB);
		wait[windex].clas[j] = c;

		/* get seating option */
		do	{
			pchar(row,15,' ',HATTB);
			statbar("S - Smoking Seat          N - Non Smoking Seat");
			gotoxy(16,row+1);
			if ((c = toupper(getch()) ) == ESC)
				if ((msgbox("Do you want to leave?",1)) == 'Y')
					return 0;
		}while (c != 'S' && c != 'N');
		pchar(row,15,c,HATTB);
		wait[windex].so[j] = c;


		textattr(HATTB);
		/* get name */
		do 	{
			statbar("Enter Passenger's Name");
			if (!(getip(row,20,wait[windex].name[j],NAMELT)))
				if ((msgbox("Do you want to leave?",1)) == 'Y')
					return 0;
				else {
					wait[windex].name[j][0] = '\0';
					continue;
				}
		}while (wait[windex].name[j][0] == '\0');
		pstr(wait[windex].name[j],row,20,HATTB);

		/* get age */
		do 	{
			statbar("Enter Passenger's Age");
			if (!(getip(row,46,temp,4)))
				if ((msgbox("Do you want to leave?",1)) == 'Y')
					return 0;
				else {
					temp[0] = '\0';
					continue;
				}
			if (!(wait[windex].age[j] = atoi(temp))) { /* converts char string to int */
				temp[0] = '\0'; /* not valid input */
				msgbox("Age field should contain only numbers",0);
			}
		}while (temp[0] == '\0');
		gotoxy(47,row+1);
		cprintf("%d",wait[windex].age[j]);

		/* get sex */
		do	{
			statbar("M - Male          F - Female");
			gotoxy(52,row+1);
			if ((c = toupper(getch()) ) == ESC)
				if ((msgbox("Do you want to leave?",1)) == 'Y')
					return 0;
		}while (c != 'M' && c != 'F');
		pchar(row,51,c,HATTB);
		wait[windex].sex[j] = c;

		/* get meals option */
		do	{
			statbar("V - Veg             N - Non-Veg");
			gotoxy(57,row+1);
			if ((c = toupper(getch()) ) == ESC)
				if ((msgbox("Do you want to leave?",1)) == 'Y')
					return 0;
		}while (c != 'V' && c != 'N');
		pchar(row,56,c,HATTB);
		wait[windex].mo[j] = c;

		/* get SSR option */
		do	{
			statbar("Special Service Request(SSR) required? Y/N");
			gotoxy(63,row+1);
			if ((c = toupper(getch()) ) == ESC)
				if ((msgbox("Do you want to leave?",1)) == 'Y')
					return 0;
		}while (c != 'Y' && c != 'N');
		pchar(row,62,c,HATTB);
		wait[windex].ssr[j] = c;

		/* print fare */
		gotoxy(66,row+1);
		switch(wait[windex].clas[j]) {
		case 'F':
			wait[windex].fare[j] = flight[enq[eindex].flino].f_fare;
			tot_fare += flight[enq[eindex].flino].f_fare;
			break;
		case 'B':
			wait[windex].fare[j] = flight[enq[eindex].flino].b_fare;
			tot_fare += flight[enq[eindex].flino].b_fare;
			break;
		case 'E':
			wait[windex].fare[j] = flight[enq[eindex].flino].e_fare;
			tot_fare += flight[enq[eindex].flino].e_fare;
			break;
		}
		cprintf("%9.2d", wait[windex].fare[j]);

		/* clear the highlighted bar */
		pstr("     ³       ³                         ³   ³   ³      ³   ³         "\
		,row,6,0x3F);
		/* reprint all datas */
		attb=0x35;
		textattr(attb);
		pchar(row,8,wait[windex].clas[j],attb);
		pchar(row,15,wait[windex].so[j],attb);
		pstr(wait[windex].name[j],row,20,attb);
		gotoxy(47,row+1);
		cprintf("%d",wait[windex].age[j]);
		pchar(row,51,wait[windex].sex[j],attb);
		pchar(row,56,wait[windex].mo[j],attb);
		pchar(row,62,wait[windex].ssr[j],attb);
		gotoxy(66,row+1);
		cprintf("%9.2f", wait[windex].fare[j]);
	}

	/* print total fare */
	gotoxy(66,22);
	cprintf("%9.2f",tot_fare);
	cprintf(" ");
	gotoxy(1,1); /* send cursor from middle of screen */

	pstr("       ",3,67,0x6F);
	return 1;
}


void print(void)
{
}


/* Opens WAIT.DAT file for global access */
void waitdata(void)
{
	/* read wait data */
	if (access("wait.dat", 0))
		msgbox("File does not exist:\"WAIT.DAT\". New file created", 0);
	if ((wait_fp = fopen("wait.dat", "a+")) != NULL) {
		windex = fread(wait, sizeof(struct waits), MAXWAITS, wait_fp);
		/* windex: next free structure position */
	}
	else	{
		errbeep();
		msgbox("File creation error:\"WAIT.DAT\"", 0);
		exit(1);
	}
}


/* prints cells */
void cells(void)
{
	const int row=12, col=5;
	int r=row, c=col, cellattb = 0x3F, attb=0x31;

	box(c,r,74,row+8,cellattb,-1);
	/* fill box */
	for (r = row+1; r <= row+7; r++)
		for(c=col+1; c < 74; c++)
			pchar(r,c,' ',cellattb);
	r = row, c = col;
	textline(r+3,c+1,73,cellattb); /* horizontal line */
	pchar(r+3,c,'Ã',cellattb);
	pchar(r+3,74,'´',cellattb);
	pstr("Class Seating     Name of Passenger     Age Sex Meals  SSR   Fare",\
	r+1,c+1,attb);
	pstr("      Option                                    Option Req          ",\
	r+2,c+1,attb);
	vertline(11);
	vertline(19);
	vertline(45);
	vertline(49);
	vertline(53);
	vertline(60);
	vertline(64);
	pstr("Total",row+9,60,0x6B);
}


/* prints a verticle line at column c */
void vertline(int c)
{
	int r, cellattb = 0x3F;
	pchar(12,c,'Â',cellattb);
	for(r=13; r<20; r++)
		pchar(r,c,'³',cellattb);
	pchar(15,c,'Å',cellattb);
	pchar(20,c,'Á',cellattb);
}

