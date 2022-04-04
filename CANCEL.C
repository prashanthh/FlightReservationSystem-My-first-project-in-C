/* Reservation Against Cancellation */
#include<stdio.h>
#include<dos.h>    /* for date and time structures */
#include<string.h> /* for strcmpi() */
#include"defs.h"   /* local file containing define statements & function declarations */

#define MAXCANCELS	10
#define PNRLT		15
#define DATELT		11
#define TIMELT		6
#define TOT_SEATLT	2
#define PHONELT	   	8
#define NAMELT		26
#define AGELT		4
#define FARELT		10

#define ATTB		0X1B /* normal attribute */
#define BATTB		0x3F /* block attribute */
#define HATTB		0x5F /* highlight attribute */

int cancelf(void);
void canceldata(void);
static void clrblocks(void);
static void cells(void);
static void vertline(int c);
static void print(void);
static void update(void);
void moveup_confirm(int);

/* The constants FLINOLT, SECTORLT used here are defined in defs.h for
Flight Details Entry */
extern struct airflight flight[]; /* defined in flight.c */
extern struct res_manifesto enq[]; /* defined in enquiry.c */
extern int eindex;
extern struct reservation confirm[]; /* defined in confirm.c */
extern int cindex;
extern struct con conf[]; /* definded in enquiry.c */

FILE *cancel_fp;
int ccindex; /* next free position in structure */
int c_recno; /* record no. of confirm */
int wait_rec; /* record no. of wait */
struct canc cancel[MAXCANCELS];

void cancellation(void)
{
	int i, j, itembut, attbbut, hattbbut, rowbut, colbut, skip;

	char *but[] = { "F2  Print ",
					"F10  Exit " };
	char *help[] = {"Prints the Ticket.",
					"Press ENTER or ESC to exit" };

	if (ccindex >= MAXCANCELS)	{
		msgbox("CANCEL.DAT Full. No more entries possible", 0);
		return ;
	}

	/* Screen Design */
	scrfil(' ', ATTB);
	box(0,0,79,24,0x12,-1);
	pstr(" Reservation Against Cancellation ",2,24,0x0C);
	pstr("Date:",3,4,0x1F);
	pstr("Mode:",3,60,0x1F);
	textline(4,1,78,0x12);
	pchar(4,0,'Ã',0x12);
	pchar(4,79,'´ ',0x12);
	pstr("PNR No",5,2,ATTB);
	pstr("Flight No",5,28,ATTB);
	pstr("Sector",5,51,ATTB);
	pstr("Departure Date",6,2,ATTB);
	pstr("Departure Time",6,51,ATTB);
	pstr("Total No of Seat",7,2,ATTB);
	pstr("Telephone",7,51,ATTB);
	ipblock(10,61,PHONELT,BATTB);
	cells();
	dispdate(3,10,0x1F);

	attbbut = 0x0A;
	hattbbut = 0x30;
	rowbut = 22;
	colbut = 20;
	skip = 28;
	for (i=0, j=colbut; i<=1; i++, j+=skip)
		pstr(but[i], rowbut, j, attbbut);

	if (cancelf() == 0) /* if ESC hit in middle of Cancellation Entry */
		return ;

	itembut = 0;
	pstr(but[itembut], rowbut, colbut, hattbbut); /* highlight first button */
	statbar(help[itembut]);
	pstr("YOU SHOULD PRESS THIS BUTTON FOR THIS RECORD TO UPDATE",23,25,0xDE);

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
				pstr("YOU SHOULD PRESS THIS BUTTON FOR THIS RECORD TO UPDATE",23,25,0xDE);
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
				pstr("YOU SHOULD PRESS THIS BUTTON FOR THIS RECORD TO UPDATE",23,25,0xDE);
			break;
		case F2:
			pstr(but[itembut], rowbut, colbut, attbbut);
			print();
			update();
			pstr(but[itembut], rowbut, colbut, hattbbut);
			break;
		case F10:
		case ESC:
			pstr("      ", 3, 67, 0x1F);
			return;
		case ENTER:
			switch(itembut)	{
				case 0:
					pstr(but[itembut], rowbut, colbut, attbbut);
					print();
					update();
					pstr(but[itembut], rowbut, colbut, hattbbut);
					break;
				case 1:
					pstr("      ", 3, 67, 0x1F);
					return ;
			}
			break;
		}
	}
}


