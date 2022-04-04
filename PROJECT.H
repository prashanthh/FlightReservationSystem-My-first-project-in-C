#define DATEFIELD	11

void pchar(int,int,char,int);
void pstr(char*, int,int,int);
void box(int c1,int r1,int c2,int r2,int attb, int shade);
void savescr(void); // saves enritre screen
void restorescr(void); // restore screen
char far *vid_mem = ( char far * ) 0xb8000000L ;
void scrfil(char, int);
int getip(int r, int c, char s[], int n);
void ipblock(int r, int c, int lt, int attb);
void clrline(int r);
void statbar(char *);
void dispdate(int, int);

char far *a;
char far *temp;
FILE *fp;

void savescr()
{
	char far *v;
	int j;
	v = vid_mem;
	if( (a = (char far *)malloc( 2 * 80 * 25)) == NULL )	{
		printf("\nNot enough memory");
		exit(0);
	}
	fp = fopen("outfile","w");
	temp = a;
	for(j=0; j< (2 * 80 * 25); j++ )	{
		*a++ = *v;
		fputc(*v++,fp);
	}
}


void restorescr(void)
{
	char far *v,c;
	int j;
	v = vid_mem;
	a = temp;
	fp = fopen("outfile","r");  // to flush back the the file so that it read from top
	for (j=0; j< (2 * 80 * 25); j++)
		*v++ = *a++;
		//*v++ = fgetc(fp);
	gotoxy(1,24);
}

void pchar(int r,int c,char ch,int attb)
// attb: the 8-bit sequence is blink,R,G,B,intensity,,R,G,B
{
	char far *v ;
	v = vid_mem + r * 160 + c * 2 ;
	*v = ch ;
	v++ ;
	*v = attb ;
}

void pstr( char *s, int r, int c, int attb )
{
	while ( *s != '\0' )
		pchar ( r, c++, *s++, attb ) ;
}

/*  box: creats a text-mode box */
void box(int c1,int r1,int c2,int r2,int attb, int shade)
{
	int i, c1bak, r1bak;

	pchar(r1,c1,'Ú',attb);
	pchar(r2,c1,'À',attb);
	c1bak = c1;
	r1bak = r1; // temp hold

	while( ++c1 < c2 ) {     // horiz line
		pchar(r1,c1,'Ä',attb);
		pchar(r2,c1,'Ä',attb);
	}
	pchar(r1,c2,'¿',attb);
	pchar(r2,c2,'Ù',attb);
	if (shade != -1)	{
		pchar(r2, c2+1, ' ', shade); /* ver bot shade */
		pchar(r2, c2+2, ' ', shade);
		for (i=c1bak+2; i <= c2+2; i++) /* bot shade */
			pchar(r2+1, i, ' ', shade);
	}

	while( ++r1 < r2 ) {     // vert line
		pchar(r1,c1bak,'³',attb);
		pchar(r1,c2,'³',attb);
		if (shade != -1)
			if (r1 > r1bak)	{
				pchar(r1,c2+1,' ',shade); /* vert shade */
				pchar(r1,c2+2,' ',shade);
			}
	}
}

void scrfil(char ch, int attb)
{
	int r, c;
	for (r = 0; r <= 24; r++)
		for(c=0; c < 80; c++)
			pchar(r,c,ch,attb);
}

void ipblock(int r, int c, int lt, int attb)
{
	int i;

	lt--;
	for (i=0; i<lt; i++)
		pchar(r, c++, ' ', attb);
}

int getip(int r, int c, char s[], int n)
{
	char ch;
	int i=0;

	gotoxy(c+1,r+1);
	//fflush(stdin);
	//while (n > 1 && (ch=getc(stdin)) != '\n' && ch != '%')	// n>1, because a space for \0
	while (n > 1 && (ch=getche()) != ENTER && ch != ESC)	{	// n>1, because a space for \0
		if (i==0 && ch == ' ')	continue; //ignore initial spaces
		if (i==0 && ch == '0')	continue; //ignore leading zeros
		if (i==0 && ch == '\b')	{
			//printf("\x1B[C"); //moves cursor right by one col
			putchar(' ');
			continue;
		}
		if (i != 0 && ch == '\b')	{
			i--;
			n++;
			putchar(' ');
			putchar('\b');
			continue;
		}
		n--;
		s[i++] = ch;
	}
	if (n==1)	{
		putchar('\b');
		while (getch() != ENTER)
			;
	}
	s[i] = '\0';
	fflush(stdin);
	if (ch == 27) // if ESC key
		return 0;
	return 1;
}


void clrline(int r)
{
	int i, c=0; // while calling "wchar", 0 to row/col is valid. But to "gotoxy()" it is invalid.
	for (i=0; i<80; i++)
		pchar(r,c++,' ',0x00);
}

void statbar(char *s)
{
	int col, row=23;
	for (col=1; col<79; col++)	//clear status bar
		pchar(row,col,' ',0x70);
	col = 2;
	pstr(s, row, col, 0x71);
}

void dispdate(int r, int c)
{
	struct dosdate_t d;

	_dos_getdate(&d);
	ipblock(r,c,DATEFIELD,0x31); // date block
	gotoxy(c+1,r+1);
	printf("%2d/%2d/%d", d.day, d.month, d.year);
}
