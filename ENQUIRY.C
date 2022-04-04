/* Reservation Enquiry */
#include<stdio.h>
#include<conio.h>
#include<string.h> /* for strcmpi */
#include<dos.h>    /* for date and time structures */
#include<stdlib.h> /* for atof() */
#include"defs.h" /* local file containing define statements & function declarations */

#define MAXENQ	50
#define ATTB	0x5E
#define HATTB	0x12
#define BATTB	0x31

/* The constants FLINOLT and SECTORLT used here are defined in DEFS.H for
Flight Details Entry */

static void showlist(int row, int col);
static void cells(void);
void vertline(int c);
int query(void);
void update(void);
void wait_update(void);

extern struct airflight flight[]; /* defined in FLIGHT.C */
extern int findex;
extern struct mastdetail mdetail[]; /* defined in mdetail.c */
extern int mdindex;
extern struct reservation confirm[];
extern int cindex;
extern FILE *confirm_fp;
extern struct waits wait[];
extern int windex;
extern FILE *wait_fp;

static FILE *fp;
FILE *conf_fp;
struct res_manifesto enq[MAXENQ];
int eindex; /* next free position in sturcture enq */
struct con conf[MAXCONF_FLIGHTS];

void enquiry(void)
{
	int i, j, itembut, attbbut, hattbbut, rowbut, colbut, skip, status;
	int firsttime, conf_index, md_recno, fno, d,acode, count, Fseats, Bseats, Eseats, r, c;
	char ch, *p;
	struct dosdate_t date, today;
	char *but[] = { "F2     Confirm ",
					"F3 Waiting List",
					"F10       Exit " };
	char *help[] = {"Enter here for a confirmed ticket",
					"Enter here if the passenger has to be placed in Waiting List",
					"ENTER or ESC to exit"};

	/* Screen Design */
	scrfil(' ',ATTB);
	box(0,0,79,24,0x50,-1);
	pstr(" Reservation Enquiry ",2,30,0x1B);
	pstr("Date:",3,4,0x5F);
	pstr("Mode:",3,60,0x5F);
	textline(4,1,78,0x50);
	pchar(4,0,'Ã',0x50);
	pchar(4,79,'´',0x50);
	pstr("Flight No",5,12,ATTB);
	pstr("Sector",5,49,ATTB);
	pstr("Departure Date",7,7,ATTB);
	pstr("Departure Time",7,41,ATTB);
	pstr("Seats Availability",9,32,0xDF);
	dispdate(3,10,0x5F);
	cells();

	attbbut = 0x6F;
	hattbbut = 0x3E;
	rowbut = 21;
	colbut = 6;
	skip = 26;
	for (i=0, j=colbut; i<=2; i++, j+=skip)
		pstr(but[i], rowbut, j, attbbut);

	/* memory allocation to save entire screen */
	if ((p = (char *)malloc(25 * 80 * 2)) == NULL)	{
		printf("Not Enough Memory");
		exit(1);
	}
	/* free all old date's reservations */
	_dos_getdate(&today);
	firsttime=1;
	for (i=0; i < MAXCONF_FLIGHTS; i++)
		if (conf[i].reserve == 1 && conf[i].day < today.day && conf[i].month \
		<= today.month && conf[i].year <= today.year)	{
			if (firsttime)	{
				ch=msgbox("Free all old date's reservations?", 1);
				firsttime=0;
			}
			if (ch=='Y')	{
				conf[i].reserve = 0;
				for (j=0; j<MAXACSEATS; j++)
					conf[i].confirmed[j]=0;
			}
			else
				break;
		}
	/* save conf */
	if ((conf_fp = fopen("conf.dat","w")) != NULL)
		fwrite(conf, sizeof(struct con), MAXCONF_FLIGHTS, conf_fp);
	else
		msgbox("Error opening file: CONF.DAT", 0);
	fclose(conf_fp);

	for (;;)	{
		if (eindex >= MAXENQ)	{
			msgbox("ENQUIRY.DAT Full. No more enquiry operations possible", 0);
			free(p);
			return;
		}
		if (query()== 0)	{ /* gets queries on flight */
			free(p);
			return;
		}

		/* find conf index matching the datas entered above */
		conf_index = -1;
		for (i=0; i < MAXCONF_FLIGHTS; i++)	{
			/* hold index of first free conf */
			if (conf_index == -1 && conf[i].reserve == 0)
				conf_index = i;
			/* hold index of matching conf */
			if (conf[i].reserve == 1 && enq[eindex].flino == conf[i].flino && \
			enq[eindex].depdate.day == conf[i].day && enq[eindex].depdate.month == \
			conf[i].month && conf[i].year == enq[eindex].depdate.year)	{
				conf_index = i;
				break;
			}
		}
		/* if this is the first reservation for the flight */
		if (conf[conf_index].reserve == 0)	{
			conf[conf_index].reserve = 1;
			conf[conf_index].flino = enq[eindex].flino;
			conf[conf_index].day = enq[eindex].depdate.day;
			conf[conf_index].month = enq[eindex].depdate.month;
			conf[conf_index].year = enq[eindex].depdate.year;
		}
		if (conf_index == -1)	{
			msgbox("Can't handle flights anymore for reservation", 0);
			msgbox("Delete any old reservations to make room", 0);
			return;
		}
		/* corresponding AIRCRAFT CODE of FLIGHT NO in MASTER */
		fno = enq[eindex].flino;
		acode = flight[fno].aircode;
		/* find acode in MDETAIL */
		for (md_recno=0; md_recno < mdindex; md_recno++)
			if (acode == mdetail[md_recno].aircode)
				break;

		/* generate PNR No */
		_dos_getdate(&date); /* get date */
		sprintf(enq[eindex].pnr, "HAI%d%d%d%d", date.day, date.month, date.year,\
		eindex+1);

		/* display seats availability */
		/* total F class seats */
		Fseats = mdetail[md_recno].seats[0][2] + mdetail[md_recno].seats[0][3];
		/* total B class seats */
		Bseats = mdetail[md_recno].seats[1][2] + mdetail[md_recno].seats[1][3];
		/* total E class seats */
		Eseats = mdetail[md_recno].seats[2][2] + mdetail[md_recno].seats[2][3];
		textattr(0x0A);
		d=40; /* delay */
		/* No Smoking First Class */
		for (i=count=0; i < mdetail[md_recno].seats[0][3]; i++)
			if (conf[conf_index].confirmed[i] == 0)
					count++;
		gotoxy(62,14);
		cprintf("%3d", j=count);
		delay(d);
		/* Smoking First Class */
		for (count=0, i=mdetail[md_recno].seats[0][3];i < Fseats; i++)
			if (conf[conf_index].confirmed[i] == 0)
				count++;
		gotoxy(47,14);
		cprintf("%3d", count);delay(d);
		/* Total First Class */
		gotoxy(32,14);
		cprintf("%3d", j+count);delay(d);
		/* No Smoking Business Class */
		for (count=0, i=Fseats; i < Fseats + mdetail[md_recno].seats[1][3]; i++)
			if (conf[conf_index].confirmed[i] == 0)
				count++;
		gotoxy(62,16);
		cprintf("%3d", j=count);delay(d);
		/* Smoking Business Class */
		for (count=0, i= Fseats + mdetail[md_recno].seats[1][3];i < Fseats + Bseats; i++)
			if (conf[conf_index].confirmed[i] == 0)
				count++;
		gotoxy(47,16);
		cprintf("%3d",count);   delay(d);
		/* Total Business Class */
		gotoxy(32,16);
		cprintf("%3d",j+count);delay(d);
		/* No Smoking Economy Class */
		for (count=0, i = Fseats + Bseats; i < Fseats + Bseats + \
		mdetail[md_recno].seats[2][3]; i++)
			if (conf[conf_index].confirmed[i] == 0)
				count++;
		gotoxy(62,18);
		cprintf("%3d", j=count);delay(d);
		/* Smoking Economy Class */
		for (count=0, i= Fseats + Bseats + mdetail[md_recno].seats[2][3]; i < \
		Fseats + Bseats + Eseats; i++)
			if (conf[conf_index].confirmed[i] == 0)
				count++;
		gotoxy(47,18);
		cprintf("%3d",count);   delay(d);
		/* Total Economy Class */
		gotoxy(32,18);
		cprintf("%3d",j+count); delay(d);

		/* highlight first button */
		itembut=0, colbut=6;
		pstr(but[itembut], rowbut, colbut, hattbbut); /* highlight first button */
		statbar(help[itembut]);

		for (;;)	{
			status=0;
			switch (scan())	{
			case RIGHT:
			case TAB:
				pstr(but[itembut], rowbut, colbut, attbbut);
				if (itembut == 2)	/* if at last button */
					itembut=0, colbut=6;
				else
					itembut++, colbut += skip;
				pstr(but[itembut], rowbut, colbut, hattbbut);
				statbar(help[itembut]);
				break;
			case LEFT:
			case SHTAB:
				pstr(but[itembut], rowbut, colbut, attbbut);
				if (itembut == 0)	/* if at first button */
					itembut=2, colbut=58;
				else
					itembut--, colbut -= skip;
				pstr(but[itembut], rowbut, colbut, hattbbut);
				statbar(help[itembut]);
				break;
			case F2:
				pstr(but[itembut], rowbut, colbut, attbbut);
				savescr(0,0,24,79,p);
				if (confirmation(conf_index, md_recno) != 0) /* if ticket printed out */
					update();
				else /* if exit in between, re-read conf data */
					if ((conf_fp = fopen("conf.dat", "r")) != NULL)
						fread(conf, sizeof(struct con), MAXCONF_FLIGHTS, conf_fp);
					else	{
						errbeep();
						msgbox("File open error:\"CONF.DAT\"", 0);
						exit(1);
					}
				status=1;
				restorescr(0,0,24,79,p);
				break;
			case F3:
				pstr(but[itembut], rowbut, colbut, attbbut);
				savescr(0,0,24,79,p);
				if (waiting() != 0) /* if ticket printed out */
					wait_update();
				status=1;
				restorescr(0,0,24,79,p);
				break;
			case F10:
			case ESC:
				free(p);
				pstr("         ", 3, 67, 0x5F);
				return;
			case ENTER:
				switch(itembut)	{
					case 0:
						pstr(but[itembut], rowbut, colbut, attbbut);
						savescr(0,0,24,79,p);
						/* if ticket printed out */
						if (confirmation(conf_index, md_recno) != 0)
							update();
						else /* if exit in between, re-read conf data */
							if ((conf_fp = fopen("conf.dat", "r")) != NULL)
								fread(conf, sizeof(struct con), MAXCONF_FLIGHTS, conf_fp);
							else	{
								errbeep();
								msgbox("File open error:\"CONF.DAT\"", 0);
								exit(1);
							}
						status=1;
						restorescr(0,0,24,79,p);
						break;
					case 1:
						pstr(but[itembut], rowbut, colbut, attbbut);
						savescr(0,0,24,79,p);
						if (waiting() != 0) /* if ticket printed out */
							wait_update();
						status=1;
						restorescr(0,0,24,79,p);
						break;
					case 2:
						free(p);
						pstr("        ", 3, 67, 0x5F);
						return;
				}
				break;
			}
			if (status == 1) /* if returned from confirm/wait screen */
				break;
		}
		/* clear the data cells */
		textattr(0x0A);
		for(r=14; r<=18; r+=2)
			for(c=32; c<=62; c+=15)	{
				gotoxy(c,r);
				cprintf("   ");
			}
	}
}