/* Takes cancellation details. returns 0, if job breaks in middle, 1 if job completes
successfully */
int cancelf(void)
{
	char c, temp[16], temp2[50];
	int  i, j, attb, row, seatindex;
	float tot_fare = 0.0;

	pstr("CANCEL",3,67,0x1F);

	/* get PNR No. */
	textattr(HATTB);
	do 	{
		ipblock(5,10,PNRLT,HATTB);
		statbar("Enter PNR No. from the ticket");
		if (!(getip(5,10,temp,PNRLT)))
			if ((msgbox("Do you want to leave?",1)) == 'Y')
				return 0;
			else {
				temp[0] = '\0';
				continue;
			}
		for (c_recno=0; c_recno < MAXCONFIRMS; c_recno++) /* check if PNR No. exists */
			if (confirm[c_recno].status != 0 && strcmpi(temp, confirm[c_recno].pnr) == 0)
				break;
		if (c_recno == MAXCONFIRMS)	{
			msgbox("PNR No. not found", 0);
			temp[0] = '\0';
			continue; /* re-enter PNR No. */
		}
		break;
	}while (temp[0] == '\0');
	ipblock(5,10,PNRLT,BATTB);
	pstr(temp,5,10,BATTB);

	pstr(flight[confirm[c_recno].flino].flino,5,38,BATTB);
	pstr(flight[confirm[c_recno].flino].sector,5,58,BATTB);
	textattr(BATTB);
	gotoxy(18,7);
	cprintf("%d/%d/%d", confirm[c_recno].depdate.day, confirm[c_recno].depdate.month, \
	confirm[c_recno].depdate.year);
	gotoxy(67,7);
	cprintf("%.2f",confirm[c_recno].deptime);
	textattr(HATTB);

	/* get total no of seats */
	/* if totseat is not more than 1, why to ask for input */
	if (confirm[c_recno].totseat != 1)	{
		for(;;)  {
			pchar(7,19,' ',HATTB);
			statbar("Enter Total No of Seats. It should be less than 4.");
			gotoxy(20,8);
			if ((c = getch()) == ESC)
				if ((msgbox("Do you want to leave?",1)) == 'Y')
					return 0;
				else
					continue;
			switch(c)	{
				case '1': cancel[ccindex].totseat = 1; break;
				case '2': cancel[ccindex].totseat = 2; break;
				case '3': cancel[ccindex].totseat = 3; break;
				case '4': cancel[ccindex].totseat = 4; break;
				default: cancel[ccindex].totseat = 0; break;
			}
			if (cancel[ccindex].totseat < 1 || cancel[ccindex].totseat > 4)
				msgbox("Total seats should be in between 1 and 4",0);
			else if (cancel[ccindex].totseat > confirm[c_recno].totseat)	{
				sprintf(temp2,"Maximum reserved seats under this PNR No. is %d", \
				confirm[c_recno].totseat);
				msgbox(temp2, 0);
			}
			else
				break;
		}
		pchar(7,19,c,BATTB);
	}
	else	{
		textattr(BATTB);
		gotoxy(20,8);
		cprintf("%d",cancel[ccindex].totseat = 1);
	}

	pstr(confirm[c_recno].phone,7,61,BATTB); /* print phone from structure confirm */

	for (j=0, row=13; j < cancel[ccindex].totseat; j++, row++)	{
		textattr(0x50);
		pstr("       ³     ³       ³                         ³   ³   ³      ³   ³         "\
		,row,2,HATTB);

		if (confirm[c_recno].totseat > 1)	{
			/* get seat no */
			getseat: do 	{
				statbar("Enter seat no to cancel");
				if (!(getip(row,4,temp,4)))
					if ((msgbox("Do you want to leave?",1)) == 'Y')
						return 0;
					else	{
						temp[0] = '\0';
						continue;
					}
				/* converts char string to int */
				if (!(cancel[ccindex].seatno[j] = atoi(temp)))	{
					temp[0] = '\0'; /* not valid input */
					msgbox("Seat No. field should contain only numbers",0);
				}
			}while (temp[0] == '\0');
			gotoxy(5,row+1);
			cprintf("%d",cancel[ccindex].seatno[j]);

			/* search Seat No in confirm */
			for (seatindex=0; seatindex < confirm[c_recno].totseat; seatindex++)
				if (confirm[c_recno].seatno[seatindex] == cancel[ccindex].seatno[j])
					break;
			if (seatindex == confirm[c_recno].totseat)	{
				msgbox("This is not a confirmed Seat No.",0);
				pstr("   ",row,4,HATTB);
				goto getseat;
			}
			for (i=0; i<j; i++)
				if (cancel[ccindex].seatno[j] == cancel[ccindex].seatno[i])	{
					msgbox("Repeat Entry", 0);
					pstr("   ",row,4,HATTB);
					goto getseat;
				}
		}
		else	{
			seatindex=0;
			cancel[ccindex].seatno[j] = confirm[c_recno].seatno[j];
		}

		/* copy other details */
		cancel[ccindex].clas[j] = confirm[c_recno].clas[seatindex];
		cancel[ccindex].so[j] = confirm[c_recno].so[seatindex];
		strcpy(cancel[ccindex].name[j], confirm[c_recno].name[seatindex]);
		cancel[ccindex].age[j] = confirm[c_recno].age[seatindex];
		cancel[ccindex].sex[j] = confirm[c_recno].sex[seatindex];
		cancel[ccindex].mo[j] = confirm[c_recno].mo[seatindex];
		cancel[ccindex].ssr[j] = confirm[c_recno].ssr[seatindex];
		tot_fare += cancel[ccindex].fare[j] = confirm[c_recno].fare[seatindex];

		/* clear the highlighted bar */
		pstr("       ³     ³       ³                         ³   ³   ³      ³   ³         "\
		,row,2,0x6F);
		/* reprint all datas */
		attb=0x6A;
		textattr(attb);
		gotoxy(5,row+1);
		cprintf("%d",cancel[ccindex].seatno[j]);
		pchar(row,12,cancel[ccindex].clas[j],attb);
		pchar(row,17,cancel[ccindex].so[j],attb);
		pstr(cancel[ccindex].name[j],row,24,attb);
		gotoxy(51,row+1);
		cprintf("%d",cancel[ccindex].age[j]);
		pchar(row,55,cancel[ccindex].sex[j],attb);
		pchar(row,60,cancel[ccindex].mo[j],attb);
		pchar(row,66,cancel[ccindex].ssr[j],attb);
		gotoxy(70,row+1);
		cprintf("%9.2f",cancel[ccindex].fare[j]);
	}

	gotoxy(70,19); /* print total fare */
	cprintf("%9.2f ",tot_fare);
	gotoxy(70,20); /* cancellation charge */
	cprintf("%9.2f ", 0.1 * tot_fare); /* 10% of ticket amount */
	gotoxy(70,21); /* total refund */
	cprintf("%9.2f ", tot_fare - (0.1 * tot_fare));
	gotoxy(1,1); /* send off cursor from middle of screen */

	pstr("       ",3,67,0x1F);
	return 1;
}


