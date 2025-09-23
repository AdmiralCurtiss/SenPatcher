// This is a modified zlib v0.95 that will produce bit-exact output matching
// whatever tool was originally used to compress the *.bra files in the PC
// version of Tokyo Xanadu eX+. Do not use it for any other purpose, you're
// better off just using standard zlib instead.

/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

/* $Id: zutil.c,v 1.8 1995/05/03 17:27:12 jloup Exp $ */

#include <stdio.h>

#include "zutil.h"

struct TXBRA_internal_state      {int dummy;}; /* for buggy compilers */

#ifndef __GO32__
extern void exit OF((int));
#endif

char *TXBRA_z_errmsg[] = {
"stream end",          /* TXBRA_Z_STREAM_END    1 */
"",                    /* TXBRA_Z_OK            0 */
"file error",          /* TXBRA_Z_ERRNO        (-1) */
"stream error",        /* TXBRA_Z_STREAM_ERROR (-2) */
"data error",          /* TXBRA_Z_DATA_ERROR   (-3) */
"insufficient memory", /* TXBRA_Z_MEM_ERROR    (-4) */
"buffer error",        /* TXBRA_Z_BUF_ERROR    (-5) */
""};


void TXBRA_z_error (m)
    char *m;
{
    fprintf(stderr, "%s\n", m);
    exit(1);
}

#ifndef TXBRA_HAVE_MEMCPY

void TXBRA_zmemcpy(dest, source, len)
    TXBRA_Bytef* dest;
    TXBRA_Bytef* source;
    TXBRA_uInt  len;
{
    if (len == 0) return;
    do {
        *dest++ = *source++; /* ??? to be unrolled */
    } while (--len != 0);
}

void TXBRA_zmemzero(dest, len)
    TXBRA_Bytef* dest;
    TXBRA_uInt  len;
{
    if (len == 0) return;
    do {
        *dest++ = 0;  /* ??? to be unrolled */
    } while (--len != 0);
}
#endif

#if defined( __TURBOC__) && !defined(__SMALL__) && !defined(__MEDIUM__)
/* Small and medium model are for now limited to near allocation with
 * reduced TXBRA_MAX_WBITS and TXBRA_MAX_MEM_LEVEL
 */
#  define MY_ZCALLOC

/* Turbo C malloc() does not allow dynamic allocation of 64K bytes
 * and farmalloc(64K) returns a pointer with an offset of 8, so we
 * must fix the pointer. Warning: the pointer must be put back to its
 * original form in order to free it, use TXBRA_zcfree().
 */

#define MAX_PTR 10
/* 10*64K = 640K */

TXBRA_local int next_ptr = 0;

typedef struct ptr_table_s {
    TXBRA_voidpf org_ptr;
    TXBRA_voidpf new_ptr;
} ptr_table;

TXBRA_local ptr_table table[MAX_PTR];
/* This table is used to remember the original form of pointers
 * to large buffers (64K). Such pointers are normalized with a zero offset.
 * Since MSDOS is not a preemptive multitasking OS, this table is not
 * protected from concurrent access. This hack doesn't work anyway on
 * a protected system like OS/2. Use Microsoft C instead.
 */

TXBRA_voidpf TXBRA_zcalloc (TXBRA_voidpf opaque, unsigned items, unsigned size)
{
    TXBRA_voidpf buf = opaque; /* just to make some compilers happy */
    TXBRA_ulg bsize = (TXBRA_ulg)items*size;

    if (bsize < 65536L) {
        buf = farmalloc(bsize);
        if (*(TXBRA_ush*)&buf != 0) return buf;
    } else {
        buf = farmalloc(bsize + 16L);
    }
    if (buf == NULL || next_ptr >= MAX_PTR) return NULL;
    table[next_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((TXBRA_ush*)&buf+1) += ((TXBRA_ush)((TXBRA_uch*)buf-0) + 15) >> 4;
    *(TXBRA_ush*)&buf = 0;
    table[next_ptr++].new_ptr = buf;
    return buf;
}

void  TXBRA_zcfree (TXBRA_voidpf opaque, TXBRA_voidpf ptr)
{
    int n;
    if (*(TXBRA_ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        return;
    }
    /* Find the original pointer */
    for (n = 0; n < next_ptr; n++) {
        if (ptr != table[n].new_ptr) continue;

        farfree(table[n].org_ptr);
        while (++n < next_ptr) {
            table[n-1] = table[n];
        }
        next_ptr--;
        return;
    }
    ptr = opaque; /* just to make some compilers happy */
    TXBRA_z_error("TXBRA_zcfree: ptr not found");
}
#endif /* __TURBOC__ */

#if defined(M_I86SM)||defined(M_I86MM)||defined(M_I86CM)||defined(M_I86LM)
/* Microsoft C */

#  define MY_ZCALLOC

#if (!defined(_MSC_VER) || (_MSC_VER < 600))
#  define _halloc  halloc
#  define _hfree   hfree
#endif

TXBRA_voidpf TXBRA_zcalloc (TXBRA_voidpf opaque, unsigned items, unsigned size)
{
    if (opaque) opaque = 0; /* to make compiler happy */
    return _halloc((long)items, size);
}

void  TXBRA_zcfree (TXBRA_voidpf opaque, TXBRA_voidpf ptr)
{
    if (opaque) opaque = 0; /* to make compiler happy */
    _hfree(ptr);
}

#endif /* MSC */


#ifndef MY_ZCALLOC /* Any system without a special alloc function */

#ifndef __GO32__
extern TXBRA_voidp  calloc OF((TXBRA_uInt items, TXBRA_uInt size));
extern void   free   OF((TXBRA_voidpf ptr));
#endif

TXBRA_voidpf TXBRA_zcalloc (opaque, items, size)
    TXBRA_voidpf opaque;
    unsigned items;
    unsigned size;
{
    return (TXBRA_voidpf)calloc(items, size);
}

void  TXBRA_zcfree (opaque, ptr)
    TXBRA_voidpf opaque;
    TXBRA_voidpf ptr;
{
    free(ptr);
}

#endif /* MY_ZCALLOC */
