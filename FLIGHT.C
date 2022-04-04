/* Flight Details Entry */
#include <stdio.h>
#include <conio.h>
#include <string.h> /* for strcmpi */
#include <stdlib.h> /* for atof & malloc() */
#include"defs.h" /* local file containing define statements & function declarations */

#define	ROW		6		/* Row Col position of Flight No */
#define	COL		22
#define	BATTB	0X74	/* Block attribute */
#define	HATTB	0X5E    /* Highlight attribute */
#define	ATTB	0X1A    /* Normal attribute */

/* The constants MAXAIRCRAFTS, MAXNAME, MAXCODE and MAXSEATDIG used here are
defined in defs.h for Master Entry */

extern int index;
extern struct airmaster mast[];
extern int mdindex;
extern struct mastdetail mdetail[];

static void add(void);
static void modify(void);
static void view(void);
static void save(void);
static void clrblocks(void);
static void disprecord(int);
static void showlist(int row, int col);

struct airflight flight[MAXFLIGHTS];
static FILE *fp;
int findex; /* next free position in flight structure */

void flightentry(void)
{
	int i, j, itembut, attbbut, hattbbut, rowbut, colbut, skip;
	char *but[]={	"F2    Add",
					"F4 Modify",
					"F5   View",
					"F6   Save",
					"F10  Exit"	};
	char *help[]={	"Add a record",
					"Modify a record",
					"Displays a record. Use UP/DOWN keys to scroll",
					"Save all records to FLIGHT.DAT",
					"Press Enter or Esc to Exit"	};

	/* Screen design */
	scrfil(' ', ATTB);
	box(0,0,79,24,0x1B,-1);
	pstr(" Flight Detail Entry ", 2, 29, 0x3F);
	pstr("Date:", 3, 4, 0x17);
	pstr("Mode:", 3, 60, 0x17);
	textline(4,1,78,0x1B);
	pchar(4,0,'Ã',0x1B);
	pchar(4,79,'´',0x1B);
	pstr("Flight No", ROW, COL-10, ATTB);
	pstr("Sector", ROW, COL+FLINOLT+21, ATTB);
	pstr("Operating Days", ROW+2, COL-15, ATTB);
	pstr("Departure Time", ROW+4, COL-15, ATTB);
	pstr("Arrival Time", ROW+4, COL+DEP_DIG+15, ATTB);
	pstr("First Class Fare", ROW+6, COL-17, ATTB);
	pstr("Aircraft Code", ROW+6, COL+F_FAREDIG+10, 0x1E); /* Foreign key */
	pstr("Business Class Fare", ROW+8, COL-20, ATTB);
	pstr("Economy Class Fare", ROW+10, COL-19, ATTB);
	clrblocks(); /* displays all input blocks */
	textline(19,1,78,0x1B);
	pchar(19,0,'Ã',0x1B);
	pchar(19,79,'´',0x1B);
	dispdate(3,10,0x1F);
	gotoxy(1,1); /* move cursor from middle of screen */

	attbbut = 0x3E;
	hattbbut = 0x6F;
	rowbut = 21;
	colbut = 7;
	skip = 14;
	for (i=0, j=colbut; i<=4; i++, j+=skip)
		pstr(but[i], rowbut, j, attbbut);
	itembut = 0;
	pstr(but[itembut], rowbut, colbut, hattbbut); /* highlight first button */
	statbar(help[itembut]);
	for (;;)	{
		switch (scan())	{
		case RIGHT:
		case TAB:
			pstr(but[itembut], rowbut, colbut, attbbut);
			if (itembut == 4)	/* if at last button */
				itembut=0, colbut=7;
			else
				itembut++, colbut += skip;
			pstr(but[itembut], rowbut, colbut, hattbbut);
			statbar(help[itembut]);
			break;
		case LEFT:
		case SHTAB:
			pstr(but[itembut], rowbut, colbut, attbbut);
			if (itembut == 0)	/* if at first button */
				itembut=4, colbut=63;
			else
				itembut--, colbut -= skip;
			pstr(but[itembut], rowbut, colbut, hattbbut);
			statbar(help[itembut]);
			break;
		case F2:
			pstr(but[itembut], rowbut, colbut, attbbut);
			add();
			pstr(but[itembut], rowbut, colbut, hattbbut);
			break;
		case F4:
			pstr(but[itembut], rowbut, colbut, attbbut);
			modify();
			pstr(but[itembut], rowbut, colbut, hattbbut);
			break;
		case F5:
			pstr(but[itembut], rowbut, colbut, attbbut);
			view();
			pstr(but[itembut], rowbut, colbut, hattbbut);
			break;
		case F6:
			pstr(but[itembut], rowbut, colbut, attbbut);
			save();
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
					add();
					pstr(but[itembut], rowbut, colbut, hattbbut);
					break;
				case 1:
					pstr(but[itembut], rowbut, colbut, attbbut);
					modify();
					pstr(but[itembut], rowbut, colbut, hattbbut);
					break;
				case 2:
					pstr(but[itembut], rowbut, colbut, attbbut);
					view();
					pstr(but[itembut], rowbut, colbut, hattbbut);
					break;
				case 3:
					pstr(but[itembut], rowbut, colbut, attbbut);
					save();
					pstr(but[itembut], rowbut, colbut, hattbbut);
					break;
				case 4:
					pstr("      ", 3, 67, 0x1F);
					return;
			}
			break;
		}
	}
}