void print(void)
{
}


/* Updates confirm entry, wait entry, frees cancelled seats */
void update(void)
{
	int i, j, k, conf_rec;
	FILE *confirm_fp, *conf_fp;

	/* update confirm */
	for (i=0; i < cancel[ccindex].totseat; i++)
		for (j=0; j < confirm[c_recno].totseat; j++)
			if (cancel[ccindex].seatno[i] == confirm[c_recno].seatno[j])
				moveup_confirm(j);

	confirm[c_recno].totseat -= cancel[ccindex].totseat;
	/* if all seats were cancelled in this PNR No. */
	if (confirm[c_recno].totseat == 0)
		confirm[c_recno].status = 0; /* delete the record in confirm */
	if ((confirm_fp = fopen("confirm.dat","w")) != NULL)
		fwrite(confirm, sizeof(struct reservation), MAXCONFIRMS, confirm_fp);
	else
		msgbox("Error opening file: CONFIRM.DAT", 0);
	fclose(confirm_fp);

	/* update conf */
	for (conf_rec=0; conf_rec < MAXCONF_FLIGHTS; conf_rec++)
		/* hold index of matching conf */
		if (conf[conf_rec].reserve == 1 && confirm[c_recno].flino == \
		conf[conf_rec].flino && confirm[c_recno].depdate.day == conf[conf_rec].day \
		&& confirm[c_recno].depdate.month == conf[conf_rec].month && \
		conf[conf_rec].year == confirm[c_recno].depdate.year)
			break;
	/* set cancelled seats to 0 */
	for (k=0; k < cancel[ccindex].totseat; k++)
		for (j=0; j < MAXACSEATS; j++)
			if (conf[conf_rec].confirmed[j] == cancel[ccindex].seatno[k])	{
				conf[conf_rec].confirmed[j] = 0;
				break;
			}

	/* append current record to file 'cancel' */
	fwrite(&cancel[ccindex], sizeof(struct canc), 1, cancel_fp);
	ccindex++;
	fclose(cancel_fp);

	/* save conf */
	if ((conf_fp = fopen("conf.dat","w")) != NULL)
		fwrite(conf, sizeof(struct con), MAXCONF_FLIGHTS, conf_fp);
	else
		msgbox("Error opening file: CONF.DAT", 0);
	fclose(conf_fp);
}



