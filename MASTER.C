/* Aircraft Master File */
#include <stdio.h>
#include <conio.h>
#include <string.h> /* for strcmpi */
#include"defs.h" /* local file containing define statements & function declarations */

#define ATTB	0x1B /* Screen Attribute */
#define BATTB	0x20 /* Block Attribute */
#define HATTB	0x4F /* Highlight Attribute */
#define ROW		9	 /* row, col position of CODE field */
#define COL     40

static void add(void);
static void save(void);
static void view(void);
static void dele(void);
static void moveup(int);
static void modify(void);
static void clrblocks(void);
static void disprecord(int);
void md_moveup(int i);
void f_moveup(int i);

extern int mdindex;
extern struct mastdetail mdetail[];
extern struct airflight flight[]; /* defined in flight.c */
extern int findex;

struct airmaster mast[MAXAIRCRAFTS];
int index; /* next free position in mast */
int deleted; /* deletion status of master record */
static FILE *fp;


void masterentry(void)
{
	int i, j, itembut, attbbut, hattbbut, rowbut, colbut, skip;
	char *but[]={	"F2    Add",
					"F3 Delete",
					"F4 Modify",
					"F5   View",
					"F6   Save",
					"F10  Exit"	};
	char *help[]={	"Add new record",
					"Delete a record",
					"Modify a record",
					"Displays a record. Use UP/DOWN keys to scroll",
					"Save all records to MASTER.DAT",
					"Press Enter or Esc to Exit" };

	/* Screen design */
	scrfil(' ', ATTB);
	box(0,0,79,24, 0x1E,-1);
	for(i=1; i<=78; i++)
		pchar(6,i,'Í',0x1E);
	pstr(" Aircraft Master Entry ",3,28,0x71);
	pstr("Date:" , 4,4,0x1F);
	pstr("Mode:",4,60,0x1F);
	pstr("Aircraft Code :",9,20,ATTB);
	pstr("Aircraft Name :",12,20,ATTB);
	pstr("   Total Seats:",15,20,ATTB);
	dispdate(4,10,0x1F);
	clrblocks();
	gotoxy(1,1); /* move cursor from middle of screen */

	attbbut = 0x5E;
	hattbbut = 0x0C;
	rowbut = 20;
	colbut = 5;
	skip = 12;
	for (i=0, j=colbut; i<=5; i++, j+=skip)
		pstr(but[i], rowbut, j, attbbut);
	itembut = 0;
	pstr(but[itembut], rowbut, colbut, hattbbut); /* highlight first button */
	statbar(help[itembut]);
	for (;;)	{
		switch (scan())	{
		case RIGHT:
		case TAB:
			pstr(but[itembut], rowbut, colbut, attbbut);
			if (itembut == 5)	/* if at last button */
				itembut=0, colbut=5;
			else
				itembut++, colbut += skip;
			pstr(but[itembut], rowbut, colbut, hattbbut);
			statbar(help[itembut]);
			break;
		case LEFT:
		case SHTAB:
			pstr(but[itembut], rowbut, colbut, attbbut);
			if (itembut == 0)	/* if at first button */
				itembut=5, colbut=65;
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
		case F3:
			pstr(but[itembut], rowbut, colbut, attbbut);
			dele();
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
					dele();
					pstr(but[itembut], rowbut, colbut, hattbbut);
					break;
				case 2:
					pstr(but[itembut], rowbut, colbut, attbbut);
					modify();
					pstr(but[itembut], rowbut, colbut, hattbbut);
					break;
				case 3:
					pstr(but[itembut], rowbut, colbut, attbbut);
					view();
					pstr(but[itembut], rowbut, colbut, hattbbut);
					break;
				case 4:
					pstr(but[itembut], rowbut, colbut, attbbut);
					save();
					pstr(but[itembut], rowbut, colbut, hattbbut);
					break;
				case 5:
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
	char seatstr[MAXSEATDIG], temp[40], msg[80];
	int i, ret;

	pstr("ADD   ",4,66,0x1F);
	for (;;)	{
		if (index == MAXAIRCRAFTS)	{
			errbeep();
			msgbox("Enough Flights. Addition not possible", 0);
			return ;
		}
		clrblocks();

		/* get code */
		for (;;)	{
			ipblock(ROW, COL, MAXCODE, HATTB);
			statbar("Enter Aircraft CODE. It can be NUMBER or ALPHABET");
			textattr(HATTB);
			if (! getip (ROW, COL, mast[index].code, MAXCODE))	{
			/* if ESC pressed from within first field, exit current mode */
				ipblock(ROW, COL, MAXCODE, BATTB);
				pstr("      ", 4, 66, 0x1F);  // clear MODE
				statbar(""); /* clear status bar */
				gotoxy(1,1); /* take off cursor also */
				return;
			}
			if (mast[index].code[0] == '\0')	{
				errbeep();
				msgbox("Primary Key shouldn't be NULL", 0);
				continue;
			}
			/* check for duplicates */
			for (i=0; i<index; i++)
				if (strcmpi(mast[index].code, mast[i].code) == 0)	{
					errbeep();
					msgbox("CODE Already Exists", 0);
					break;
				}
			if (i != index)
				continue; /* re-enter Code */
			ipblock(ROW, COL, MAXCODE, BATTB);
			pstr(mast[index].code, ROW, COL, BATTB);
			break; /* all is well */
		}

		/* get name */
		do	{
			ret = 1;
			ipblock(ROW+3, COL, MAXNAME, HATTB);
			statbar("Enter Aircraft Name");
			if (!(ret=getip(ROW+3, COL, mast[index].name, MAXNAME)))
				break;
		}while (mast[index].name[0] == '\0');
		if (ret == 0) /* re-enter from beginning */
			continue;
		ipblock(ROW+3, COL, MAXNAME, BATTB);
		pstr(mast[index].name, ROW+3, COL, BATTB);

		/* get seat */
		do	{
			ret = 1;
			ipblock(ROW+6, COL, MAXSEATDIG, HATTB);
			sprintf(msg,"Enter seat capacity of Aircraft. It should be less than %d"\
			, MAXACSEATS);
			statbar(msg);
			if (!(ret=getip(ROW+6, COL, seatstr, MAXSEATDIG)))
				break;
			/* Here the no. of seats is taken as a char string, because of
			some benefits */
			if (!(mast[index].seats = atoi(seatstr))) /* convert char string to int */
				seatstr[0]='\0'; /* not a valid string */
			if (mast[index].seats < 1 || mast[index].seats > MAXACSEATS)	{
				sprintf(msg, "Total seats should be 0 - %d", MAXACSEATS);
				msgbox(msg, 0);
				seatstr[0] = '\0';
				continue;
			}
		}while (seatstr[0] == '\0');
		if (ret == 0) /* re-enter from beginning */
			continue;
		ipblock(ROW+6, COL, MAXSEATDIG, HATTB);
		pstr(seatstr, ROW+6, COL, BATTB);

		index++; /* update record no. */
	}
}


/* saves MASTER.DAT */
void save(void)
{
	FILE *tfp;

	pstr("SAVE  ",4,66,0x1F);
	if ((fp = fopen("master.dat","w")) != NULL)	{
		fwrite(&mast, sizeof(struct airmaster), index, fp);
		statbar("MASTER.DAT Saved");
		while (!kbhit());
	}
	else
		msgbox("Error saving to MASTER.DAT", 0);

	if (deleted)	{ /* if master record deleted, save mdetail & flight records also */
		/* save mdetail */
		if ((tfp = fopen("mdetail.dat","w")) != NULL)	{
			fwrite(&mdetail, sizeof(struct mastdetail), mdindex, tfp);
			fclose(tfp);
		}
		else
			msgbox("Error saving to MDETAIL.DAT", 0);
		/* save flight */
		if ((tfp = fopen("flight.dat","w")) != NULL)	{
			fwrite(&flight, sizeof(struct airflight), findex, tfp);
			fclose(tfp);
		}
		else
			msgbox("Error saving to: FLIGHT.DAT", 0);
		deleted = 0;
	}
	pstr("      ",4,66,0x11);
	fclose(fp);
	return;
}


/* allows user to view records usint UP/DOWN keys or directly entering
the record no. */
void view(void)
{
	int ch, i, ind;
	char code[MAXCODE]; /* to hold the entered string for CODE */

	pstr("VIEW  ",4,66,0x1F);
	if (index == NULL)	{
		msgbox("No Records", 0);
		return ;
	}
	ind = 0;
	disprecord(ind);

	for (;;)	{
		gotoxy(COL+1, ROW+1); /* cursor at code field */
		switch(ch=scan())	{
		case UP:
		case DOWN:
			if (ch == DOWN && ind < (index-1))
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
			if (index != NULL)	{
				ind = index-1;
				disprecord(ind);
			}
			break;

		case ESC:
			clrblocks();
			pstr("      ",4,66,0x11);
			gotoxy(1,1);
			return;

		default:
			if (isalnum(ch))	{
				ipblock(ROW, COL, MAXCODE, HATTB);
				ungetch(ch); /* the char read at switch */
				statbar("Enter the CODE to view");
				textattr(HATTB);
				if (!getip(ROW, COL, code, MAXCODE))
					return;
				for (i=0; i < index; i++)	{ /* check if record exists */
					 if (strcmpi(code, mast[i].code) == 0)	{
						  ind = i;
						  disprecord(ind);
						  break;
					 }
					 if (code[0] == '\0')
						return;
				}
				if (i==index)	{
					errbeep();
					msgbox("Record Not Found", 0);
				}
			}
			break;
		}
	}
}


/* displays the specified record */
void disprecord(int i)
{
	clrblocks();
	pstr(mast[i].code, ROW, COL, BATTB);
	pstr(mast[i].name, ROW+3, COL, BATTB);
	textattr(BATTB);
	gotoxy(COL+1,ROW+7);
	cprintf("%d",mast[i].seats);
}


/* deletes a record */
void dele(void)
{
	int i, j, k, ch;
	char code[MAXCODE];

	pstr("DELETE",4,66,0x1F);
	clrblocks();

	gotoxy(COL+1, ROW+1);
	statbar("Enter The Record Number To DELETE");
	for (i=0; i < MAXCODE; i++)
		code[i] = '\0';
	textattr(HATTB);
	ipblock(ROW, COL, MAXCODE, HATTB);
	if (! getip(ROW, COL, code, MAXCODE))	{
		ipblock(ROW, COL, MAXCODE, BATTB);
		return;
	}
	i=0;
	while ( i < index)	{
		if (strcmpi(code, mast[i].code) == 0)	{
			ch=msgbox("All the corresponding records will be deleted. Are you sure?",1);
			if (ch == 'N')	{
				ipblock(ROW, COL, MAXCODE, BATTB);
				gotoxy(1,1);
				return;
			}
			moveup(i);
			deleted = 1; /* save mdetail & flight records */
			statbar("Record Deleted");
			/* delete corresponding mdetail entry */
			for (j=0; j < mdindex; j++)
				if (mdetail[j].aircode == i)
					md_moveup(j);
			/* assign changed master codes to mdetail */
			for (k=i+1; k < index; k++)
				for (j=0; j < mdindex; j++)
					if (mdetail[j].aircode == k)
						mdetail[j].aircode = k-1;

			/* delete corresponding flight entry */
			for (j=0; j < findex; j++)
				if (flight[j].aircode == i)
					f_moveup(j);
			/* assign changed master codes to flight */
			for (k=i+1; k < index; k++)
				for (j=0; j < findex; j++)
					if (flight[j].aircode == k)
						flight[j].aircode = k-1;
			i--;
			break;
		}
		if (code[0] == '\0')
			return;
		i++;
	}
	if (i == index)	{
		errbeep();
		msgbox("Record Not Found", 0);
	}
	clrblocks();
	gotoxy(1,1);
	pstr("      ",4,66,0x11);
}


/* moves master records one position UP from the specified record */
void moveup(int i)
{
	for (; i < index-1; i++)	{
		strcpy(mast[i].code ,mast[i+1].code);
		strcpy(mast[i].name ,mast[i+1].name);
		mast[i].seats = mast[i+1].seats;
	}
	index--;
}


/* moves mdetail records one position UP from the specified record */
void md_moveup(int i)
{
	int j,k;

	for (; i < mdindex-1; i++)	{
		mdetail[i].aircode = mdetail[i+1].aircode;
		for (j=0; j<3; j++)
			for (k=0; k<4; k++)
				mdetail[i].seats[j][k] = mdetail[i+1].seats[j][k];
	}
	mdindex--;
}


/* moves flight records one position UP from the specified record */
void f_moveup(int i)
{
	int j,k;

	for (; i < findex-1; i++)	{
		strcpy(flight[i].flino ,flight[i+1].flino);
		strcpy(flight[i].sector ,flight[i+1].sector);
		strcpy(flight[i].op_days ,flight[i+1].op_days);
		flight[i].dep = flight[i+1].dep;
		flight[i].arriv = flight[i+1].arriv;
		flight[i].f_fare = flight[i+1].f_fare;
		flight[i].b_fare = flight[i+1].b_fare;
		flight[i].e_fare = flight[i+1].e_fare;
		flight[i].aircode = flight[i+1].aircode;
	}
	findex--;
}


/* record modification */
void modify(void)
{
	int i, ret, ind, flag, sig;
	char code[MAXCODE], seat[MAXSEATDIG], temp[25], msg[80];

	pstr("MODIFY",4,66,0x1F);
	if (index == NULL)	{
		errbeep();
		msgbox("No Records", 0);
		return;
	}
	for (;;)	{
		clrblocks();
		ipblock(ROW, COL, MAXCODE, HATTB);
		statbar("Enter The Record Number To Modify");
		textattr(HATTB);
		if (! getip(ROW, COL, code, MAXCODE))	{
			ipblock(ROW, COL, MAXCODE, BATTB);
			break;
		}
		/* check if code exists */
		for (ind=0; ind < index; ind++)
			if (strcmpi(code, mast[ind].code) == 0)
				 break;
		if (ind==index) {
			errbeep();
			msgbox("Record Not Found", 0);
			continue;
		}
		/* record found at ind */
		disprecord(ind);

		/* Modify CODE */
		do {
			ipblock(ROW, COL, MAXCODE, HATTB);
			pstr(mast[ind].code, ROW, COL, HATTB);
			statbar("Modify CODE field. Press ESC to exit");
			strcpy(temp, mast[ind].code); /* save original */
			ret = modi(mast[ind].code, MAXCODE, ROW, COL, HATTB);
			if (ret == -1)	{ /* if ESC hit at CODE field before modication,
			come out of MODIFY mode */
				clrblocks();
				pstr("       ",4,66,0x11);
				gotoxy(1,1);
				break;
			}
			if (ret == 0)	{  /* if ESC hit after any modification restore
			original field value */
				strcpy(mast[ind].code, temp); /* restore original */
				continue;
			}
			/* Primary key shouldn't be NULL */
			if (mast[ind].code[0] == '\0')	{
				errbeep();
				msgbox("Primary Key Value May Not Be NULL", 0);
				continue;
			}

			/* check for duplicates */
			for (i=0; i<index; i++)	{
				if (i == ind)
					continue; /* skip current index */
				if (strcmpi(mast[ind].code, mast[i].code) == 0)	{
					errbeep();
					msgbox("CODE Already Exist", 0);
					break;
				}
			}
		}while (i != index); /* until the modified CODE isn't a duplicate copy */
		if (ret == -1)	/* continuation control from above loop */
			break;
		ipblock(ROW, COL, MAXCODE, BATTB);
		pstr(mast[ind].code, ROW, COL, BATTB);

		/* Modify NAME */
		do 	{
			ipblock(ROW+3, COL, MAXNAME, HATTB);
			pstr(mast[ind].name, ROW+3, COL, HATTB);
			statbar("Modify NAME");
			strcpy(temp, mast[ind].name);
			ret=modi(mast[ind].name, MAXNAME, ROW+3, COL, HATTB);
			if (ret == -1) /* if ESC hit before modification, Re-enter record */
				break;
			if (ret == 0)/* if ESC hit in between modification, restore original value */
				strcpy(mast[ind].name, temp); /* restore original */
		}while (ret != 1 || mast[ind].name[0] == '\0');
		if (ret == -1)
			continue; /* re-enter record (continuation of control from above loop) */
		ipblock(ROW+3, COL, MAXNAME, BATTB);
		pstr(mast[ind].name, ROW+3, COL, BATTB);

		/* modify total seats */
		sig = MAXSEATDIG-1;
		do 	{
			ipblock(ROW+6, COL, MAXSEATDIG, HATTB);
			sprintf(msg,"Enter seat capacity of Aircraft. It should be less than %d", MAXACSEATS);
			statbar(msg);
			gcvt((double)mast[ind].seats, sig, seat);
			pstr(seat, ROW+6, COL, HATTB);
			strcpy(temp, seat);
			ret = modi(seat, MAXSEATDIG, ROW+6, COL, HATTB);
			if (ret == -1)	/* if ESC hit before modification, Re-enter record */
				break;
			if (ret == 0) /* if ESC hit in between modification,
			restart modification with original value */
				continue;
			if (! (mast[ind].seats = atoi(seat)))	{
				seat[0] = '\0';
				continue;
			}
			if (mast[ind].seats < 1 || mast[ind].seats > MAXACSEATS)	{
				sprintf(msg, "Total seats should be less than %d", MAXACSEATS);
				msgbox(msg, 0);
				seat[0] = '\0';
				continue;
			}
		}while (ret != 1 || seat[0] == '\0');
		if (ret == -1)
			continue; /* re-enter record (continuation of control from above loop) */
		ipblock(ROW+6, COL, MAXSEATDIG, BATTB);
		pstr(seat, ROW+6, COL, BATTB);
	}
	pstr("      ",4,66,0x11);
}


/* clear all fields */
void clrblocks(void)
{
	ipblock(ROW, COL, MAXCODE, BATTB);
	ipblock(ROW+3, COL, MAXNAME, BATTB);
	ipblock(ROW+6, COL, MAXSEATDIG, BATTB);
}


/* Opens MASTER.DAT file for global access */
void mastdata(void)
{
	/* read master data */
	if ((fp = fopen("master.dat", "r")) != NULL) {
		index = fread(mast, sizeof(struct airmaster), MAXAIRCRAFTS, fp);
		/* index: next free structure position */
	}
	else if ( (fp = fopen("master.dat", "w")) != NULL )
		msgbox("File does not exist:\"MASTER.DAT\". New file created", 0);
	else	{
		errbeep();
		msgbox("File creation error:\"MASTER.DAT\"", 0);
		exit(1);
	}
}
