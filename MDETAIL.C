/* Aircraft Detail Entry */
#include <stdio.h>
#include <conio.h>
#include <string.h> /* for strcmpi */
#include <stdlib.h> /* for atof */
#include"defs.h" /* local file containing define statements & function declarations */

#define SEATDIG	4
#define	SMOKEDIG	3
#define	ROW	11		/* row, col position of First cell */
#define	COL	25
#define	BATTB	0X2F	/* Block attribute */
#define	HATTB	0X5A    /* Highlight attribute */
#define	ATTB	0X6E    /* Normal attribute */

/* The constants MAXAIRCRAFTS, MAXNAME, MAXCODE used here are defined in
defs.h for Master Entry */

extern int index;
extern struct airmaster mast[];

static void add(void);
static void dele(void);
static void modify(void);
static void view(void);
static void save(void);
static void clrblocks(void);
static void disprecord(int);
static void cells(void);
static void vertline(int c);
static void showlist(int row, int col);

static FILE *fp;
struct mastdetail mdetail[MAXAIRCRAFTS];
int mdindex; /* next free position in flight structure */

void airdetailentry(void)
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
					"Save all records to MDETAIL.DAT",
					"Press Enter or Esc to Exit"	};

	/* Screen design */
	scrfil(' ', ATTB);
	box(0,0,79,24,0x6B,-1);
	pstr(" Aircraft Detail Entry ", 2, 28, 0x34);
	pstr("Date:", 3, 4, 0x67);
	pstr("Mode:", 3, 60, 0x67);
	textline(4,1,78,0x6B);
	pchar(4,0,'Ã',0x6B);
	pchar(4,79,'´',0x6B);
	pstr("Aircraft Code", 6, 7, ATTB);
	pstr("Aircraft Name", 6, 43, ATTB);
	pstr("Total Seats", 17, 52, ATTB);
	cells(); /* Cells creation */
	clrblocks(); /* displays all input blocks */
	textline(19,1,78,0x6B);
	pchar(19,0,'Ã',0x6B);
	pchar(19,79,'´',0x6B);
	dispdate(3,10,0x6F);
	gotoxy(1,1); /* just to move it from middle of screen */

	attbbut = 0x3E;
	hattbbut = 0x1F;
	rowbut = 21;
	colbut = 5;
	skip = 15;
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
				itembut=4, colbut=65;
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


