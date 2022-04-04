/* Reservation Cum Confirm */
#include<stdio.h>
#include<dos.h>    /* for date and time structures */
#include"defs.h"   /* local file containing define consts & function declns */

#define PNRLT		15
#define DATELT		11
#define TIMELT		6
#define TOT_SEATLT	2
#define PHONELT	   	8
#define NAMELT		26
#define AGELT		4
#define FARELT		10

#define ATTB		0X31 /* normal attribute */
#define BATTB		0x0B /* block attribute */
#define HATTB		0x0B /* highlight attribute */

int confirmf(int, int);
void confirmdata(void);
int confirmation(int, int);
static void clrblocks(void);
static void cells(void);
static void vertline(int c);
static void print(void);

/* The constants FLINOLT, SECTORLT used here are defined in defs.h for
Flight Details Entry */
extern struct mastdetail mdetail[]; /* defined in mdetail.c */
extern struct airflight flight[]; /* defined in flight.c */
extern struct res_manifesto enq[]; /* defined in enquiry.c */
extern struct con conf[]; /* definded in enquiry.c */
extern int eindex;

FILE *confirm_fp;
int cindex; /* next free position in structure */
struct reservation confirm[MAXCONFIRMS];

int confirmation(int conf_index, int md_recno)
{
	int i, j, itembut, attbbut, hattbbut, rowbut, colbut, skip;
	int printed = 0;

	char *but[] = { "F2  Print ",
					"F10  Exit " };
	char *help[] = {"Prints the Ticket.",
					"Press ENTER or ESC to exit" };

	/* Screen Design */
	scrfil(' ', ATTB);
	box(0,0,79,24,0x38,-1);
	pstr(" Reservation Cum Confirm ",2,27,0x2F);
	pstr("Date:",3,4,0x3F);
	pstr("Mode:",3,60,0x3F);
	textline(4,1,78,0x38);
	pchar(4,0,'Ã',0x38);
	pchar(4,79,'´ ',0x38);
	pstr("PNR No",6,2,ATTB);
	pstr("Flight No",6,28,ATTB);
	pstr("Sector",6,51,ATTB);
	pstr("Departure Date",8,2,ATTB);
	pstr("Departure Time",8,51,ATTB);
	pstr("Total No of Seat",10,2,ATTB);
	pstr("Telephone",10,51,ATTB);
	ipblock(10,61,PHONELT,BATTB);
	cells();
	dispdate(3,10,0x3F);

	attbbut = 0x09;
	hattbbut = 0x6F;
	rowbut = 22;
	colbut = 20;
	skip = 28;
	for (i=0, j=colbut; i<=1; i++, j+=skip)
		pstr(but[i], rowbut, j, attbbut);

	/* if ESC hit in middle of Confirmation Entry */
	if (confirmf(conf_index, md_recno) == 0)
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
				pstr("YOU SHOULD PRESS THIS BUTTON FOR THIS RECORD TO UPDATE",\
				23,25,0x9E);
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
				pstr("YOU SHOULD PRESS THIS BUTTON FOR THIS RECORD TO UPDATE",\
				23,25,0x9E);
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


/* Takes confirmation details. Returns 0, if job breaks in middle,
1 if job completes successfully */
int confirmf(int conf_index, int md_recno)
{
	short seatno, seatip[TOT_SEATLT];
	char c, temp[8];
	int i, j, attb, row, Fseats, Bseats, Eseats;
	float tot_fare = 0.0;

	pstr("CONFIRM",3,67,0x3F);

	for (i=0; i < MAXCONFIRMS; i++)
		if (confirm[i].status == 0)
			break;
	if (i == MAXCONFIRMS)	{
		msgbox("CONFIRM.DAT Full. No more entries possible.", 0);
		return 0;
	}
	cindex=i; /* first free record */

	/* copy flino and PNR No. from structure enq to sturcture confirm */
	strcpy(confirm[cindex].pnr, enq[eindex].pnr);
	confirm[cindex].flino = enq[eindex].flino;
	/* copy departure date */
	confirm[cindex].depdate.day = enq[eindex].depdate.day;
	confirm[cindex].depdate.month = enq[eindex].depdate.month;
	confirm[cindex].depdate.year = enq[eindex].depdate.year;
	/* copy departure time */
	confirm[cindex].deptime = enq[eindex].deptime;

	/* print PNR No. , flight no. etc. */
	pstr(confirm[cindex].pnr, 6, 9, BATTB);
	pstr(flight[confirm[cindex].flino].flino,6,38,BATTB);
	pstr(flight[confirm[cindex].flino].sector,6,58,BATTB);
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
			case '1': confirm[cindex].totseat = 1; break;
			case '2': confirm[cindex].totseat = 2; break;
			case '3': confirm[cindex].totseat = 3; break;
			case '4': confirm[cindex].totseat = 4; break;
			default: confirm[cindex].totseat = 0; break;
		}
		if (confirm[cindex].totseat < 1 || confirm[cindex].totseat > 4)
			msgbox("Total seats should be in between 1 and 4",0);
		else
			break;
	}
	pchar(10,19,c,BATTB);

	/* get telephone no */
	for(;;) 	{
		ipblock(10,61,PHONELT,HATTB);
		statbar("Enter Passenger's Telephone No.");
		if (!(getip(10,61,confirm[cindex].phone,PHONELT)))
			if ((msgbox("Do you want to leave?",1)) == 'Y')
				return 0;
			else
				continue;
		break;
	}
	ipblock(10,61,PHONELT,BATTB);
	pstr(confirm[cindex].phone,10,61,BATTB);

	for (j=0, row=16; j < confirm[cindex].totseat; j++, row++)	{
		pstr("     ³       ³       ³                         ³   ³   ³      ³   ³         "\
		,row,2,0x0F);
		getclass: /* get class */
		do	{
			pchar(row,4,' ',HATTB);
			statbar("F - First Class      B - Business Class       E - Economy Class");
			gotoxy(5,row+1);
			if ((c = toupper(getch()) ) == ESC)
				if ((msgbox("Do you want to leave?",1)) == 'Y')
					return 0;
		}while (c != 'F' && c != 'B' && c != 'E');
		pchar(row,4,c,HATTB);
		confirm[cindex].clas[j] = c;

		/* get seating option */
		do	{
			pchar(row,11,' ',HATTB);
			statbar("S - Smoking Seat          N - Non Smoking Seat");
			gotoxy(12,row+1);
			if ((c = toupper(getch()) ) == ESC)
				if ((msgbox("Do you want to leave?",1)) == 'Y')
					return 0;
		}while (c != 'S' && c != 'N');
		pchar(row,11,c,HATTB);
		confirm[cindex].so[j] = c;

		/* generate seat no */
		/* total F class seats */
		Fseats = mdetail[md_recno].seats[0][2] + mdetail[md_recno].seats[0][3];
		/* total B class seats */
		Bseats = mdetail[md_recno].seats[1][2] + mdetail[md_recno].seats[1][3];
		/* total E class seats */
		Eseats = mdetail[md_recno].seats[2][2] + mdetail[md_recno].seats[2][3];

		switch(confirm[cindex].clas[j])	{
		case 'F':
			if (confirm[cindex].so[j] == 'N')	{	/* if Non-Smoking */
				/* refer number of Non-Smoking seats in mdetail */
				for (i=0; i < mdetail[md_recno].seats[0][3]; i++)
					if (conf[conf_index].confirmed[i] == 0)	{
						conf[conf_index].confirmed[i] = i+1;
						textattr(HATTB);
						gotoxy(19,row+1);
						cprintf("%d", conf[conf_index].confirmed[i]);
						break;
					}
				if (i == mdetail[md_recno].seats[0][3])	{
					msgbox("All seats reserved. Try other class", 0);
					pchar(row,11,' ',HATTB);
					goto getclass;
				}
			}
			if (confirm[cindex].so[j] == 'S')	{  /* if Smoking */
				/* refer number of smoking seats in mdetail */
				/* Start searching from the end of Non-Smoking seat */
				for (i=mdetail[md_recno].seats[0][3];i < Fseats; i++)
					if (conf[conf_index].confirmed[i] == 0)	{
						conf[conf_index].confirmed[i] = i+1;
						textattr(HATTB);
						gotoxy(19,row+1);
						cprintf("%d", conf[conf_index].confirmed[i]);
						break;
					}
				if (i == Fseats)	{
					msgbox("All seats reserved. Try other class",0);
					pchar(row,11,' ',HATTB);
					goto getclass;
				}
			}
			break;

		case 'B':
			if (confirm[cindex].so[j] == 'N')	{	/* if Non-Smoking */
				/* refer number of Non-Smoking seats in mdetail */
				/* start searching from the end of F class */
				for (i=Fseats; i < Fseats + mdetail[md_recno].seats[1][3]; i++)
					if (conf[conf_index].confirmed[i] == 0)	{
						conf[conf_index].confirmed[i] = i+1;
						textattr(HATTB);
						gotoxy(19,row+1);
						cprintf("%d", conf[conf_index].confirmed[i]);
						break;
					}
				if (i == Fseats + mdetail[md_recno].seats[1][3])	{
					msgbox("All seats reserved. Try other class", 0);
					pchar(row,11,' ',HATTB);
					goto getclass;
				}
			}
			if (confirm[cindex].so[j] == 'S')	{  /* if Smoking */
				/* refer number of smoking seats in mdetail */
				/* start searching from the end of Non-Smoking seat */
				for (i= Fseats + mdetail[md_recno].seats[1][3];i < Fseats + Bseats; i++)
					if (conf[conf_index].confirmed[i] == 0)	{
						conf[conf_index].confirmed[i] = i+1;
						textattr(HATTB);
						gotoxy(19,row+1);
						cprintf("%d", conf[conf_index].confirmed[i]);
						break;
					}
				if (i == Fseats + Bseats)	{
					msgbox("All seats reserved. Try other class",0);
					pchar(row,11,' ',HATTB);
					goto getclass;
				}
			}
			break;

		case 'E':
			if (confirm[cindex].so[j] == 'N')	{	/* if Non-Smoking */
				/* start searching from the end of B class */
				for (i = Fseats + Bseats; i < Fseats + Bseats + mdetail[md_recno].\
				seats[2][3]; i++)
					if (conf[conf_index].confirmed[i] == 0)	{
						conf[conf_index].confirmed[i] = i+1;
						textattr(HATTB);
						gotoxy(19,row+1);
						cprintf("%d", conf[conf_index].confirmed[i]);
						break;
					}
				if (i == Fseats + Bseats + mdetail[md_recno].seats[2][3])	{
					msgbox("All seats reserved. Try other class", 0);
					pchar(row,11,' ',HATTB);
					goto getclass;
				}
			}
			if (confirm[cindex].so[j] == 'S')	{  /* if Smoking */
				/* refer number of smoking seats in mdetail */
				/* start searching from the end of Non-Smoking seat */
				for (i= Fseats + Bseats + mdetail[md_recno].seats[2][3]; i < Fseats + \
				Bseats + Eseats; i++)
					if (conf[conf_index].confirmed[i] == 0)	{
						conf[conf_index].confirmed[i] = i+1;
						textattr(HATTB);
						gotoxy(19,row+1);
						cprintf("%d", conf[conf_index].confirmed[i]);
						break;
					}
				if (i == Fseats + Bseats + Eseats)	{
					msgbox("All seats reserved. Try other class",0);
					pchar(row,11,' ',HATTB);
					goto getclass;
				}
			}
			break;
		}
		confirm[cindex].seatno[j] = i+1; /* store in seat no */

		textattr(HATTB);
		/* get name */
		do 	{
			statbar("Enter Passenger's Name");
			if (!(getip(row,24,confirm[cindex].name[j],NAMELT)))
				if ((msgbox("Do you want to leave?",1)) == 'Y')
					return 0;
				else {
					confirm[cindex].name[j][0] = '\0';
					continue;
				}
		}while (confirm[cindex].name[j][0] == '\0');
		pstr(confirm[cindex].name[j],row,24,HATTB);

		/* get age */
		do 	{
			statbar("Enter Passenger's Age");
			if (!(getip(row,50,temp,4)))
				if ((msgbox("Do you want to leave?",1)) == 'Y')
					return 0;
				else {
					temp[0] = '\0';
					continue;
				}
			/* converts char string to int */
			if (!(confirm[cindex].age[j] = atoi(temp)))	{
				temp[0] = '\0'; /* not valid input */
				msgbox("Age field should contain only numbers",0);
			}
		}while (temp[0] == '\0');
		gotoxy(51,row+1);
		cprintf("%d",confirm[cindex].age[j]); /* pstr() not usmed because phoneip
		prints succeeding characters if any */

		/* get sex */
		do	{
			statbar("M - Male          F - Female");
			gotoxy(56,row+1);
			if ((c = toupper(getch()) ) == ESC)
				if ((msgbox("Do you want to leave?",1)) == 'Y')
					return 0;
		}while (c != 'M' && c != 'F');
		pchar(row,55,c,HATTB);
		confirm[cindex].sex[j] = c;

		/* get meals option */
		do	{
			statbar("V - Veg             N - Non-Veg");
			gotoxy(61,row+1);
			if ((c = toupper(getch()) ) == ESC)
				if ((msgbox("Do you want to leave?",1)) == 'Y')
					return 0;
		}while (c != 'V' && c != 'N');
		pchar(row,60,c,HATTB);
		confirm[cindex].mo[j] = c;

		/* get SSR option */
		do	{
			statbar("Special Service Request(SSR) required? Y/N");
			gotoxy(67,row+1);
			if ((c = toupper(getch()) ) == ESC)
				if ((msgbox("Do you want to leave?",1)) == 'Y')
					return 0;
		}while (c != 'Y' && c != 'N');
		pchar(row,66,c,HATTB);
		confirm[cindex].ssr[j] = c;

		/* print fare */
		gotoxy(70,row+1);
		switch(confirm[cindex].clas[j]) {
		case 'F':
			confirm[cindex].fare[j] = flight[enq[eindex].flino].f_fare;
			tot_fare += flight[enq[eindex].flino].f_fare;
			break;
		case 'B':
			confirm[cindex].fare[j] = flight[enq[eindex].flino].b_fare;
			tot_fare += flight[enq[eindex].flino].b_fare;
			break;
		case 'E':
			confirm[cindex].fare[j] = flight[enq[eindex].flino].e_fare;
			tot_fare += flight[enq[eindex].flino].e_fare;
			break;
		}
		cprintf("%9.2d", confirm[cindex].fare[j]);

		/* clear the highlighted bar */
		pstr("     ³       ³       ³                         ³   ³   ³      ³   ³         "\
		,row,2,0x1F);
		/* reprint all datas */
		attb=0x1B;
		textattr(attb);
		pchar(row,4,confirm[cindex].clas[j],attb);
		pchar(row,11,confirm[cindex].so[j],attb);
		gotoxy(19,row+1);
		cprintf("%d",confirm[cindex].seatno[j]);
		pstr(confirm[cindex].name[j],row,24,attb);
		gotoxy(51,row+1);
		cprintf("%d",confirm[cindex].age[j]);
		pchar(row,55,confirm[cindex].sex[j],attb);
		pchar(row,60,confirm[cindex].mo[j],attb);
		pchar(row,66,confirm[cindex].ssr[j],attb);
		gotoxy(70,row+1);
		cprintf("%9.2f", confirm[cindex].fare[j]);
	}

	/* print total fare */
	gotoxy(70,22);
	cprintf("%9.2f",tot_fare);
	cprintf(" ");
	gotoxy(1,1); /* send cursor from middle of screen */

	pstr("       ",3,67,0x3F);
	return 1;
}