/* adds new record */
void add(void)
{
	int i, ret, hour, min;
	char fareip[F_FAREDIG], timeip[DEP_DIG]; /* to store input temporarily */
	char temp[MAXCODE]; /* for input of Aircraft code */
	char *p;

	pstr("ADD   ",3,67,0x1F);
	for (;;)	{
		if (findex == MAXFLIGHTS)	{
			putchar('\a');
			msgbox("No More Flights Addition Possible", 0);
			return ;
		}
		clrblocks(); /* clears all input blocks */

		/* get Flight No. */
		for (;;)	{
			ipblock(ROW, COL, FLINOLT, HATTB);
			statbar("PRIMARY KEY: Enter Flight No.");
			textattr(HATTB);
			if (!getip(ROW, COL, flight[findex].flino, FLINOLT))	{
			/* if ESC pressed from within firsr filed, exit current mode */
				ipblock(ROW, COL, FLINOLT, BATTB);
				pstr("      ", 3, 67, 0x1F);
				statbar("");
				gotoxy(1,1); /* take off cursor also */
				return;
			}
			if (flight[findex].flino[0] == '\0')	{
				errbeep();
				msgbox("Primary Key shouldn't be NULL", 0);
				continue;
			}
			for (i=0; i<findex; i++) /* check for duplicates */
				if (strcmpi(flight[findex].flino, flight[i].flino) == 0)	{
					errbeep();
					msgbox("Flight No. already exists", 0);
					break;
				}
			if (i != findex)
				continue; /* re-enter Flight No. */
			ipblock(ROW, COL, FLINOLT, BATTB);
			pstr(flight[findex].flino, ROW, COL, BATTB);
			break; /* all is well */
		}

		/* get sector */
		do	{
			ret = 1;
			ipblock(ROW, COL+F_FAREDIG+24, SECTORLT, HATTB);
			statbar("Enter Sector. For eg. enter \"Delhi-Mumbai\" for the sector Delhi-Mumbai-Delhi");
			if (!(ret=getip(ROW, COL+F_FAREDIG+24, flight[findex].sector, SECTORLT)))
				break;
		}while (flight[findex].sector[0] == '\0');
		if (ret == 0) /* re-enter from beginning */
			continue;
		ipblock(ROW, COL+F_FAREDIG+24, SECTORLT, BATTB);
		pstr(flight[findex].sector, ROW, COL+F_FAREDIG+24, BATTB);

		/* get operating days */
		do	{
			ret = 1;
			ipblock(ROW+2, COL, OP_DAYSLT, HATTB);
			statbar("Enter Days operated in the week");
			if (!(ret=getip(ROW+2, COL, flight[findex].op_days, OP_DAYSLT)))
				break;
		}while (flight[findex].op_days[0] == '\0');
		if (ret == 0) /* re-enter from beginning */
			continue;
		ipblock(ROW+2, COL, OP_DAYSLT, BATTB);
		pstr(flight[findex].op_days, ROW+2, COL, BATTB);

		/* get departure time */
		do	{
			ret = 1;
			ipblock(ROW+4, COL, DEP_DIG, HATTB);
			statbar("Enter local Deaprture Time in 24 hour format like 22.30");
			if (!(ret=getip(ROW+4, COL, timeip, DEP_DIG)))
				break; /* time is taken as float */
			if (!(flight[findex].dep = atof(timeip))) /* convert char string to float */
				timeip[0]='\0'; /* not a valid string */
			/* seperate time into 2 parts */
			hour = flight[findex].dep * 100;
			min = hour % 100;
			hour /= 100;
			if (hour < 0 || min < 0 || hour > 23 || min > 59)	{
				errbeep();
				msgbox("Invalid Time", 0);
				timeip[0]='\0'; /* re-loop */
			}
		}while (timeip[0] == '\0');
		if (ret == 0) /* re-enter from beginning */
			continue;
		ipblock(ROW+4, COL, DEP_DIG, BATTB);
		pstr(timeip, ROW+4, COL, BATTB);

		/* get Arrival time */
		do	{
			ret = 1;
			ipblock(ROW+4, COL+F_FAREDIG+24, ARRIV_DIG, HATTB);
			statbar("Enter local Arrival Time in 24 hour format like 22.30");
			if (!(ret=getip(ROW+4, COL+F_FAREDIG+24, timeip, ARRIV_DIG)))
				break; /* time is taken as float */
			/* convert char string to float */
			if (!(flight[findex].arriv = atof(timeip)))
				timeip[0]='\0'; /* not a valid string */
			/* seperate time into 2 parts */
			hour = flight[findex].arriv * 100;
			min = hour % 100;
			hour /= 100;
			if (hour < 0 || min < 0 || hour > 23 || min > 59)	{
				errbeep();
				msgbox("Invalid Time", 0);
				timeip[0]='\0'; /* re-loop */
			}
		}while (timeip[0]  == '\0');
		if (ret == 0) /* re-enter from beginning */
			continue;
		ipblock(ROW+4, COL+F_FAREDIG+24, ARRIV_DIG, BATTB);
		pstr(timeip, ROW+4, COL+F_FAREDIG+24, BATTB);

		/* get first class fare */
		do	{
			ret = 1;
			ipblock(ROW+6, COL, F_FAREDIG, HATTB);
			statbar("Enter Fare for First Class Travel");
			if (!(ret=getip(ROW+6, COL, fareip, F_FAREDIG)))
				break;
			/* convert char string to float */
			if (!(flight[findex].f_fare = atof(fareip)))
				fareip[0]='\0'; /* not a valid string */
			if (flight[findex].f_fare > 999999)	{ /* max 6 digits for integral part */
				msgbox("Beyond Range", 0);
				fareip[0]='\0';
			}
		}while (fareip[0] == '\0');
		if (ret == 0) /* re-enter from beginning */
			continue;
		ipblock(ROW+6, COL, F_FAREDIG, BATTB);
		pstr(fareip, ROW+6, COL, BATTB);

		/* get Business class fare */
		do	{
			ret = 1;
			ipblock(ROW+8, COL, B_FAREDIG, HATTB);
			statbar("Enter Fare for Business Class Travel");
			if (!(ret=getip(ROW+8, COL, fareip, B_FAREDIG)))
				break;
			/* convert char string to float */
			if (!(flight[findex].b_fare = atof(fareip)))
				fareip[0]='\0'; /* not a valid string */
			if (flight[findex].b_fare > 999999)	{ /* max 6 digits for integral part */
				msgbox("Beyond Range", 0);
				fareip[0]='\0';
			}
		}while (fareip[0] == '\0');
		if (ret == 0) /* re-enter from beginning */
			continue;
		ipblock(ROW+8, COL, B_FAREDIG, BATTB);
		pstr(fareip, ROW+8, COL, BATTB);

		/* get Economy class fare */
		do	{
			ret = 1;
			ipblock(ROW+10, COL, E_FAREDIG, HATTB);
			statbar("Enter Fare for Economy Class Travel");
			if (!(ret=getip(ROW+10, COL, fareip, E_FAREDIG)))
				break;
			/* convert char string to float */
			if (!(flight[findex].e_fare = atof(fareip)))
				fareip[0]='\0'; /* not a valid string */
			if (flight[findex].e_fare > 999999)	{ /* max 6 digits for integral part */
				msgbox("Beyond Range", 0);
				fareip[0]='\0';
			}
		}while (fareip[0] == '\0');
		if (ret == 0) /* re-enter from beginning */
			continue;
		ipblock(ROW+10, COL, E_FAREDIG, BATTB);
		pstr(fareip, ROW+10, COL, BATTB);

		/* get Aircraft Code */
		if ((p = (char *)malloc(8*22*2)) == NULL)	{
			fprintf(stderr,"Not Enough Memory");
			exit(1);
		}
		savescr(1,5,1+21,5+MAXCODE+2,p);
		for (;;)	{
			ret = 1;
			ipblock(ROW+6, COL+F_FAREDIG+24, MAXCODE, HATTB);
			statbar("FOREIGN KEY: Select Aricraft Code for this flight from the list");
			gotoxy(COL+F_FAREDIG+24+1, ROW+6+1);
			showlist(1,5); /* shows aircraft list */
			if (!(ret = getip(ROW+6, COL+F_FAREDIG+24, temp, MAXCODE)))
				break;
			if (temp[0] == '\0')
				continue;
			for (i=0; i<index; i++) /* check if code exists */
				if (strcmpi(temp, mast[i].code) == 0)
					break;
			if (i == index)	{
				msgbox("Aircraft Code not found in Master. Assign different Aircraft",0);
				continue; /* re-enter Aircraft Code */
			}
			flight[findex].aircode = i; /* new value */
			for (i=0; i<mdindex; i++) /* check if it is in mdetail */
				if (flight[findex].aircode == mdetail[i].aircode)
					break;
			if (i == mdindex) {
				msgbox("Details for this aircraft not entered in MASTER DETAIL ENTRY", 0);
				continue; /* re-enter Aircraft Code */
			}
			ipblock(ROW+6, COL+F_FAREDIG+24, MAXCODE, BATTB);
			pstr(mast[i].code, ROW+6, COL+F_FAREDIG+24, BATTB);
			break;
		}
		restorescr(1,5,1+21,5+MAXCODE+2,p);
		free(p);
		if (ret == 0)
			continue;

		findex++; /* Current record input complete */
	}
}