/* add record */
void add(void)
{
	char temp[MAXCODE], temp2[SEATDIG]; /* to hold entered string for Aircraft
	Code and Seats respectively */
	char *p;
	int i, j, ind, diff, inc, mast_recno, totseats, cellattb=0x6F;

	pstr("ADD    ",3,67,0x6F);
	for (;;)	{
		 reentry: clrblocks();

		 /* get Aircraft Code */
		 if ((p = (char *)malloc(8 * 22 * 2)) == NULL)	{
			fprintf(stderr,"Not Enough Memory");
			exit(1);
		 }
		 savescr(1,68,21+1,68+MAXCODE+2,p);
		 for (;;)	{
			ipblock(6, 21, MAXCODE, HATTB);
			statbar("Primary Key: Enter Aircraft Code. Select from the list");
			textattr(HATTB);
			showlist(1,68); /* shows aircraft list */
			if (!getip(6, 21, temp, MAXCODE))	{
			/* if ESC pressed from within first field, exit current mode */
				ipblock(6, 21, MAXCODE, BATTB);
				restorescr(1,68,1+21,68+MAXCODE+2,p);
				free(p);
				pstr("       ", 3, 67, 0x6F);
				statbar("");
				gotoxy(1,1); /* take off cursor also */
				return;
			}
			if (temp[0] == '\0')	{
				errbeep();
				msgbox("Primary Key shouldn't be NULL", 0);
				continue;
			}
			/* check if code exists */
			for (mast_recno=0; mast_recno<index; mast_recno++)
				if (strcmpi(temp, mast[mast_recno].code) == 0)
					break;
			if (mast_recno == index)	{
				msgbox("Aircraft Code not found in Master", 0);
				continue; /* re-enter Aircraft Code */
			}
			for (i=0; i<mdindex; i++) /* check for duplicates */
				if (mdetail[i].aircode == mast_recno)	{
					errbeep();
					msgbox("Details for this Aircraft already entered", 0);
					break;
				}
			if (i != mdindex)
				continue; /* re-enter Aircraft Code */
			mdetail[mdindex].aircode = mast_recno; /* new value */
			ipblock(6, 21, MAXCODE, BATTB);
			pstr(mast[mast_recno].code, 6, 21, BATTB);
			restorescr(1,68,1+21,68+MAXCODE+2,p);
			free(p);
			break; /* all went well */
		}
		pstr(mast[mast_recno].name,6,57,BATTB);
		textattr(BATTB);
		gotoxy(65,18);
		cprintf("%d",mast[mast_recno].seats);

		/* get data */
		statbar("Enter Seats");
		do {
			totseats=0;
			for (i=0,inc=0; i<3; i++,inc+=2)
				for (j=0,diff=0; j<4; j++,diff+=13)     {
					do {
						ipblock(ROW+inc, COL+diff, SEATDIG, HATTB);
						textattr(HATTB);
						if (!(getip(ROW+inc, COL+diff, temp2, SEATDIG)))
							goto reentry; /* re-input */
						if (!(mdetail[mdindex].seats[i][j] = atoi(temp2))) /* convert char string to int */
							temp2[0]='\0'; /* not a valid string */
						if (mdetail[mdindex].seats[i][j] < 0)
							temp2[0]='\0'; /* not a valid string */
					}while (temp2[0] == '\0');
					if (j == 3 && mdetail[mdindex].seats[i][0] * mdetail[mdindex].\
					seats[i][1] != mdetail[mdindex].seats[i][2] + mdetail[mdindex].\
					seats[i][3]) {
						errbeep();
						msgbox("Usage:Seat in Row X Tot Row = Smoke Seat + No Smoke Seat"\
						,0);
						/* clear the row */
						for (j=diff=0; j<4; j++,diff+=13)
							ipblock(ROW+inc, COL+diff, SEATDIG, cellattb);
						j = -1; /* it will increment in next iteration */
						diff = -13; /* it will increment in next iteration */
						continue; /* row re-entry */
					}
					if (j == 3)
						totseats += mdetail[mdindex].seats[i][0] * \
						mdetail[mdindex].seats[i][1];
					ipblock(ROW+inc, COL+diff, SEATDIG, cellattb);
					pstr(temp2, ROW+inc, COL+diff, cellattb);
				}
			if (totseats != mast[mast_recno].seats)	{
				errbeep();
				msgbox("Total Seats does not matches", 0);
				clrblocks();
				pstr(mast[mdetail[mdindex].aircode].code, 6, 21, cellattb);
				pstr(mast[mdetail[mdindex].aircode].name, 6, 57, cellattb);
				textattr(cellattb);
				gotoxy(65, 18);
				cprintf("%d", mast[mdetail[mdindex].aircode].seats);
			}
		}while (totseats != mast[mast_recno].seats);

		mdindex++;
	}
}