void print(void)
{
}


/* Opens CONFIRM.DAT file for global access */
void confirmdata(void)
{
	/* read enquiry data */
	if ((confirm_fp = fopen("confirm.dat", "r")) != NULL)
		fread(confirm, sizeof(struct reservation), MAXCONFIRMS, confirm_fp);
	else if ( (confirm_fp = fopen("confirm.dat", "w")) != NULL )
		msgbox("File does not exist:\"CONFIRM.DAT\". New file created", 0);
	else	{
		errbeep();
		msgbox("File creation error:\"CONFIRM.DAT\"", 0);
		exit(1);
	}
}


/* generates a cell box */
void cells(void)
{
	const int row=12, col=1;
	int r=row, c=col, cellattb = 0x1F, attb=0x1A;

	box(c,r,78,row+8,cellattb,-1);
	/* fill box */
	for (r = row+1; r <= row+7; r++)
		for(c=col+1; c < 78; c++)
			pchar(r,c,' ',cellattb);
	r = row, c = col;
	textline(r+3,c+1,77,cellattb); /* horizontal line */
	pchar(r+3,c,'Ã',cellattb);
	pchar(r+3,78,'´',cellattb);
	pstr("Class Seating Seat No     Name of Passenger     Age Sex Meals  SSR   Fare",\
	r+1,c+1,attb);
	pstr("      Option                                            Option Req          ",\
	r+2,c+1,attb);
	vertline(7);
	vertline(15);
	vertline(23);
	vertline(49);
	vertline(53);
	vertline(57);
	vertline(64);
	vertline(68);
	pstr("Total",row+9,63,0x30);
}


/* prints a verticle line in the column c */
void vertline(int c)
{
	int r, cellattb = 0x1F;
	pchar(12,c,'Â',cellattb);
	for(r=13; r<20; r++)
		pchar(r,c,'³',cellattb);
	pchar(15,c,'Å',cellattb);
	pchar(20,c,'Á',cellattb);
}
