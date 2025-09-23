// This is a modified zlib v0.95 that will produce bit-exact output matching
// whatever tool was originally used to compress the *.bra files in the PC
// version of Tokyo Xanadu eX+. Do not use it for any other purpose, you're
// better off just using standard zlib instead.

/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* $Id: zutil.h,v 1.9 1995/05/03 17:27:12 jloup Exp $ */

#ifndef TXBRA__Z_UTIL_H
#define TXBRA__Z_UTIL_H

#include "zlib.h"

#include <stddef.h>
#include <errno.h>
#include <string.h>

#ifndef TXBRA_local
#  define TXBRA_local static
#endif
/* compile with -Dlocal if your debugger can't find static symbols */

typedef unsigned char  TXBRA_uch;
typedef TXBRA_uch TXBRA_FAR TXBRA_uchf;
typedef unsigned short TXBRA_ush;
typedef TXBRA_ush TXBRA_FAR TXBRA_ushf;
typedef unsigned long  TXBRA_ulg;

extern char *TXBRA_z_errmsg[]; /* indexed by 1-zlib_error */

#define TXBRA_ERR_RETURN(strm,err) return (strm->msg=TXBRA_z_errmsg[1-err], err)
/* To be used only when the state is known to be valid */

        /* common constants */

#define TXBRA_DEFLATED   8

#ifndef TXBRA_DEF_WBITS
#  define TXBRA_DEF_WBITS TXBRA_MAX_WBITS
#endif
/* default windowBits for decompression. TXBRA_MAX_WBITS is for compression only */

#if TXBRA_MAX_MEM_LEVEL >= 8
#  define TXBRA_DEF_MEM_LEVEL 8
#else
#  define TXBRA_DEF_MEM_LEVEL  TXBRA_MAX_MEM_LEVEL
#endif
/* default memLevel */

#define TXBRA_STORED_BLOCK 0
#define TXBRA_STATIC_TREES 1
#define TXBRA_DYN_TREES    2
/* The three kinds of block type */

#define TXBRA_MIN_MATCH  3
#define TXBRA_MAX_MATCH  258
/* The minimum and maximum match lengths */

         /* functions */

#if defined(pyr)
#  define TXBRA_NO_MEMCPY
#endif
#if (defined(M_I86SM) || defined(M_I86MM)) && !defined(_MSC_VER)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  */
#  define TXBRA_NO_MEMCPY
#endif
#if defined(STDC) && !defined(TXBRA_HAVE_MEMCPY) && !defined(TXBRA_NO_MEMCPY)
#  define TXBRA_HAVE_MEMCPY
#endif
#ifdef TXBRA_HAVE_MEMCPY
#  if defined(M_I86SM) || defined(M_I86MM) /* MSC small or medium model */
#    define TXBRA_zmemcpy _fmemcpy
#    define TXBRA_zmemzero(dest, len) _fmemset(dest, 0, len)
#  else
#    define TXBRA_zmemcpy memcpy
#    define TXBRA_zmemzero(dest, len) memset(dest, 0, len)
#  endif
#else
   extern void TXBRA_zmemcpy  OF((TXBRA_Bytef* dest, TXBRA_Bytef* source, TXBRA_uInt len));
   extern void TXBRA_zmemzero OF((TXBRA_Bytef* dest, TXBRA_uInt len));
#endif

/* Diagnostic functions */
#ifdef DEBUG
#  include <stdio.h>
#  ifndef TXBRA_verbose
#    define TXBRA_verbose 0
#  endif
#  define TXBRA_Assert(cond,msg) {if(!(cond)) TXBRA_z_error(msg);}
#  define TXBRA_Trace(x) fprintf x
#  define TXBRA_Tracev(x) {if (TXBRA_verbose) fprintf x ;}
#  define TXBRA_Tracevv(x) {if (TXBRA_verbose>1) fprintf x ;}
#  define TXBRA_Tracec(c,x) {if (TXBRA_verbose && (c)) fprintf x ;}
#  define TXBRA_Tracecv(c,x) {if (TXBRA_verbose>1 && (c)) fprintf x ;}
#else
#  define TXBRA_Assert(cond,msg)
#  define TXBRA_Trace(x)
#  define TXBRA_Tracev(x)
#  define TXBRA_Tracevv(x)
#  define TXBRA_Tracec(c,x)
#  define TXBRA_Tracecv(c,x)
#endif


typedef TXBRA_uLong (*TXBRA_check_func) OF((TXBRA_uLong check, TXBRA_Bytef *buf, TXBRA_uInt len));

extern void TXBRA_z_error    OF((char *m));

TXBRA_voidpf TXBRA_zcalloc OF((TXBRA_voidpf opaque, unsigned items, unsigned size));
void   TXBRA_zcfree  OF((TXBRA_voidpf opaque, TXBRA_voidpf ptr));

#define TXBRA_ZALLOC(strm, items, size) \
           (*((strm)->zalloc))((strm)->opaque, (items), (size))
#define TXBRA_ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (TXBRA_voidpf)(addr))
#define TXBRA_TRY_FREE(s, p) {if (p) TXBRA_ZFREE(s, p);}

#endif /* TXBRA__Z_UTIL_H */