/* modify record */
void modify(void)
{
	int i, j, inc, ind, ret, mast_recno, sig, diff, totseats, cellattb=0x6f;
	char *p, temp[MAXCODE], temp2[SEATDIG];

	pstr("MODIFY ",3,67,0x6F);
	if (mdindex == NULL)	{
		msgbox("No Records", 0);
		return;
	}
	for (;;)	{
		remodify: clrblocks();
		ipblock(6, 21, MAXCODE, HATTB);
		statbar("Enter Aircraft Code to Modify");
		textattr(HATTB);
		if (!getip(6,21,temp,MAXCODE))	{
			ipblock(6, 21, MAXCODE, BATTB);
			pstr("       ",3,67,0x6F);
			gotoxy(1,1);
			break;
		}
		/* check if code exists */
		/* find Master Record No. for temp in struct mast */
		for (mast_recno=0; mast_recno < index; mast_recno++)
			if (strcmpi(temp, mast[mast_recno].code) == 0)
				break;
		/* check if Record No. exists in struct mdetail */
		for (i=0; i < mdindex; i++)
			if (mdetail[i].aircode == mast_recno)
				  break;
		if (i==mdindex)	{
			msgbox("Record Not Found", 0);
			continue;
		}

		/* record found at i */
		ind = i;
		disprecord(ind);

		/* modify Aircraft Code */
		if ((p = (char *)malloc(8*22*2)) == NULL)	{
			fprintf(stderr,"Not Enough Memory");
			exit(1);
		}
		savescr(1,5,1+21,5+MAXCODE+2,p);
		do	{
			ipblock(6,21,MAXCODE,HATTB);
			pstr(temp, 6, 21, HATTB);
			statbar("Select Aircraft Code from list. Press ESC to exit");
			showlist(1,5); /* shows aircraft list */
			ret = modi(temp, MAXCODE, 6, 21, HATTB);
			if (ret == -1) { /* if ESC hit before modification,
			come out of MODIFY mode */
				ipblock(6, 21, MAXCODE, BATTB);
				pstr("       ",3,67,0x6F);
				gotoxy(1,1);
				break;
			}
			if (ret == 0) { /* if ESC hit after modification, restore original
			field value */
				strcpy(temp, mast[mdetail[ind].aircode].code);
				continue;
			}
			/* primary key shouldn't be NULL */
			if (temp[0] == '\0')	{
				msgbox("Primary Key value shouldn't be NULL", 0);
				continue;
			}
			/* check if Aircraft Code exists */
			for (mast_recno=0; mast_recno < index; mast_recno++)
				if (strcmpi(temp, mast[mast_recno].code) == 0)
					break;
			if (mast_recno == index)	{
				msgbox("Aircraft Code not found in Master", 0);
				continue;
			}
		}while (mast_recno == index);
		restorescr(1,5,1+21,5+MAXCODE+2,p);
		free(p);
		if (ret == -1)
			break;	/* continuation of control from above loop */
		mdetail[ind].aircode = mast_recno; /* new value */
		ipblock(6, 21, MAXCODE, BATTB);
		pstr(mast[mast_recno].code, 6, 21, BATTB);

		/* modify data */
		statbar("Modify Seats");
		sig=3;
		do {
			totseats=0;
			for (i=0,inc=0; i<3; i++,inc+=2)
				for (j=0,diff=0; j<4; j++,diff+=13)     {
					do	{
						ipblock(ROW+inc, COL+diff, SEATDIG, HATTB);
						gcvt(mdetail[ind].seats[i][j],sig, temp2);
						pstr(temp2, ROW+inc, COL+diff, HATTB);
						ret = modi(temp2, SEATDIG, ROW+inc, COL+diff, HATTB);
						if (ret == 0) /* if ESC hit in between modification, restart
						modification with original value */
							continue;
						if ( !(mdetail[ind].seats[i][j] = atoi(temp2)))	{
							temp2[0] = '\0'; /* not a valid input */
							continue;
						}
						if (mdetail[ind].seats[i][j] < 0)
							temp2[0] = '\0'; /* not valid input */
					}while (temp2[0] == '\0' || ret != 1);
					if (j==3 && mdetail[ind].seats[i][0] * mdetail[ind].seats[i][1] \
					!= mdetail[ind].seats[i][2] + mdetail[ind].seats[i][3]) {
						errbeep();
						msgbox("Usage:Seat in Row X Tot Row = Smoke Seat + No Smoke Seat"\
						,0);
						/* clear the row */
						ipblock(ROW+inc, COL+diff, SEATDIG, cellattb);
						pstr(temp2, ROW+inc, COL+diff, cellattb);
						j = -1; /* it will increment in next iteration */
						diff = -13; /* it will increment in next iteration */
						continue; /* row re-modify */
					}
					if (j == 3)
						totseats += mdetail[ind].seats[i][0] * mdetail[ind].seats[i][1];
					ipblock(ROW+inc, COL+diff, SEATDIG, cellattb);
					pstr(temp2, ROW+inc, COL+diff, cellattb);
				}
			if (totseats != mast[mast_recno].seats)	{
				errbeep();
				msgbox("Total Seats does not matches", 0);
				pstr(mast[mdetail[ind].aircode].code, 6, 21, cellattb);
				pstr(mast[mdetail[ind].aircode].name, 6, 57, cellattb);
				textattr(cellattb);
				gotoxy(65, 18);
				cprintf("%d", mast[mdetail[ind].aircode].seats);
			}
		}while (totseats != mast[mast_recno].seats);
	}
}