/* gets query on flights from user */
int query(void)
{
	char temp[FLINOLT]; /* to hold entered string for Flight No */
	char temp2[5]; /* to hold entered date */
	char timeip[TIMELT]; /* to hold entered time */
	char *p;
	int i, j, ind, diff, inc, fli_recno, totseats, ret, hour, min;

	pstr("ENQUIRY",3,67,0x5F);
	for (;;)	{
		/* clear blocks */
		ipblock(5, 22, FLINOLT, BATTB);
		ipblock(5, 56, SECTORLT, BATTB);
		ipblock(7, 22, DATELT, BATTB);
		ipblock(7, 56, TIMELT, BATTB);

		/* get Flight No. */
		if ((p = (char *)malloc(9*22*2)) == NULL)	{
			fprintf(stderr,"Not Enough Memory");
			exit(1);
		}
		savescr(1,0,21+1,0+FLINOLT+2,p);
		for (;;)	{
			ipblock(5, 22, FLINOLT, HATTB);
			statbar("Foreign Key: Enter Flight No. Select from the list");
			textattr(HATTB);
			showlist(1,0); /* shows Flights list */
			if (!getip(5, 22, temp, FLINOLT))	{
			/* if ESC pressed from within first field, exit current mode */
				ipblock(5, 22, FLINOLT, BATTB);
				restorescr(1,0,1+21,0+FLINOLT+2,p);
				free(p);
				pstr("       ", 3, 67, 0x5F);
				statbar("");
				return 0;
			}
			if (temp[0] == '\0')	{
				errbeep(); /* input shouldn't be NULL */
				continue;
			}
			for (fli_recno=0; fli_recno<findex; fli_recno++) /* check if code exists */
				if (strcmpi(temp, flight[fli_recno].flino) == 0)
					break;
			if (fli_recno == findex)	{
				msgbox("Flight No. not found", 0);
				continue; /* re-enter Flight No */
			}
			enq[eindex].flino = fli_recno; /* new value */
			ipblock(5, 22, FLINOLT, BATTB);
			pstr(flight[fli_recno].flino, 5, 22, BATTB);
			restorescr(1,0,1+21,0+FLINOLT+2,p);
			free(p);
			break; /* all went well */
		}

		/* display sector */
		pstr(flight[fli_recno].sector,5,56,BATTB);

		/* get Departure  Date */
		do	{
			ret = 1;
			ipblock(7, 22, DATELT, HATTB);
			statbar("Enter Date and press ENTER to continue");
			/* get day 1-31 */
			if (!(ret=getip(7, 22, temp2, 3)))
				break;
			/* convert char string to int */
			if (!(enq[eindex].depdate.day = atoi(temp2))) {
				temp2[0]='\0'; /* not a valid string */
				continue;
			}
			if (enq[eindex].depdate.day > 31 || enq[eindex].depdate.day < 1)	{
				errbeep();
				msgbox("Invalid Day", 0);
				temp2[0]='\0'; /* re-loop */
				continue;
			}
			/* print seperator */
			gotoxy(wherex()+1,8);
			cprintf("/");

			/* get month 1-12 */
			statbar("Enter Month and press ENTER to continue");
			if (!(ret=getip(7, 25, temp2, 3)))	{
				temp2[0]='\0';
				continue; /* if ESC hit while entering */
			}
			/* convert char string to int */
			if (!(enq[eindex].depdate.month = atoi(temp2))) {
				temp2[0]='\0'; /* not a valid string */
				continue;
			}
			if (enq[eindex].depdate.month > 12 || enq[eindex].depdate.month < 1)	{
				errbeep();
				msgbox("Invalid Month", 0);
				temp2[0]='\0'; /* re-loop */
				continue;
			}
			/* print seperator */
			gotoxy(wherex()+1,8);
			cprintf("/");

			/* get year */
			statbar("Enter Year and press ENTER to continue");
			if (!(ret=getip(7, 28, temp2, 5)))	{
				temp2[0]='\0';
				continue; /* if ESC hit while entering */
			}
			/* convert char string to int */
			if (!(enq[eindex].depdate.year = atoi(temp2))) {
				temp2[0]='\0'; /* not a valid string */
				continue;
			}
			if (enq[eindex].depdate.year > 2010 || enq[eindex].depdate.year < 2000)	{
				errbeep();
				msgbox("Invalid Year", 0);
				temp2[0]='\0'; /* re-loop */
				continue;
			}
			gotoxy(wherex()-1,8); /* pull cursor into the boundary */
		}while (temp2[0] == '\0');
		if (ret == 0) /* re-enter from beginning */
			continue;
		ipblock(7, 22, DATELT, BATTB);
		gotoxy(23,8);
		textattr(BATTB);
		cprintf("%d/%d/%d", enq[eindex].depdate.day, enq[eindex].depdate.month, \
		enq[eindex].depdate.year);
		gotoxy(1,1); /* get cursor from middle of screen */

		/* get departure time */
		do	{
			ret = 1;
			ipblock(7, 56, TIMELT, HATTB);
			textattr(HATTB);
			statbar("Enter Departure Time in 24 hour format like 22.30");
			if (!(ret=getip(7, 56, timeip, TIMELT)))
				break; /* time is taken as float */
			if (!(enq[eindex].deptime = atof(timeip))) /* convert char string to float */
				timeip[0]='\0'; /* not a valid string */
			/* seperate time into 2 parts */
			hour = enq[eindex].deptime * 100;
			min = hour % 100;
			hour /= 100;
			if (hour < 0 || min < 0 || hour > 23 || min > 59)	{
				errbeep();
				msgbox("Invalid Time", 0);
				timeip[0]='\0'; /* re-loop */
			}
			if (flight[fli_recno].dep != enq[eindex].deptime)	{
				errbeep();
				msgbox("Fight not available at this time", 0);
				timeip[0]='\0'; /* re-loop */
			}
		}while (timeip[0] == '\0');
		if (ret == 0) /* re-enter from beginning */
			continue;
		ipblock(7, 56, TIMELT, BATTB);
		pstr(timeip, 7, 56, BATTB);

		return 1;
	}
}

