// This is a modified zlib v0.95 that will produce bit-exact output matching
// whatever tool was originally used to compress the *.bra files in the PC
// version of Tokyo Xanadu eX+. Do not use it for any other purpose, you're
// better off just using standard zlib instead.

/* zlib.h -- interface of the 'zlib' general purpose compression library
  version 0.95, Aug 16th, 1995.

  Copyright (C) 1995 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Jean-loup Gailly        Mark Adler
  gzip@prep.ai.mit.edu    madler@alumni.caltech.edu
 */

#ifndef TXBRA__ZLIB_H
#define TXBRA__ZLIB_H

#include "zconf.h"

#define TXBRA_ZLIB_VERSION "0.95"

/* 
     The 'zlib' compression library provides in-memory compression and
  decompression functions, including integrity checks of the uncompressed
  data.  This version of the library supports only one compression method
  (deflation) but other algorithms may be added later and will have the same
  stream interface.

     For compression the application must provide the output buffer and
  may optionally provide the input buffer for optimization. For decompression,
  the application must provide the input buffer and may optionally provide
  the output buffer for optimization.

     Compression can be done in a single step if the buffers are large
  enough (for example if an input file is mmap'ed), or can be done by
  repeated calls of the compression function.  In the latter case, the
  application must provide more input and/or consume the output
  (providing more output space) before each call.
*/

typedef TXBRA_voidpf (*TXBRA_alloc_func) OF((TXBRA_voidpf opaque, TXBRA_uInt items, TXBRA_uInt size));
typedef void   (*TXBRA_free_func)  OF((TXBRA_voidpf opaque, TXBRA_voidpf address));

struct TXBRA_internal_state;

typedef struct TXBRA_z_stream_s {
    TXBRA_Bytef    *next_in;  /* next input byte */
    TXBRA_uInt     avail_in;  /* number of bytes available at next_in */
    TXBRA_uLong    total_in;  /* total nb of input bytes read so far */

    TXBRA_Bytef    *next_out; /* next output byte should be put there */
    TXBRA_uInt     avail_out; /* remaining free space at next_out */
    TXBRA_uLong    total_out; /* total nb of bytes output so far */

    char     *msg;      /* last error message, NULL if no error */
    struct TXBRA_internal_state TXBRA_FAR *state; /* not visible by applications */

    TXBRA_alloc_func zalloc;  /* used to allocate the internal state */
    TXBRA_free_func  zfree;   /* used to free the internal state */
    TXBRA_voidp      opaque;  /* private data object passed to zalloc and zfree */

    TXBRA_Byte     data_type; /* best guess about the data type: ascii or binary */

} TXBRA_z_stream;

/*
   The application must update next_in and avail_in when avail_in has
   dropped to zero. It must update next_out and avail_out when avail_out
   has dropped to zero. The application must initialize zalloc, zfree and
   opaque before calling the init function. All other fields are set by the
   compression library and must not be updated by the application.

   The opaque value provided by the application will be passed as the first
   parameter for calls of zalloc and zfree. This can be useful for custom
   memory management. The compression library attaches no meaning to the
   opaque value.

   zalloc must return TXBRA_Z_NULL if there is not enough memory for the object.
   On 16-bit systems, the functions zalloc and zfree must be able to allocate
   exactly 65536 bytes, but will not be required to allocate more than this
   if the symbol MAXSEG_64K is defined (see zconf.h). WARNING: On MSDOS,
   pointers returned by zalloc for objects of exactly 65536 bytes *must*
   have their offset normalized to zero. The default allocation function
   provided by this library ensures this (see zutil.c). To reduce memory
   requirements and avoid any allocation of 64K objects, at the expense of
   compression ratio, compile the library with -DMAX_WBITS=14 (see zconf.h).

   The fields total_in and total_out can be used for statistics or
   progress reports. After compression, total_in holds the total size of
   the uncompressed data and may be saved for use in the decompressor
   (particularly if the decompressor wants to decompress everything in
   a single step).
*/

                        /* constants */

#define TXBRA_Z_NO_FLUSH      0
#define TXBRA_Z_PARTIAL_FLUSH 1
#define TXBRA_Z_FULL_FLUSH    2
#define TXBRA_Z_SYNC_FLUSH    3 /* experimental: partial_flush + byte align */
#define TXBRA_Z_FINISH        4
/* See deflate() below for the usage of these constants */