/* allows user to view records */
void view(void)
{
	int mast_recno, i, ind, ch;
	char temp[MAXCODE]; /* to hold the entered string for Aircraft Code */

	pstr("VIEW   ",3,67,0x6F);
	if (mdindex == NULL)	{
		msgbox("No Records", 0);
		return ;
	}
	ind=0;
	disprecord(ind);

	for (;;)	{
		gotoxy(21+1, 6+1);
		switch(ch=scan())	{
		case UP:
		case DOWN:
			if (ch == DOWN && ind < (mdindex-1))
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
			if (mdindex != NULL)	{
				ind = mdindex-1;
				disprecord(ind);
			}
			break;

		case ESC:
			ipblock(6, 21, MAXCODE, BATTB);
			pstr("       ",3,67,0x6F);
			gotoxy(1,1);
			return;
		default:
			if (isalnum(ch))	{
				ipblock(6, 21, MAXCODE, HATTB);
				ungetch(ch); /* the char read at switch */
				statbar("Enter the AIRCRAFT CODE to view");
				textattr(HATTB);
				if (!getip(6, 21, temp, MAXCODE))	{
					ipblock(6, 21, MAXCODE, BATTB);
					pstr("       ",3,67,0x6F);
					gotoxy(1,1);
					return;
				}
				if (temp[0] == '\0') return;
				/* find Master Record No. for temp in struct mast */
				for (mast_recno=0; mast_recno < index; mast_recno++)
					if (strcmpi(temp, mast[mast_recno].code) == 0)
						break;
				/* check if Record No. exists in struct mdetail */
				for (i=0; i < mdindex; i++)
					if (mdetail[i].aircode == mast_recno)	{
						  ind = i;
						  disprecord(ind);
						  break;
					 }
				if (i==mdindex)
					msgbox("Record Not Found", 0);
			}
			break;
		}
	}
}


/* saves MDETAIL.DAT */
void save(void)
{
	char ch;

	pstr("SAVE  ",3,67,0x6F);
	if ((fp = fopen("mdetail.dat","w")) != NULL)	{
		fwrite(&mdetail, sizeof(struct mastdetail), mdindex, fp);
		statbar("MDETAIL.DAT Saved");
		while (!kbhit());
	}
	else
		msgbox("Error opening file: MDETAIL.DAT", 0);
	pstr("      ",3,67,0x6F);
	fclose(fp);
	return;
}