/* update: saves current enquiry record and updates it, saves conf data,
confirm data */
void update(void)
{
	/* append current record to file */
	fwrite(&enq[eindex], sizeof(struct res_manifesto), 1, fp);
	eindex++;

	/* save conf data */
	fclose(conf_fp); /* to prevent from multiple openings of single file */
	if ((conf_fp = fopen("conf.dat","w")) != NULL)
		fwrite(&conf, sizeof(struct con), MAXCONF_FLIGHTS, conf_fp);
	else
		msgbox("Error opening file: CONF.DAT", 0);

	/* save confirm data */
	fclose(confirm_fp);
	if ((confirm_fp = fopen("confirm.dat","w")) != NULL)	{
		confirm[cindex].status=1;
		fwrite(confirm, sizeof(struct reservation), MAXCONFIRMS, confirm_fp);
	}
	else
		msgbox("Error opening file: CONFIRM.DAT", 0);
}


/* update: saves current enquiry record and updates it, saves wait data */
void wait_update(void)
{
	/* append current record to file */
	fwrite(&enq[eindex], sizeof(struct res_manifesto), 1, fp);
	eindex++;

	/* save wait data */
	fclose(wait_fp);
	if ((wait_fp = fopen("wait.dat", "w")) != NULL)	{
		wait[windex].status=1;
		fwrite(wait, sizeof(struct waits), MAXWAITS, wait_fp);
	}
	else
		msgbox("Error opening file: WAIT.DAT", 0);
}