#define TXBRA_Z_OK            0
#define TXBRA_Z_STREAM_END    1
#define TXBRA_Z_ERRNO        (-1)
#define TXBRA_Z_STREAM_ERROR (-2)
#define TXBRA_Z_DATA_ERROR   (-3)
#define TXBRA_Z_MEM_ERROR    (-4)
#define TXBRA_Z_BUF_ERROR    (-5)
/* error codes for the compression/decompression functions */

#define TXBRA_Z_BEST_SPEED             1
#define TXBRA_Z_BEST_COMPRESSION       9
#define TXBRA_Z_DEFAULT_COMPRESSION  (-1)
/* compression levels */

#define TXBRA_Z_FILTERED            1
#define TXBRA_Z_HUFFMAN_ONLY        2
#define TXBRA_Z_DEFAULT_STRATEGY    0

#define TXBRA_Z_BINARY   0
#define TXBRA_Z_ASCII    1
#define TXBRA_Z_UNKNOWN  2
/* Used to set the data_type field */

#define TXBRA_Z_NULL  0  /* for initializing zalloc, zfree, opaque */

                        /* basic functions */

extern int TXBRA_deflate OF((TXBRA_z_stream *strm, int flush));
/*
  Performs one or both of the following actions:

  - Compress more input starting at next_in and update next_in and avail_in
    accordingly. If not all input can be processed (because there is not
    enough room in the output buffer), next_in and avail_in are updated and
    processing will resume at this point for the next call of deflate().

  - Provide more output starting at next_out and update next_out and avail_out
    accordingly. This action is forced if the parameter flush is non zero.
    Forcing flush frequently degrades the compression ratio, so this parameter
    should be set only when necessary (in interactive applications).
    Some output may be provided even if flush is not set.

  Before the call of deflate(), the application should ensure that at least
  one of the actions is possible, by providing more input and/or consuming
  more output, and updating avail_in or avail_out accordingly; avail_out
  should never be zero before the call. The application can consume the
  compressed output when it wants, for example when the output buffer is full
  (avail_out == 0), or after each call of deflate().

    If the parameter flush is set to TXBRA_Z_PARTIAL_FLUSH, the current compression
  block is terminated and flushed to the output buffer so that the
  decompressor can get all input data available so far. For method 9, a future
  variant on method 8, the current block will be flushed but not terminated.
  If flush is set to TXBRA_Z_FULL_FLUSH, the compression block is terminated, a
  special marker is output and the compression dictionary is discarded; this
  is useful to allow the decompressor to synchronize if one compressed block
  has been damaged (see inflateSync below).  Flushing degrades compression and
  so should be used only when necessary.  Using TXBRA_Z_FULL_FLUSH too often can
  seriously degrade the compression. If deflate returns with avail_out == 0,
  this function must be called again with the same value of the flush
  parameter and more output space (updated avail_out), until the flush is
  complete (deflate returns with non-zero avail_out).

    If the parameter flush is set to TXBRA_Z_FINISH, all pending input is processed,
  all pending output is flushed and deflate returns with TXBRA_Z_STREAM_END if there
  was enough output space; if deflate returns with TXBRA_Z_OK, this function must be
  called again with TXBRA_Z_FINISH and more output space (updated avail_out) but no
  more input data, until it returns with TXBRA_Z_STREAM_END or an error. After
  deflate has returned TXBRA_Z_STREAM_END, the only possible operations on the
  stream are deflateReset or deflateEnd.
  
    TXBRA_Z_FINISH can be used immediately after deflateInit if all the compression
  is to be done in a single step. In this case, avail_out must be at least
  0.1% larger than avail_in plus 12 bytes.  If deflate does not return
  TXBRA_Z_STREAM_END, then it must be called again as described above.

    deflate() may update data_type if it can make a good guess about
  the input data type (TXBRA_Z_ASCII or TXBRA_Z_BINARY). In doubt, the data is considered
  binary. This field is only for information purposes and does not affect
  the compression algorithm in any manner.

    deflate() returns TXBRA_Z_OK if some progress has been made (more input
  processed or more output produced), TXBRA_Z_STREAM_END if all input has been
  consumed and all output has been produced (only when flush is set to
  TXBRA_Z_FINISH), TXBRA_Z_STREAM_ERROR if the stream state was inconsistent (for example
  if next_in or next_out was NULL), TXBRA_Z_BUF_ERROR if no progress is possible.
*/


