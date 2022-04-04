/* Commonly used function definitions */
#include<stdio.h>
#include<conio.h>
#include<dos.h>
#include<alloc.h>
#include"defs.h" /* local file containing define statements & function declarations */

#define DATEFIELD	11

/* video memory address of standard */
char far *vid_mem = ( char far * ) 0xb8000000L;


/* saves screen contents of selected region into memory */
void savescr(int r, int c, int rr, int cc, char *p)
{
	int i,j,skip,start;
	char far *v;

	start = (r * 80 + c) * 2;
	v = vid_mem + start;
	for (i=0; i< rr-r+1; i++)	{
		for (j=0; j < (cc-c+1) * 2; j++, v++, p++)
			*p = *v;
		skip = (80 - (cc-c+1)) * 2;
		v += skip;
	}
}


/* outputs contents of memory onto selected region on screen */
void restorescr(int r, int c, int rr, int cc, char *p)
{
	int i,j,skip,start;
	char far *v;

	start = (r * 80 + c) * 2;
	v = vid_mem + start;
	for (i=0; i< rr-r+1; i++)	{
		for (j=0; j < (cc-c+1) * 2; j++)
			*v++ = *p++;
		skip = (80 - (cc-c+1)) * 2;
		v += skip;
	}
}


/* outputs a character on screen */
void pchar(int r,int c,char ch,int attb)
{
	char far *v;

	v = vid_mem + r * 160 + c * 2;
	*v = ch;
	v++;
	*v = attb;
}


/* outputs a given string onto specified co-ordinates on screen */
void pstr( char *s, int r, int c, int attb )
{
	while ( *s != '\0' )
		pchar ( r, c++, *s++, attb ) ;
}


/* creats a text-mode box, with shade if shade is not equal to -1 */
void box(int c1,int r1,int c2,int r2,int attb, int shade)
{
	int i, c1bak, r1bak;

	pchar(r1,c1,'Ú',attb);
	pchar(r2,c1,'À',attb);
	c1bak = c1;
	r1bak = r1; /* temp hold */

	while( ++c1 < c2 ) {     /* horiz line */
		pchar(r1,c1,'Ä',attb);
		pchar(r2,c1,'Ä',attb);
	}
	pchar(r1,c2,'¿',attb);
	pchar(r2,c2,'Ù',attb);
	/* draws shade */
	if (shade != -1)	{
		pchar(r2, c2+1, ' ', shade); /* ver bot shade */
		pchar(r2, c2+2, ' ', shade);
		for (i=c1bak+2; i <= c2+2; i++) /* bot shade */
			pchar(r2+1, i, ' ', shade);
	}

	while( ++r1 < r2 ) {     /* vert line */
		pchar(r1,c1bak,'³',attb);
		pchar(r1,c2,'³',attb);
		if (shade != -1)
			if (r1 > r1bak)	{
				pchar(r1,c2+1,' ',shade); /* vert shade */
				pchar(r1,c2+2,' ',shade);
			}
	}
}


/* fills screen with specified character with given attribute */
void scrfil(char ch, int attb)
{
	int r, c;
	for (r = 0; r <= 24; r++)
		for(c=0; c < 80; c++)
			pchar(r,c,ch,attb);
}


/* outputs a block of given lenth used mostly as input field */
void ipblock(int r, int c, int lt, int attb)
{
	int i;

	lt--;
	for (i=0; i<lt; i++)
		pchar(r, c++, ' ', attb);
}


/* getip: gets formatted input from row r and column n and stores it in
 s not exeeding the width
 returns 1 for success, 0 if user hits ESC */
int getip(int row, int col, char *s, int width)
{
	int ch, i=0;

	gotoxy(col+1, row+1);
	for (;;)	{
		ch = getch();
		if (ch==' ' && i==0)
			continue;
		if (ch=='\b')
			if (i==width-1)	{ /* if at the end of boundary */
				i--;
				cprintf("\b \b");
				s[width-3] = s[width-2];
				cprintf("%c \b",s[width-3]);
				continue;
			}
			else if (i != 0)	{
				i--;
				cprintf("\b \b");
				continue;
			}
			else
				continue;
		if (ch==0)	{
			getch();
				continue;
		}
		if (ch == ENTER)	{
			s[i]='\0';
			return 1;
		}
		if (ch==ESC)
			return 0;
		if (i < width-1)	{
			putch(s[i]=ch);
			if (i==width-2) /* if out of boundary */
				putchar('\b');
			i++;
		}
	}
}


/* Displays given string in status bar */
void statbar(char *s)
{
	int col, row=23;

	for (col=1; col<79; col++)	/* clear status bar */
		pchar(row,col,' ',0x70);
	col = 2;
	pstr(s, row, col, 0x71);
}


