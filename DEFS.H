/* Declarations Header File */
#include<dos.h> /* for date structure */

/* ascii and scan key codes */
#define ESC		27
#define ENTER	13
#define BKSPACE	8
#define	TAB		9
#define	SHTAB	143
#define DOWN	80+128
#define LEFT	75+128
#define RIGHT	77+128
#define UP		72+128
#define HOME	71+128
#define INS		128+82
#define END		79+128
#define DEL		83+128

#define	F1		59+128
#define	F2		60+128
#define	F3		61+128
#define	F4		62+128
#define	F5		63+128
#define	F6		64+128
#define	F7		65+128
#define	F8		66+128
#define	F9		67+128
#define	F10		68+128

/* MASTER.C Defines. Also used by other files */
#define MAXAIRCRAFTS	15
#define MAXCODE			5 /* for char strings, an extra space for '\0' at the end */
#define MAXNAME			16
#define MAXSEATDIG		5
#define MAXACSEATS		400 /* max. aircraft seats */

struct airmaster	{
	char code[MAXCODE];
	char name[MAXNAME];
	short int seats;
};

/* MDETAIL.C defines */
struct mastdetail	{
	int aircode;	/* stores the index no. of structure mast */
	int seats[3][4]; /* 3 X 4 cells */
};

/* FLIGHT.C Defines. */
#define MAXFLIGHTS	15
#define	FLINOLT		6
#define	SECTORLT	20
#define	OP_DAYSLT	16
#define	DEP_DIG		6
#define	ARRIV_DIG	6
#define	F_FAREDIG	10
#define	B_FAREDIG	10
#define	E_FAREDIG	10

struct airflight	{
	char flino[FLINOLT];	/* Flight no. */
	char sector[SECTORLT];	/* Sector */
	char op_days[OP_DAYSLT];	/* Operating days */
	float dep;		/* Departure time */
	float arriv;	/* Arrival time */
	float f_fare;	/* First class fare */
	float b_fare;	/* Business class fare */
	float e_fare;	/* Economy class fare */
	int aircode;	/* stores the index no. of structure mast */
};

/* ENQUIRY.C defines */
#define PNRLT	15
#define DATELT	11
#define TIMELT	6
struct res_manifesto
{
	char pnr[PNRLT]; /* consists company code, date and serial no. */
	short flino; /* stores index no of struct flight */
	struct dosdate_t depdate; /* library structure */
	float deptime; /* time stored as float */
};

/* stores seat details of each flight */
#define MAXCONF_FLIGHTS	10 	/* max. flights handled for confirmation */
struct con
{
	int day, month, year, flino;
	int reserve; /* 0 value indicates this particular index is not used for
	reservation. 1 indicates opposite */
	int confirmed[MAXACSEATS]; /* used to represent reserved seats. Free seats are
	represented by value 0. Reserved seats are by storing seat no itself */
};

/* common for CONFIRM.C, WAIT.C and CANCEL.C */
#define PHONELT	   	8 /* phone length */
#define NAMELT		26 /* name length */

/* CONFIRM.C defines */
#define MAXCONFIRMS	50
struct reservation
{
	short status; /* if 1, this record is being used. If 0, record is unused */
	char pnr[PNRLT];
	short flino; /* stores index no of struct flight */
	struct dosdate_t depdate; /* library structure */
	float deptime; /* time stored as float */
	short totseat;
	char phone[PHONELT];
	char clas[4];
	char so[4];
	short int seatno[4];
	char name[4][NAMELT];
	short age[4];
	char sex[4];
	char mo[4];
	char ssr[4];
	float fare[4];
};

/* WAIT.C defines */
#define MAXWAITS	20
struct waits
{
	short status; /* if 1, this record is being used. If 0, record is unused */
	char pnr[PNRLT];
	short flino; /* stores index no of struct flight */
	struct dosdate_t depdate; /* library structure */
	float deptime; /* time stored as float */
	short totseat;
	char phone[PHONELT];
	char clas[4];
	char so[4];
	char name[4][NAMELT];
	short age[4];
	char sex[4];
	char mo[4];
	char ssr[4];
	float fare[4];
};

/* CANCEL.C defines */
struct canc
{
	short flino; /* stores index no of struct flight */
	short totseat;
	char phone[PHONELT];
	short int seatno[4];
	char clas[4];
	char so[4];
	char name[4][NAMELT];
	short age[4];
	char sex[4];
	char mo[4];
	char ssr[4];
	float fare[4];
};

/* declarations for commonly used functions. The definitions are in function.h */
void pchar(int,int,char,int);
void pstr(char*, int,int,int);
void box(int c1,int r1,int c2,int r2,int attb, int shade);
void savescr(int r, int c, int rr, int cc, char *p);
void restorescr(int r, int c, int rr, int cc, char *p);
void scrfil(char, int);
int getip(int r, int c, char s[], int n);
void ipblock(int r, int c, int lt, int attb);
void statbar(char *);
void dispdate(int, int, int);
void textline(int r, int c, int cc, int attb);
int scan(void);
int modi(char *s, int limit, int row, int col, int attb);
int msgbox(char *s, int type);
void errbeep(void);