extern int TXBRA_deflateEnd OF((TXBRA_z_stream *strm));
/*
     All dynamically allocated data structures for this stream are freed.
   This function discards any unprocessed input and does not flush any
   pending output.

     deflateEnd returns TXBRA_Z_OK if success, TXBRA_Z_STREAM_ERROR if the
   stream state was inconsistent. In the error case, msg may be set
   but then points to a static string (which must not be deallocated).
*/


                        /* advanced functions */

/*
    The following functions are needed only in some special applications.
*/

extern int TXBRA_deflateInit2 OF((TXBRA_z_stream *strm,
                            int  level,
                            int  method,
                            int  windowBits,
                            int  memLevel,
                            int  strategy));
/*   
     This is another version of deflateInit with more compression options. The
   fields next_in, zalloc and zfree must be initialized before by the caller.

     The method parameter is the compression method. It must be 8 in this
   version of the library. (Method 9 will allow a 64K history buffer and
   partial block flushes.)

     The windowBits parameter is the base two logarithm of the window size
   (the size of the history buffer).  It should be in the range 8..15 for this
   version of the library (the value 16 will be allowed for method 9). Larger
   values of this parameter result in better compression at the expense of
   memory usage. The default value is 15 if deflateInit is used instead.

    The memLevel parameter specifies how much memory should be allocated
   for the internal compression state. memLevel=1 uses minimum memory but
   is slow and reduces compression ratio; memLevel=9 uses maximum memory
   for optimal speed. The default value is 8. See zconf.h for total memory
   usage as a function of windowBits and memLevel.

     The strategy parameter is used to tune the compression algorithm. Use
   the value TXBRA_Z_DEFAULT_STRATEGY for normal data, TXBRA_Z_FILTERED for data
   produced by a filter (or predictor), or TXBRA_Z_HUFFMAN_ONLY to force Huffman
   encoding only (no string match).  Filtered data consists mostly of small
   values with a somewhat random distribution. In this case, the
   compression algorithm is tuned to compress them better. The strategy
   parameter only affects the compression ratio but not the correctness of
   the compressed output even if it is not set appropriately.

     If next_in is not null, the library will use this buffer to hold also
   some history information; the buffer must either hold the entire input
   data, or have at least 1<<(windowBits+1) bytes and be writable. If next_in
   is null, the library will allocate its own history buffer (and leave next_in
   null). next_out need not be provided here but must be provided by the
   application for the next call of deflate().

     If the history buffer is provided by the application, next_in must
   must never be changed by the application since the compressor maintains
   information inside this buffer from call to call; the application
   must provide more input only by increasing avail_in. next_in is always
   reset by the library in this case.

      deflateInit2 returns TXBRA_Z_OK if success, TXBRA_Z_MEM_ERROR if there was
   not enough memory, TXBRA_Z_STREAM_ERROR if a parameter is invalid (such as
   an invalid method). msg is set to null if there is no error message.
   deflateInit2 does not perform any compression: this will be done by
   deflate().
*/

extern int TXBRA_deflateReset OF((TXBRA_z_stream *strm));
/*
     This function is equivalent to deflateEnd followed by deflateInit,
   but does not free and reallocate all the internal compression state.
   The stream will keep the same compression level and any other attributes
   that may have been set by deflateInit2.

      deflateReset returns TXBRA_Z_OK if success, or TXBRA_Z_STREAM_ERROR if the source
   stream state was inconsistent (such as zalloc or state being NULL).
*/


                        /* checksum functions */

/*
     These functions are not related to compression but are exported
   anyway because they might be useful in applications using the
   compression library.
*/

extern TXBRA_uLong TXBRA_adler32 OF((TXBRA_uLong adler, TXBRA_Bytef *buf, TXBRA_uInt len));

/*
     Update a running Adler-32 checksum with the bytes buf[0..len-1] and
   return the updated checksum. If buf is NULL, this function returns
   the required initial value for the checksum.
   An Adler-32 checksum is almost as reliable as a CRC32 but can be computed
   much faster. Usage example:

     TXBRA_uLong adler = adler32(0L, TXBRA_Z_NULL, 0);

     while (read_buffer(buffer, length) != EOF) {
       adler = adler32(adler, buffer, length);
     }
     if (adler != original_adler) error();
*/

#ifndef TXBRA__Z_UTIL_H
    struct TXBRA_internal_state {int dummy;}; /* hack for buggy compilers */
#endif

#endif /* TXBRA__ZLIB_H */