/* saves FLIGHT.DAT */
void save(void)
{
	pstr("SAVE   ",3,67,0x1F);
	if ((fp = fopen("flight.dat","w")) != NULL)	{
		fwrite(&flight, sizeof(struct airflight), findex, fp);
		statbar("FLIGHT.DAT Saved");
		while (!kbhit());
	}
	else
		msgbox("Error opening file: FLIGHT.DAT", 0);
	pstr("       ",4,70,0x1F);
	fclose(fp);
	return;
}


/* allows user to modify record */
void modify(void)
{
	int i, j, k, ret, hour, min;
	int sig=4; /* significant digits *; for use of gcvt */
	char str[FLINOLT], timestr[DEP_DIG], fareip[F_FAREDIG];
	char temp[25]; /* to hold orinal field value before modification */
	char *p;

	pstr("MODIFY ",3,67,0x1F);
	if (findex == NULL)	{
		errbeep();
		msgbox("No Records", 0);
		return;
	}
	for (;;)	{
		clrblocks();
		ipblock(ROW, COL, FLINOLT, HATTB);
		statbar("Enter FLIGHT NO. to Modify");
		textattr(HATTB);
		if (!getip(ROW,COL,str,FLINOLT))	{
			ipblock(ROW, COL, FLINOLT, BATTB);
			break;
		}
		/* check if code exists */
		for (i=0; i<findex; i++)
			if (strcmpi(flight[i].flino, str) == 0)
				break;
		if (i==findex)	{
			errbeep();
			msgbox("Record Not Found", 0);
			continue;
		}
		/* Record found at i */
		disprecord(i);

		/* Modify FLIGHT NO */
		do	{
			ipblock(ROW, COL, FLINOLT, HATTB);
			pstr(flight[i].flino, ROW, COL, HATTB);
			statbar("Modify FLIGHT NO. Press ESC to Exit");
			strcpy(temp, flight[i].flino); /* save original */
			ret = modi(flight[i].flino, FLINOLT, ROW, COL, HATTB);
			if (ret == -1)	{ /* if ESC hit before modification come out of
			MODIFY mode */
				clrblocks();
				ipblock(ROW, COL, FLINOLT, BATTB);
				pstr("      ", 3, 67, 0x1F);
				statbar("");
				gotoxy(1,1); /* take off cursor also */
				break;
			}
			if (ret == 0)	{ /* if ESC hit after any modification restore
			original field value */
				strcpy(flight[i].flino, temp);
				continue;
			}
			/* primary key shoudn't be NULL */
			if (flight[i].flino[0] == '\0')	{
				errbeep();
				msgbox("Primary Key value may not be NULL", 0);
				continue;
			}
			/* check for duplicates */
			for (j=0; j<findex; j++)	{
				if (j==i)
					continue; /* skip current index */
				if (strcmpi(flight[i].flino, flight[j].flino) == 0)	{
					errbeep();
					msgbox("FLIGHT NO Already Exists", 0);
					break;
				}
			}
		}while (j != findex); /* until the modified FLIGHT NO isn't a duplicate copy */
		if (ret == -1)
			break; 	/* continuation of control from the above loop */
		ipblock(ROW, COL, FLINOLT, BATTB);
		pstr(flight[i].flino, ROW, COL, BATTB);

		/* modify sector */
		do {
			ipblock(ROW, COL+F_FAREDIG+24, SECTORLT, HATTB);
			statbar("Modify Sector");
			pstr(flight[i].sector, ROW, COL+F_FAREDIG+24, HATTB);
			strcpy(temp, flight[i].sector);
			ret = modi(flight[i].sector, SECTORLT, ROW, COL+F_FAREDIG+24, HATTB);
			if (ret == -1)	/* if ESC hit before modification, Re-enter record */
				break;
			if (ret == 0)/* if ESC hit in between modification, restore original value*/
				strcpy(flight[i].sector, temp);
		}while (ret != 1 || flight[i].sector[0] == '\0');
		if (ret == -1)
			continue; /* re-enter record (continuation of control from above loop) */
		ipblock(ROW, COL+F_FAREDIG+24, SECTORLT, BATTB);
		pstr(flight[i].sector, ROW, COL+F_FAREDIG+24, BATTB);

		/* modify operating days */
		do {
			ipblock(ROW+2, COL, OP_DAYSLT, HATTB);
			statbar("Modify Operating Days");
			pstr(flight[i].op_days, ROW+2, COL, HATTB);
			strcpy(temp, flight[i].op_days);
			ret = modi(flight[i].op_days, OP_DAYSLT, ROW+2, COL, HATTB);
			if (ret == -1)	/* if ESC hit before modification, Re-enter record */
				break;
			if (ret == 0)/* if ESC hit in between modification, restore original value*/
				strcpy(flight[i].op_days, temp);
		}while (ret != 1 || flight[i].op_days[0] == '\0');
		if (ret == -1)
			continue; /* re-enter record (continuation of control from above loop */
		ipblock(ROW+2, COL, OP_DAYSLT, BATTB);
		pstr(flight[i].op_days, ROW+2, COL, BATTB);

		/* modify Departure time */
		do 	{
			ipblock(ROW+4, COL, DEP_DIG, HATTB);
			statbar("Modify local Departure Time. Time should be in 24 hours format like 22.30");
			gcvt(flight[i].dep, sig, timestr);
			pstr(timestr, ROW+4, COL, HATTB);
			strcpy(temp, timestr);
			ret = modi(timestr, DEP_DIG, ROW+4, COL, HATTB);
			if (ret == -1)	/* if ESC hit before modification, Re-enter record */
				break;
			if (ret == 0) /* if ESC hit in between modification,
			restart modification with original value */
				continue;
			if (! (flight[i].dep = atof(timestr)))	{
				timestr[0] = '\0';
				continue;
			}
			/* seperate time into 2 parts */
			hour = flight[i].dep * 100;
			min = hour % 100;
			hour /= 100;
			if (hour < 0 || min < 0 || hour > 23 || min > 59)	{
				msgbox("Invalid Time", 0);
				timestr[0]='\0'; /* re-loop */
			}
		}while (ret != 1 || timestr[0] == '\0');
		if (ret == -1)
			continue; /* re-enter record (continuation of control from above loop */
		ipblock(ROW+4, COL, DEP_DIG, BATTB);
		pstr(timestr, ROW+4, COL, BATTB);

		/* modify Arrival time */
		do 	{
			ipblock(ROW+4, COL+F_FAREDIG+24, ARRIV_DIG, HATTB);
			statbar("Modify Arrival Time. It should be in 24 hours format like 22.30");
			gcvt(flight[i].arriv, sig, timestr);
			pstr(timestr, ROW+4, COL+F_FAREDIG+24, HATTB);
			strcpy(temp, timestr);
			ret = modi(timestr, ARRIV_DIG, ROW+4, COL+F_FAREDIG+24, HATTB);
			if (ret == -1)	/* if ESC hit before modification, Re-enter record */
				break;
			if (ret == 0) /* if ESC hit in between modification,
			restart modification with original value */
				continue;
			if (! (flight[i].arriv = atof(timestr)))	{
				timestr[0] = '\0';
				continue;
			}
			/* seperate time into 2 parts */
			hour = flight[i].arriv * 100;
			min = hour % 100;
			hour /= 100;
			if (hour < 0 || min < 0 || hour > 23 || min > 59)	{
				msgbox("Invalid Time", 0);
				timestr[0]='\0'; /* re-loop */
			}
		}while (ret != 1 || timestr[0] == '\0');
		if (ret == -1)
			continue; /* re-enter record (continuation of control from above loop */
		ipblock(ROW+4, COL+F_FAREDIG+24, ARRIV_DIG, BATTB);
		pstr(timestr, ROW+4, COL+F_FAREDIG+24, BATTB);

		sig=8;
		/* modify First class fare*/
		do	{
			ipblock(ROW+6, COL, F_FAREDIG, HATTB);
			statbar("Modify First Class Fare");
			gcvt(flight[i].f_fare, sig, fareip);
			pstr(fareip, ROW+6, COL, HATTB);
			strcpy(temp, fareip);
			ret = modi(fareip, F_FAREDIG, ROW+6, COL, HATTB);
			if (ret == -1)	/* if ESC hit before modification, Re-enter record */
				break;
			if (ret == 0) /* if ESC hit in between modification,
			restart modification with original value */
				continue;
			if (! (flight[i].f_fare = atof(fareip)))	{
				fareip[0] = '\0';
				continue;
			}
			if (flight[i].f_fare > 999999)	{ /* max 6 digits for integral part */
				msgbox("Beyond Range", 0);
				fareip[0]='\0';
			}
		}while (ret != 1 || fareip[0] == '\0');
		if (ret == -1)
			continue; /* re-enter record (continuation of control from above loop */
		ipblock(ROW+6, COL, F_FAREDIG, BATTB);
		pstr(fareip, ROW+6, COL, BATTB);

		/* modify Business class fare*/
		do	{
			ipblock(ROW+8, COL, B_FAREDIG, HATTB);
			statbar("Modify Business Class Fare");
			gcvt(flight[i].b_fare, sig, fareip);
			pstr(fareip, ROW+8, COL, HATTB);
			strcpy(temp, fareip);
			ret = modi(fareip, B_FAREDIG, ROW+8, COL, HATTB);
			if (ret == -1)	/* if ESC hit before modification, Re-enter record */
				break;
			if (ret == 0) /* if ESC hit in between modification,
			restart modification with original value */
				continue;
			if (! (flight[i].b_fare = atof(fareip)))	{
				fareip[0] = '\0';
				continue;
			}
			if (flight[i].b_fare > 999999)	{ /* max 6 digits for integral part */
				msgbox("Beyond Range", 0);
				fareip[0]='\0';
			}
		}while (ret != 1 || fareip[0] == '\0');
		if (ret == -1)
			continue; /* re-enter record (continuation of control from above loop */
		ipblock(ROW+8, COL, B_FAREDIG, BATTB);
		pstr(fareip, ROW+8, COL, BATTB);

		/* modify Economy class fare*/
		do	{
			ipblock(ROW+10, COL, E_FAREDIG, HATTB);
			statbar("Modify Economy Class Fare");
			gcvt(flight[i].e_fare, sig, fareip);
			pstr(fareip, ROW+10, COL, HATTB);
			strcpy(temp, fareip);
			ret = modi(fareip, E_FAREDIG, ROW+10, COL, HATTB);
			if (ret == -1)	/* if ESC hit before modification, Re-enter record */
				break;
			if (ret == 0) /* if ESC hit in between modification,
			restart modification with original value */
				continue;
			if (! (flight[i].e_fare = atof(fareip)))	{
				fareip[0] = '\0';
				continue;
			}
			if (flight[i].e_fare > 999999)	{ /* max 6 digits for integral part */
				msgbox("Beyond Range", 0);
				fareip[0]='\0';
			}
		}while (ret != 1 || fareip[0] == '\0');
		if (ret == -1)
			continue; /* re-enter record (continuation of control from above loop */
		ipblock(ROW+10, COL, E_FAREDIG, BATTB);
		pstr(fareip, ROW+10, COL, BATTB);

		/* modify Aircraft Code */
		if ((p = (char *)malloc(8*22*2)) == NULL)	{
			fprintf(stderr,"Not Enough Memory");
			exit(1);
		}
		savescr(1,5,1+21,5+MAXCODE+2,p);
		do	{
			ipblock(ROW+6, COL+F_FAREDIG+24, MAXCODE, HATTB);
			pstr(mast[flight[i].aircode].code, ROW+6, COL+F_FAREDIG+24, HATTB);
			statbar("FOREIGN KEY: Select Aricraft Code for this flight from the list");
			strcpy(temp, mast[flight[i].aircode].code); /* save original */
			showlist(1,5); /* shows aircraft list */
			ret = modi(temp, MAXCODE, ROW+6, COL+F_FAREDIG+24, HATTB);
			if (ret == -1) /* if ESC hit before modification come out of
			MODIFY mode */
				break;
			if (ret == 0) /* if ESC hit after any modification restore
			original field value */
				continue;
			for (j=0; j<index; j++) /* check if code exists */
				if (strcmpi(temp, mast[j].code) == 0)
					break;
			if (j == index)	{
				msgbox("Aircraft Code not found in Master. Assign different Aircraft",0);
				continue; /* re-enter Aircraft Code */
			}
			for (k=0; k<mdindex; k++) /* check if it is in mdetail */
				if (j == mdetail[k].aircode)
					break;
			if (k == mdindex) {
				msgbox("Details for this aircraft not entered in MASTER DETAIL ENTRY",0);
				continue; /* re-enter Aircraft Code */
			}
		/* until the modified AIRCRAFT code not found in Master */
		}while (j==index || k == mdindex);
		restorescr(1,5,1+21,5+MAXCODE+2,p);
		free(p);
		if (ret == -1)
			break; 	/* continuation of control from the above loop */
		flight[i].aircode = j; /* new value */
		ipblock(ROW+6, COL+F_FAREDIG+24, MAXCODE, HATTB);
		pstr(mast[flight[i].aircode].code, ROW+6, COL+F_FAREDIG+24, BATTB);
	}
}