/* shows list of entered Aircrafts */
void showlist(int row, int col)
{
	int i,r,c;

	box(col,row,col+FLINOLT,row+20,0x30,0x00);
	/* fill list box */
	for (r=row+1,c=col+1; r<=row+19; r++)
		for (c=col+1; c<=col+FLINOLT-1; c++)
			pchar(r,c,' ',0x6A);
	for (i=0; i<findex; i++)
		pstr(flight[i].flino,++row,col+1,0x6A);
}


/* displays cell box */
void cells(void)
{
	int r, c, cellattb=0x0E, attb=0x0C;

	box(10,10,70,18,cellattb,-1);
	/* fill box */
	for (r = 11; r <= 17; r++)
		for(c=11; c < 70; c++)
			pchar(r,c,' ',0x00);
	textline(12,11,69,cellattb); /* horizontal line */
	pchar(12,10,'Ã',cellattb);
	pchar(12,70,'´',cellattb);
	textline(14,11,69,cellattb); /* horizontal line */
	pchar(14,10,'Ã',cellattb);
	pchar(14,70,'´',cellattb);
	textline(16,11,69,cellattb); /* horizontal line */
	pchar(16,10,'Ã',cellattb);
	pchar(16,70,'´',cellattb);
	textline(18,11,69,cellattb); /* horizontal line */
	vertline(25);
	vertline(40);
	vertline(55);
	pstr("Class",11,14,attb);
	pstr("Total Seat",11,28,attb);
	pstr("Smoking",11,44,attb);
	pstr("No Smoking",11,58,attb);
	pstr("   First    ",13,11,attb);
	pstr("   Business ",15,11,attb);
	pstr("   Economy  ",17,11,attb);
}


