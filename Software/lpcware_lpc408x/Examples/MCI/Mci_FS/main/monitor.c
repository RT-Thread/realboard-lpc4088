/*------------------------------------------------------------------------*/
/* Universal string handler for user console interface                    */
/*------------------------------------------------------------------------*/
/*
/  Copyright (C) 2010, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/

#include <stdarg.h>
#include "monitor.h"

#if _USE_XFUNC_IN
unsigned char (*xfunc_in)(void);    /* Pointer to the input stream */
static char *outptr;
#endif
#if _USE_XFUNC_OUT
void (*xfunc_out)(unsigned char);   /* Pointer to the output stream */
#endif



#if _USE_XFUNC_OUT
/*----------------------------------------------*/
/* Put a character                              */
/*----------------------------------------------*/

void xputc (char c)
{
    if (_CR_CRLF && c == '\n') xfunc_out('\r');     /* CR -> CRLF */

    if (outptr) {
        *outptr++ = (unsigned char)c;
        return;
    }

    xfunc_out((unsigned char)c);
}


/*----------------------------------------------*/
/* Put a null-terminated string                 */
/*----------------------------------------------*/

void xputs (const char* str)
{
    while (*str)
        xputc(*str++);
}

/*----------------------------------------------*/
/* Formatted string output                      */
/*----------------------------------------------*/
/*  xprintf("%d", 1234);            "1234"
    xprintf("%6d,%3d%%", -200, 5);  "  -200,  5%"
    xprintf("%-6u", 100);           "100   "
    xprintf("%ld", 12345678L);      "12345678"
    xprintf("%04x", 0xA3);          "00a3"
    xprintf("%08LX", 0x123ABC);     "00123ABC"
    xprintf("%016b", 0x550F);       "0101010100001111"
    xprintf("%s", "String");        "String"
    xprintf("%-4s", "abc");         "abc "
    xprintf("%4s", "abc");          " abc"
    xprintf("%c", 'a');             "a"
    xprintf("%f", 10.0);            <xprintf lacks floating point support>
*/

static
void xvprintf (
    const char* fmt,    /* Pointer to the format string */
    va_list arp         /* Pointer to arguments */
)
{
    unsigned int r, i, j, w, f;
    unsigned long v;
    char s[16], c, d, *p;


    for (;;) {
        c = *fmt++;                 /* Get a char */
        if (!c) break;              /* End of format? */
        if (c != '%') {             /* Pass through it if not a % sequense */
            xputc(c); continue;
        }
        f = 0;
        c = *fmt++;                 /* Get first char of the sequense */
        if (c == '0') {             /* Flag: '0' padded */
            f = 1; c = *fmt++;
        } else {
            if (c == '-') {         /* Flag: left justified */
                f = 2; c = *fmt++;
            }
        }
        for (w = 0; c >= '0' && c <= '9'; c = *fmt++)   /* Minimum width */
            w = w * 10 + c - '0';
        if (c == 'l' || c == 'L') { /* Prefix: Size is long int */
            f |= 4; c = *fmt++;
        }
        if (!c) break;              /* End of format? */
        d = c;
        if (d >= 'a') d -= 0x20;
        switch (d) {                /* Type is... */
        case 'S' :                  /* String */
            p = va_arg(arp, char*);
            for (j = 0; p[j]; j++) ;
            while (!(f & 2) && j++ < w) xputc(' ');
            xputs(p);
            while (j++ < w) xputc(' ');
            continue;
        case 'C' :                  /* Character */
            xputc((char)va_arg(arp, int)); continue;
        case 'B' :                  /* Binary */
            r = 2; break;
        case 'O' :                  /* Octal */
            r = 8; break;
        case 'D' :                  /* Signed decimal */
        case 'U' :                  /* Unsigned decimal */
            r = 10; break;
        case 'X' :                  /* Hexdecimal */
            r = 16; break;
        default:                    /* Unknown type (passthrough) */
            xputc(c); continue;
        }

        /* Get an argument and put it in numeral */
        v = (f & 4) ? va_arg(arp, long) : ((d == 'D') ? (long)va_arg(arp, int) : va_arg(arp, unsigned int));
        if (d == 'D' && (v & 0x80000000)) {
            v = 0 - v;
            f |= 8;
        }
        i = 0;
        do {
            d = (char)(v % r); v /= r;
            if (d > 9) d += (c == 'x') ? 0x27 : 0x07;
            s[i++] = d + '0';
        } while (v && i < sizeof(s));
        if (f & 8) s[i++] = '-';
        j = i; d = (f & 1) ? '0' : ' ';
        while (!(f & 2) && j++ < w) xputc(d);
        do xputc(s[--i]); while(i);
        while (j++ < w) xputc(' ');
    }
}

void xprintf (
    const char* fmt,    /* Pointer to the format string */
    ...                 /* Optional arguments */
)
{
    va_list arp;


    va_start(arp, fmt);
    xvprintf(fmt, arp);
    va_end(arp);
}