/* allows user to view records */
void view(void)
{
	int i, ind, ch;
	char fno[FLINOLT]; /* to hold the entered string for FLIGHT NO */

	pstr("VIEW   ",3,67,0x1F);
	if (findex == NULL)	{
		msgbox("No Records", 0);
		return ;
	}
	ind=0;
	disprecord(ind);

	for (;;)	{
		gotoxy(COL+1, ROW+1);
		switch(ch=scan())	{
		case UP:
		case DOWN:
			if (ch == DOWN && ind < (findex-1))
			ind++;
			else if (ch == UP && ind > 0)
			ind--;
			disprecord(ind);
			break;

		case HOME:
			ind = 0;
			disprecord(ind);
			break;

		case END:
			if (findex != NULL)	{
				ind = findex-1;
				disprecord(ind);
			}
			break;

		case ESC:
			clrblocks();
			pstr("       ",3,67,0x1F);
			gotoxy(1,1);
			return;
		default:
			if (isalnum(ch))	{
				ipblock(ROW, COL, FLINOLT, HATTB);
				ungetch(ch); /* the char read at switch */
				statbar("Enter the FLIGHT NO to view");
				textattr(HATTB);
				if (!getip(ROW, COL, fno, FLINOLT))	{
					ipblock(ROW, COL, FLINOLT, BATTB);
					return;
				}
				for (i=0; i < findex; i++)	{ /* check if record exists */
					 if (strcmpi(fno, flight[i].flino) == 0)	{
						  ind = i;
						  disprecord(ind);
						  break;
					 }
					 if (fno[0] == '\0')
						return;
				}
				if (i==findex)	{
					putchar('\a');
					msgbox("Record Not Found", 0);
				}
			}
			break;
		}
	}
}