/* displays specified record */
void disprecord(int ind)
{
	int i, j, diff, inc;

	clrblocks();
	pstr(mast[mdetail[ind].aircode].code, 6, 21, BATTB);
	pstr(mast[mdetail[ind].aircode].name, 6, 57, BATTB);
	textattr(BATTB);
	gotoxy(65, 18);
	cprintf("%d", mast[mdetail[ind].aircode].seats);

	/* disp data */
	textattr(0x6F);
	for (i=0,inc=0; i<3; i++,inc+=2)
		for (j=0,diff=0; j<4; j++,diff+=13)     {
			gotoxy(COL+diff+1, ROW+inc+1);
			cprintf("%d", mdetail[ind].seats[i][j]);
		}
}


/* clear all fileds */
void clrblocks(void)
{
	int i, j, diff, inc, cellattb=0x6F;
	ipblock(6,21,MAXCODE,BATTB);
	ipblock(6,57,MAXNAME,BATTB);
	ipblock(17,64,SEATDIG,BATTB);
	/* clear cells */
	for (i=0,inc=0; i<3; i++,inc+=2)
		for (j=0,diff=0; j<4; j++,diff+=13)
			ipblock(ROW+inc, COL+diff, SEATDIG, cellattb);
}


/* Creates a 4 X 3 cells table */
void cells(void)
{
	const int row=8, col=7;
	int r, c, diff, attb, cellattb;
	r=row; c=col; diff=13;
	cellattb=0x6B;
	attb=0x60;
	/* horiz lines */
	textline(r,c,diff*5+col-1,cellattb);
	textline(r+=2,c,diff*5+col-1,cellattb);
	textline(r+=2,c,diff*5+col-1,cellattb);
	textline(r+=2,c,diff*5+col-1,cellattb);
	textline(r+=2,c,diff*5+col-1,cellattb);

	/* vert lines */
	r=row;
	vertline(c);
	vertline(c+=diff);
	vertline(c+=diff);
	vertline(c+=diff);
	vertline(c+=diff);
	vertline(c+=diff);
	/* left first column */
	c=col;
	pchar(r,c,'Ú',cellattb);
	pchar(r+2,c,'Ã',cellattb);
	pchar(r+4,c,'Ã',cellattb);
	pchar(r+6,c,'Ã',cellattb);
	pchar(r+8,c,'À',cellattb);
	/* rightmost column */
	c=col+diff*5;
	pchar(r,c,'¿',cellattb);
	pchar(r+2,c,'´',cellattb);
	pchar(r+4,c,'´',cellattb);
	pchar(r+6,c,'´',cellattb);
	pchar(r+8,c,'Ù',cellattb);
	/* headings */
	r=row+1; c=col;
	pstr("   Class    ",r,++c,attb);
	pstr("Seats In Row",r,c+=diff,attb);
	pstr(" Total Row  ",r,c+=diff,attb);
	pstr(" Smoke Seat ",r,c+=diff,attb);
	pstr("NoSmoke Seat",r,c+=diff,attb);
	r=row+1; c=col+1;
	pstr("   First    ",r+=2,c,attb);
	pstr("   Business ",r+=2,c,attb);
	pstr("   Economy  ",r+=2,c,attb);
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


/* draw vertical line at column c */
void vertline(int c)
{
	int r, cellattb=0x6B;
	pchar(8,c,'Â',cellattb);
	for(r=9; r<17; r++)
		if ( (r) % 2 == 0)
			pchar(r,c,'Å',cellattb);
		else
			 pchar(r,c,'³',cellattb);
	pchar(16,c,'Á',cellattb);
}


/* Opens MDETAIL.DAT file for global access */
void airdetaildata(void)
{
	if ((fp = fopen("mdetail.dat", "r")) != NULL) {
		mdindex = fread(mdetail, sizeof(struct mastdetail), MAXAIRCRAFTS, fp);
		/* mdindex: next free structure position */
	}
	else if ( (fp = fopen("mdetail.dat", "w")) != NULL )	{
		msgbox("File does not exist:\"MDETAIL.DAT\". New file created", 0);
	}
	else	{
		msgbox("File creation error:\"MDETAIL.DAT\"", 0);
		putchar('\a');
		getch();
		return;
	}
}