void xsprintf (
    char* buff,         /* Pointer to the output buffer */
    const char* fmt,    /* Pointer to the format string */
    ...                 /* Optional arguments */
)
{
    va_list arp;


    outptr = buff;      /* Switch destination for memory */

    va_start(arp, fmt);
    xvprintf(fmt, arp);
    va_end(arp);

    *outptr = 0;        /* Terminate output string */
    outptr = 0;         /* Switch destination for device */
}

/*----------------------------------------------*/
/* Dump a block of byte array                   */
/*----------------------------------------------*/

void put_dump (
    const void* buff,       /* Pointer to the byte array to be dumped */
    unsigned long addr,     /* Heading address value */
    int len                 /* Number of bytes to be dumped */
)
{
    int i;
    const unsigned char *p = buff;


    xprintf("%08lX ", addr);        /* address */

    for (i = 0; i < len; i++)       /* data (hexdecimal) */
        xprintf(" %02X", p[i]);

    xputc(' ');
    for (i = 0; i < len; i++)       /* data (ascii) */
        xputc((p[i] >= ' ' && p[i] <= '~') ? p[i] : '.');

    xputc('\n');
}

#endif /* _USE_XFUNC_OUT */



#if _USE_XFUNC_IN
/*----------------------------------------------*/
/* Get a value of the string                    */
/*----------------------------------------------*/
/*  "123 -5   0x3ff 0b1111 0377  w "
        ^                           1st call returns 123 and next ptr
           ^                        2nd call returns -5 and next ptr
                   ^                3rd call returns 1023 and next ptr
                          ^         4th call returns 15 and next ptr
                               ^    5th call returns 255 and next ptr
                                  ^ 6th call fails and returns 0
*/

int xatoi (         /* 0:Failed, 1:Successful */
    char **str,     /* Pointer to pointer to the string */
    long *res       /* Pointer to the valiable to store the value */
)
{
    unsigned long val;
    unsigned char c, r, s = 0;


    *res = 0;

    while ((c = **str) == ' ') (*str)++;    /* Skip leading spaces */

    if (c == '-') {     /* negative? */
        s = 1;
        c = *(++(*str));
    }

    if (c == '0') {
        c = *(++(*str));
        switch (c) {
        case 'x':       /* hexdecimal */
            r = 16; c = *(++(*str));
            break;
        case 'b':       /* binary */
            r = 2; c = *(++(*str));
            break;
        default:
            if (c <= ' ') return 1; /* single zero */
            if (c < '0' || c > '9') return 0;   /* invalid char */
            r = 8;      /* octal */
        }
    } else {
        if (c < '0' || c > '9') return 0;   /* EOL or invalid char */
        r = 10;         /* decimal */
    }

    val = 0;
    while (c > ' ') {
        if (c >= 'a') c -= 0x20;
        c -= '0';
        if (c >= 17) {
            c -= 7;
            if (c <= 9) return 0;   /* invalid char */
        }
        if (c >= r) return 0;       /* invalid char for current radix */
        val = val * r + c;
        c = *(++(*str));
    }
    if (s) val = 0 - val;           /* apply sign if needed */

    *res = val;
    return 1;
}


/*----------------------------------------------*/
/* Get a line from the input                    */
/*----------------------------------------------*/

int get_line (      /* 0:End of stream, 1:A line arrived */
    char* buff,     /* Pointer to the buffer */
    int len         /* Buffer length */
)
{
    int c, i;

    i = 0;
    for (;;) {
        c = xfunc_in();             /* Get a char from the incoming stream */
        if (!c) return 0;           /* End of stream? */
        if (c == '\r') break;       /* End of line? */
        if (c == '\b' && i) {       /* Back space? */
            i--;
            if (_LINE_ECHO) xputc(c);
            xputc(' '); // mthomas
            xputc(c);   // mthomas
            continue;
        }
        if (c >= ' ' && i < len - 1) {  /* Visible chars */
            buff[i++] = c;
            if (_LINE_ECHO) xputc(c);
        }
    }
    buff[i] = 0;    /* Terminate with zero */
    if (_LINE_ECHO) xputc('\n');
    return 1;
}

#endif /* _USE_XFUNC_IN */


#if _USE_XSTRFUNC
/*----------------------------------------------*/
/* Simple string functions                      */
/*----------------------------------------------*/

/* Return the length of the null-terminated string str. */
int xstrlen (const char *str)
{
    register int n = 0;
     
    while (*str++) n++;
    return n;
}

/* Copy src to dst.  */
char *xstrcpy (char* dst, const char* src)
{
    register char *d = dst;

    while((*d++=*src++) !='\0');
    return dst;
}


void *xmemset (void *p, int c, int sz)
{
    register char *pf = (char*)p;

    while (sz--) *pf++ = (char)c;
    return p;
}

/* Find the first occurrence of C in str. */
char *xstrchr (const char *str, int c)
{
    while (*str) {
        if (*str == (char)c) {
            return (char*)str;
        }
        str++;
    }
    return 0;
}

#endif /* _USE_XSTRFUNC */