/* displays system date in given attribute */
void dispdate(int r, int c, int attb)
{
	struct dosdate_t d;

	_dos_getdate(&d);
	gotoxy(c+1,r+1);
	textattr(attb);
	cprintf("%d/%d/%d", d.day, d.month, d.year);
}


/* draws a line from column c to column cc */
void textline(int r, int c, int cc, int attb)
{
	int i;
	for (i=c; i <= cc; i++)
		pchar(r,i,'Ä',attb);
}


/* Gets a single char input and returns scan code of the key */
int scan(void)
{
	int ch;

	ch=getch();
	return ch==0 ? getch()+128: ch;
}


/* allows user to modify the given string */
/* limit: max field width
returns -1 if ESC hit before any modification
		 0 if ESC hit after any modification
		 1 if successful modification */
int modi(char *s, int limit, int row, int col, int attb)
{
	int ch, i,j,l;

	limit -= 2; /* one decrement for '\0' and another because count begins from 0 */
	i=0;
	gotoxy(col+1, row+1);
	if ((ch=scan()) == ESC)
		return -1; /* if ESC hit before any modification */

	for(;;)	{
		switch(ch)	{
		case RIGHT:
			if (i < strlen(s) && i < limit)	{
				i++;
				gotoxy(wherex()+1,wherey());
			}
			break;
		case LEFT:
			if (i > 0)	{
				i--;
				gotoxy(wherex()-1,wherey());
			}
			break;
		case DEL:
			if (i == strlen(s)) /* if cursor at end */
				break;
			for (j=i; s[j] !=  '\0'; j++)
				s[j]=s[j+1];
			pstr(s,row,col,attb);
			pchar(row,col+j-1,' ',attb);
			break;
		case BKSPACE:
			if (i==0)
				break;
			for (j=i; s[j] != '\0'; j++)
				s[j-1] = s[j];
			s[j-1] = '\0';
			i--;
			gotoxy(wherex()-1,wherey());
			pstr(s, row, col, attb);
			pchar(row, col+j-1, ' ', attb);
			break;
		case HOME:
			i=0;
			gotoxy(col+1,row+1);
			break;
		case END:
			if ((i=strlen(s)) == limit+1) /* if last char is at the end of field
			width */
				i = limit;
			gotoxy(col+1+i,row+1);
			break;
		case ESC:
			return 0;
		case ENTER:
			return 1;
		default:
			l=strlen(s);
			l--;
			if (isalnum(ch) || ch == '.')	{
				if (l==limit)
					break;
				/* push characters starting from NULL char */
				for (j=l+1; j >= i; j--)
					s[j+1] = s[j];
				s[i]=ch;
				pstr(s, row, col, attb);
				if (i < limit)	{
					gotoxy(wherex()+1, wherey());
					i++;
				}
			}
			else if (ch == ' ' && i != 0 && i != strlen(s))	{
				if (l==limit)
					break;
				/* push character starting from NULL char */
				for (j=l+1; j >= i; j--)
					s[j+1] = s[j];
				s[i]=ch;
				pstr(s, row, col, attb);
				if (i < limit)	{
					gotoxy(wherex()+1, wherey());
					i++;
				}
			}
			break;
		}
		ch=scan();
	}
}


/* msgbox: outputs a message box displaying string s. If type is 0, just
prints a message, if 1, asks for confirmation (Y/N ?) */
int msgbox(char *s, int type)
{
	char *p;
	int i, j, n, ch, c1, c2, r1, r2;

	n = strlen(s) / 2;
	c1= 40 - (n+2);
	c2 = 40 + n + 2;
	r1=8;
	r2=15;

	if ((p = (char *)malloc(1600)) == NULL)	{
		printf("Not Enough Memory");
		exit(1);
	}
	savescr(r1,c1,r2+1,c2+2,p);
	box(c1,r1,c2,r2,0x7F,0x00);

	for (i=r1+1; i<=r2-1; i++)	/* fill box */
		for (j=c1+1; j<=c2-1; j++)
			pchar(i,j,' ',0x44);
	if (type == 0)	{ /* print message */
		pstr(s,11,c1+2,0x4F);
		pstr("   OK   ",13,36,0x2F);
		while ((ch=getch()) != ENTER)
			;
		restorescr(r1,c1,r2+1,c2+2,p);
		free(p);
		return 0;
	}
	if (type == 1)	{ /* confirmation box */
		pstr(s,11,c1+2,0x4F);
		pstr("Y/N",13,37,0x4E);
		pchar(13,42,' ',0x2F);
		gotoxy(43,14);
		while ((ch=toupper(getch())) != 'Y' && ch != 'N' && ch != ESC)
			;
		restorescr(r1,c1,r2+1,c2+2,p);
		free(p);
		return ch;
	}
	free(p);
	return -1;
}


/* generates a error sound */
void errbeep(void)
{
	sound(550);
	delay(25);
	nosound();
}