/* displays specified record */
void disprecord(int i)
{
	clrblocks();
	pstr(flight[i].flino, ROW, COL, BATTB);
	pstr(flight[i].sector, ROW, COL+F_FAREDIG+24, BATTB);
	pstr(flight[i].op_days, ROW+2, COL, BATTB);

	/* To display numeric data */
	textattr(BATTB);
	gotoxy(COL+1,ROW+4+1);
	cprintf("%5.2f",flight[i].dep);
	gotoxy(COL+F_FAREDIG+24+1,ROW+4+1);
	cprintf("%5.2f",flight[i].arriv);
	gotoxy(COL+1,ROW+6+1);
	cprintf("%9.2f",flight[i].f_fare);
	gotoxy(COL+1,ROW+8+1);
	cprintf("%9.2f",flight[i].b_fare);
	gotoxy(COL+1,ROW+10+1);
	cprintf("%9.2f",flight[i].e_fare);
	pstr(mast[flight[i].aircode].code, ROW+6, COL+F_FAREDIG+24, BATTB);
}


/* clear all fields */
void clrblocks(void)
{
	ipblock(ROW, COL, FLINOLT, BATTB);
	ipblock(ROW, COL+F_FAREDIG+24, SECTORLT, BATTB);
	ipblock(ROW+2, COL, OP_DAYSLT, BATTB);
	ipblock(ROW+4, COL, DEP_DIG, BATTB);
	ipblock(ROW+4, COL+F_FAREDIG+24, ARRIV_DIG, BATTB);
	ipblock(ROW+6, COL, F_FAREDIG, BATTB);
	ipblock(ROW+6, COL+F_FAREDIG+24, MAXCODE, BATTB);
	ipblock(ROW+8, COL, B_FAREDIG, BATTB);
	ipblock(ROW+10, COL, E_FAREDIG, BATTB);
}


/* shows list of entered Aircrafts */
void showlist(int row, int col)
{
	int i,r,c;

	box(col,row,col+MAXCODE,row+20,0x30,0x00);
	for (r=row+1,c=col+1; r<=row+19; r++)
		for (c=col+1; c<=col+MAXCODE-1; c++)
			pchar(r,c,' ',0x55);
	for (i=0; i<index; i++)
		pstr(mast[i].code,++row,col+1,0x5B);
}


/* Opens FLIGHT.DAT file for global access */
void flightdata(void)
{
	if ((fp = fopen("flight.dat", "r")) != NULL) {
		findex = fread(flight, sizeof(struct airflight), MAXFLIGHTS, fp);
		/* findex: next free structure position */
	}
	else if ( (fp = fopen("flight.dat", "w")) != NULL )
		msgbox("File does not exist:\"FLIGHT.DAT\" New file created", 0);
	else	{
		errbeep();
		msgbox("File creation error:\"FLIGHT.DAT\"", 0);
		return;
	}
}