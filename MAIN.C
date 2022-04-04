/* MAIN PROGRAM */
/* Opening menu design */
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>	/* for exit() and malloc() */
#include <string.h> /* for strcmpi() */
#include <dos.h>	/* for dosdate_t structure */

#include "defs.h"	/* Function declarations and Key Defines */
#include "airwel.c" /* wel-come function */

/* function declarations */
void welcome(void);
void mastdata(void);
void flightdata(void);
void airdetaildata(void);
void enquirydata(void);
void masterentry(void);
void airdetailentry(void);
void flightentry(void);
void enquiry(void);
void cancellation(void);
void confirmreport(void);
void waitreport(void);

/* externally defined variables */
extern struct airflight flight[]; /* defined in flight.c */
extern struct waits wait[]; /* defined in wait.c */
extern struct reservation confirm[]; /* defined in confirm.c */


int main(void)
{
	int r,c, ch, attb, item_no, i,j, attbcursor;
	char *p;
	char *item[] = { "Aircraft Master Entry           ", "Aircraft Detail Entry           ",
					 "Flight Details Entry            ", "Reservation Enquiry             ",
					 "Reservation Against Cancellation", "Confirmation Report             " ,
					 "Waiting List Report             ", "Quit                            "};

	/* Wel-Come Screen */
//	welcome();

	/* open all data files */
	mastdata();
	airdetaildata();
	flightdata();
	enquirydata();
	confirmdata();
	waitdata();
	canceldata();

	scrfil(' ', 0x11); /* fill screen with blue color */

	r = 8, c = 25; /* co-ods where the starting chars of menu items occur */
	box(c,r,58,17,0x71,0x00);
	pstr("Airline Reservation System",6,29,0x9E);/* displays blinking title */
	r++, c++;
	attb = 0x60;
	item_no = 0;
	for (j=0, i = 0; j<8; j++,i++)	 	/* menu generation */
		pstr(item[item_no++], r+i, c, attb);

	attbcursor = 0x3F;
	item_no = 0;
	pstr(item[item_no],r,c,attbcursor); /* highlight first menu item */

	ch = scan();
	pstr("Airline Reservation System",6,29,0x1E); /* stops blinking of title
	after first key hit */
	/* memory allocation to save entire screen */
	if ((p = (char *)malloc(25 * 80 * 2)) == NULL)	{
		fprintf(stderr,"Not Enough Memory");
		exit(1);
	}

	while( ch != ESC)	{
		switch (ch)	{
		case UP:
		case RIGHT:
			/* restore selected menu item to normal color */
			pstr(item[item_no],r,c,attb);
			(item_no == 0? item_no=7: item_no--);
			r--;
			if (r == 8) r=16;
			pstr(item[item_no],r,c,attbcursor);
			break;
		case DOWN:
		case LEFT:
			pstr(item[item_no],r,c,attb); /* restores orig color */
			item_no == 7? item_no=0: item_no++;
			r++;
			if (r==17) r=9;
			pstr(item[item_no],r,c,attbcursor);
			break;
		case ENTER:
			if ((strcmp("Aircraft Master Entry           ", item[item_no]) == 0)) {
				savescr(0,0,24,79,p);
				masterentry();
				restorescr(0,0,24,79,p);
			}
			else if (!strcmp("Aircraft Detail Entry           ", item[item_no])) {
				savescr(0,0,24,79,p);
				airdetailentry();
				restorescr(0,0,24,79,p);
			}
			else if (strcmp("Flight Details Entry            ", item[item_no]) == 0) {
				savescr(0,0,24,79,p);
				flightentry();
				restorescr(0,0,24,79,p);
			}
			else if (strcmp("Reservation Enquiry             ", item[item_no]) == 0) {
				savescr(0,0,24,79,p);
				enquiry();
				restorescr(0,0,24,79,p);
			}
			else if (!strcmp("Reservation Against Cancellation", item[item_no])) {
				savescr(0,0,24,79,p);
				cancellation();
				restorescr(0,0,24,79,p);
			}
			else if (!strcmp("Confirmation Report             ", item[item_no])) {
				savescr(0,0,24,79,p);
				confirmreport();
				restorescr(0,0,24,79,p);
			}
			else if (!strcmp("Waiting List Report             ", item[item_no])) {
				savescr(0,0,24,79,p);
				waitreport();
				restorescr(0,0,24,79,p);
			}
			else if (!strcmp("Quit                            ", item[item_no])) {
				savescr(0,0,24,79,p);
				clrscr();
				exit(0);
			}
			break;
		}
		ch = scan();
	}
	free(p);
	normvideo();
	clrscr();
	return 0;
}