/* moves entries one position UP in the specified of struct confirm */
void moveup_confirm(int j)
{
	for (; j < confirm[c_recno].totseat - cancel[ccindex].totseat; j++)	{
		confirm[c_recno].clas[j] = confirm[c_recno].clas[j+1];
		confirm[c_recno].so[j] = confirm[c_recno].so[j+1];
		confirm[c_recno].seatno[j] = confirm[c_recno].seatno[j+1];
		strcpy(confirm[c_recno].name[j] ,confirm[c_recno].name[j+1]);
		confirm[c_recno].age[j] = confirm[c_recno].age[j+1];
		confirm[c_recno].sex[j] = confirm[c_recno].sex[j+1];
		confirm[c_recno].mo[j] = confirm[c_recno].mo[j+1];
		confirm[c_recno].ssr[j] = confirm[c_recno].ssr[j+1];
		confirm[c_recno].fare[j] = confirm[c_recno].fare[j+1];
	}
}


/* Opens CANCEL.DAT file for global access */
void canceldata(void)
{
	/* read cancel data */
	if (access("cancel.dat", 0))
		msgbox("File does not exist:\"CANCEL.DAT\". New file created", 0);
	if ((cancel_fp = fopen("cancel.dat", "a+")) != NULL) {
		ccindex = fread(cancel, sizeof(struct canc), MAXCANCELS, cancel_fp);
		/* ccindex: next free structure position */
	}
	else	{
		errbeep();
		msgbox("File creation error:\"CANCEL.DAT\"", 0);
		exit(1);
	}
}


/* creats a cell box for data entry */
void cells(void)
{
	const int row=9, col=1;
	int r=row, c=col, cellattb = 0x6F, attb=0x61;

	box(c,r,78,row+8,cellattb,-1);
	/* fill box */
	for (r = row+1; r <= row+7; r++)
		for(c=col+1; c < 78; c++)
			pchar(r,c,' ',cellattb);
	r = row, c = col;
	textline(r+3,c+1,77,cellattb); /* horizontal line */
	pchar(r+3,c,'Ã',cellattb);
	pchar(r+3,78,'´',cellattb);
	pstr("Seat No Class Seating     Name of Passenger     Age Sex Meals  SSR   Fare",\
	r+1,c+1,attb);
	pstr("              Option                                    Option Req          ",\
	r+2,c+1,attb);
	vertline(9);
	vertline(15);
	vertline(23);
	vertline(49);
	vertline(53);
	vertline(57);
	vertline(64);
	vertline(68);
	pstr("Total:",row+9,62,0x1B);
	pstr("Cancellation Charge:",row+10,48,0x1B);
	pstr("Total Refund:",row+11,55,0x1B);
}


/* prints a verticle line at column c */
void vertline(int c)
{
	int r, cellattb = 0x6F;
	pchar(9,c,'Â',cellattb);
	for(r=10; r<17; r++)
		pchar(r,c,'³',cellattb);
	pchar(12,c,'Å',cellattb);
	pchar(17,c,'Á',cellattb);
}