/* prints verticle line at column c */
void vertline(int c)
{
	int r, attb=0x0E;

	for(r=11; r<18; r++)
		pchar(r,c,'³',attb);
	pchar(10,c,'Â',attb);
	pchar(12,c,'Å',attb);
	pchar(14,c,'Å',attb);
	pchar(16,c,'Å',attb);
	pchar(18,c,'Á',attb);
}


/* Opens ENQUIRY.DAT and CONF.DAT files for global access */
void enquirydata(void)
{
	/* read enquiry data */
	if (access("enquiry.dat", 0))
		msgbox("File does not exist:\"ENQUIRY.DAT\". New file created", 0);
	if ((fp = fopen("enquiry.dat", "a+")) != NULL) {
		eindex = fread(enq, sizeof(struct res_manifesto), MAXENQ, fp);
		/* eindex: next free structure position */
	}
	else	{
		errbeep();
		msgbox("File creation error:\"ENQUIRY.DAT\"", 0);
		exit(1);
	}

	/* read conf data */
	if ((conf_fp = fopen("conf.dat", "r")) != NULL) {
		fread(conf, sizeof(struct con), MAXCONF_FLIGHTS, conf_fp);
	}
	else if ( (conf_fp = fopen("conf.dat", "w")) != NULL )
		msgbox("File does not exist:\"CONF.DAT\". New file created", 0);
	else	{
		errbeep();
		msgbox("File creation error:\"CONF.DAT\"", 0);
		exit(1);
	}
}

