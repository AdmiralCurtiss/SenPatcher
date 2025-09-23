// This is a modified zlib v0.95 that will produce bit-exact output matching
// whatever tool was originally used to compress the *.bra files in the PC
// version of Tokyo Xanadu eX+. Do not use it for any other purpose, you're
// better off just using standard zlib instead.

/* zconf.h -- configuration of the zlib compression library
 * Copyright (C) 1995 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

/* $Id: zconf.h,v 1.12 1995/05/03 17:27:12 jloup Exp $ */

#ifndef TXBRA__ZCONF_H
#define TXBRA__ZCONF_H

#define STDC

/*
     The library does not install any signal handler. It is recommended to
  add at least a handler for SIGSEGV when decompressing; the library checks
  the consistency of the input data whenever possible but may go nuts
  for some forms of corrupted input.
 */

/*
 * Compile with -DMAXSEG_64K if the alloc function cannot allocate more
 * than 64k bytes at a time (needed on systems with 16-bit int).
 */
#if defined(_GNUC__) && !defined(__32BIT__)
#  define __32BIT__
#endif
#if defined(__MSDOS__) && !defined(MSDOS)
#  define MSDOS
#endif
#ifdef MSDOS
#  define UNALIGNED_OK
#endif

#ifdef	__MWERKS__ /* Metrowerks CodeWarrior declares fileno() in unix.h */
#  include <unix.h>
#endif

/* Maximum value for memLevel in deflateInit2 */
#ifndef TXBRA_MAX_MEM_LEVEL
#  define TXBRA_MAX_MEM_LEVEL 9
#endif

/* Maximum value for windowBits in deflateInit2 and inflateInit2 */
#ifndef TXBRA_MAX_WBITS
#  define TXBRA_MAX_WBITS   15 /* 32K LZ77 window */
#endif

/* The memory requirements for deflate are (in bytes):
            1 << (windowBits+2)   +  1 << (memLevel+9)
 that is: 128K for windowBits=15  +  128K for memLevel = 8  (default values)
 plus a few kilobytes for small objects. For example, if you want to reduce
 the default memory requirements from 256K to 128K, compile with
     make CFLAGS="-O -DMAX_WBITS=14 -DMAX_MEM_LEVEL=7"
 Of course this will generally degrade compression (there's no free lunch).

   The memory requirements for inflate are (in bytes) 1 << windowBits
 that is, 32K for windowBits=15 (default value) plus a few kilobytes
 for small objects.
*/

                        /* Type declarations */

#ifndef OF /* function prototypes */
#  ifdef STDC
#    define OF(args)  args
#  else
#    define OF(args)  ()
#  endif
#endif

/* The following definitions for TXBRA_FAR are needed only for MSDOS mixed
 * model programming (small or medium model with some far allocations).
 * This was tested only with MSC; for other MSDOS compilers you may have
 * to define TXBRA_NO_MEMCPY in zutil.h.  If you don't need the mixed model,
 * just define TXBRA_FAR to be empty.
 */
#if defined(M_I86SM) || defined(M_I86MM) /* MSC small or medium model */
#  ifdef _MSC_VER
#    define TXBRA_FAR __far
#  else
#    define TXBRA_FAR far
#  endif
#endif
#if defined(__BORLANDC__) && (defined(__SMALL__) || defined(__MEDIUM__))
#    define TXBRA_FAR __far /* completely untested, just a best guess */
#endif
#ifndef TXBRA_FAR
#   define TXBRA_FAR
#endif

typedef unsigned char  TXBRA_Byte;  /* 8 bits */
typedef unsigned int   TXBRA_uInt;  /* 16 bits or more */
typedef unsigned long  TXBRA_uLong; /* 32 bits or more */

typedef TXBRA_Byte TXBRA_FAR TXBRA_Bytef;
typedef char TXBRA_FAR TXBRA_charf;
typedef int TXBRA_FAR TXBRA_intf;
typedef TXBRA_uInt TXBRA_FAR TXBRA_uIntf;
typedef TXBRA_uLong TXBRA_FAR TXBRA_uLongf;

typedef void TXBRA_FAR *TXBRA_voidpf;
typedef void     *TXBRA_voidp;

#endif /* TXBRA__ZCONF_H */
