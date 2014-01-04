/*------------------------------------------------------------------------*/
/* Universal string handler for user console interface  (C)ChaN, 2010     */
/*------------------------------------------------------------------------*/

#ifndef _STRFUNC
#define _STRFUNC

#define _USE_XFUNC_OUT	1	/* 1: Use output functions */
#define	_CR_CRLF		1	/* 1: Convert \n ==> \r\n in the output char */

#define _USE_XFUNC_IN	1	/* 1: Use input function */
#define	_LINE_ECHO		1	/* 1: Echo back input chars in get_line function */

/* 1: use the simple string functions in monitor.c
        It results in reduced code size but low efficiency
   0: use the string functions in library, should add <string.h>
        It results in high efficiency but expanded code size 
*/
#define _USE_XSTRFUNC	1

#if _USE_XFUNC_OUT
extern void (*xfunc_out)(unsigned char);
void xputc (char c);
void xputs (const char* str);
void xprintf (const char* fmt, ...);
void xsprintf (char* buff, const char* fmt, ...);
void put_dump (const void* buff, unsigned long addr, int len);
#endif

#if _USE_XFUNC_IN
extern unsigned char (*xfunc_in)(void);
int get_line (char* buff, int len);
int xatoi (char** str, long* res);
#endif

#if _USE_XSTRFUNC
int xstrlen (const char*);
char *xstrcpy (char*, const char*);
void *xmemset (void*, int, int);
char *xstrchr (const char*, int);
#endif

#endif