/* Generates a report of confirmed seats */
void confirmreport(void)
{
	int i, j, ch, row, attb=0x1F;

	scrfil(' ',0x11);
	textattr(attb);
	gotoxy(30,1);
	pstr(" Confirmation Report ",0,29,attb);
	pstr(" -------------------",1,29,attb);
	pstr("PNR No.        F NO   DEP DATE DEP TIME CLASS SO SEAT NAME",3,2,attb);
	pstr("-------        ----   -------- -------- ----- --  NO  ----",4,2,attb);
	for (i=0, row=7; i < MAXCONFIRMS; i++)	{
		if (confirm[i].status == 0)
			continue;
		gotoxy(2,row);
		if (row > 22)	{
			getch(); /* pause between pages */
			scrfil(' ',0x11);
			row=7;
			gotoxy(2,row);
			pstr(" Confirmation Report ",0,29,attb);
			pstr(" -------------------",1,29,attb);
			pstr("PNR No.        F NO   DEP DATE DEP TIME CLASS SO SEAT NAME",3,2,attb);
			pstr("-------        ----   -------- -------- ----- --  NO  ----",4,2,attb);
		}
		cprintf("%-16s%-5s%2d/%2d/%2d   %.2f", confirm[i].pnr, flight[confirm[i].flino].\
		flino, confirm[i].depdate.day, confirm[i].depdate.month, confirm[i].depdate.year,\
		confirm[i].deptime);
		for (j=0; j < confirm[i].totseat; j++)	{
			gotoxy(45,row++);
			cprintf("%c   %c  %2d  %-s\r\n",confirm[i].clas[j], \
			confirm[i].so[j], confirm[i].seatno[j], confirm[i].name[j]);
		}
		cprintf("\r\n");
	}
	getch();
}


/* Generates a report of seats in waiting list */
void waitreport(void)
{
	int i, j, ch, row, attb=0x3F;

	scrfil(' ',0x33);
	textattr(attb);
	gotoxy(30,1);
	pstr(" Waiting List ",0,30,attb);
	pstr(" ------------ ",1,30,attb);
	pstr("PNR No.         F NO    DEP DATE  DEP TIME CLASS SO NAME",3,2,attb);
	pstr("-------         ----    --------  -------- ----- -- ----",4,2,attb);
	for (i=0, row=7; i < MAXWAITS; i++)	{
		if (wait[i].status == 0)
			continue;
		gotoxy(2,row);
		if (row > 22)	{
			getch(); /* pause between pages */
			scrfil(' ',0x33);
			row=7;
			gotoxy(2,row);
			pstr(" Waiting List ",0,30,attb);
			pstr(" ------------ ",1,30,attb);
			pstr("PNR No.         F NO    DEP DATE  DEP TIME CLASS SO NAME",3,2,attb);
			pstr("-------         ----    --------  -------- ----- -- ----",4,2,attb);
		}
		cprintf("%-16s  %-5s %2d/%2d/%2d   %.2f", wait[i].pnr, flight[wait[i].flino].flino, \
		wait[i].depdate.day, wait[i].depdate.month, wait[i].depdate.year, wait[i].deptime);
		for (j=0; j < wait[i].totseat; j++)	{
			gotoxy(44,row++);
			cprintf("    %c    %c %-s\r\n",wait[i].clas[j], wait[i].so[j], \
			wait[i].name[j]);
		}
		cprintf("\r\n");
	}
	getch();
}
