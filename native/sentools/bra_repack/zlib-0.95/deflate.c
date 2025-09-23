// This is a modified zlib v0.95 that will produce bit-exact output matching
// whatever tool was originally used to compress the *.bra files in the PC
// version of Tokyo Xanadu eX+. Do not use it for any other purpose, you're
// better off just using standard zlib instead.

/* deflate.c -- compress data using the deflation algorithm
 * Copyright (C) 1995 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

/*
 *  ALGORITHM
 *
 *      The "deflation" process depends on being able to identify portions
 *      of the input text which are identical to earlier input (within a
 *      sliding window trailing behind the input currently being processed).
 *
 *      The most straightforward technique turns out to be the fastest for
 *      most input files: try all possible matches and select the longest.
 *      The key feature of this algorithm is that insertions into the string
 *      dictionary are very simple and thus fast, and deletions are avoided
 *      completely. Insertions are performed at each input character, whereas
 *      string matches are performed only when the previous match ends. So it
 *      is preferable to spend more time in matches to allow very fast string
 *      insertions and avoid deletions. The matching algorithm for small
 *      strings is inspired from that of Rabin & Karp. A brute force approach
 *      is used to find longer strings when a small match has been found.
 *      A similar algorithm is used in comic (by Jan-Mark Wams) and freeze
 *      (by Leonid Broukhis).
 *         A previous version of this file used a more sophisticated algorithm
 *      (by Fiala and Greene) which is guaranteed to run in linear amortized
 *      time, but has a larger average cost, uses more memory and is patented.
 *      However the F&G algorithm may be faster for some highly redundant
 *      files if the parameter max_chain_length (described below) is too large.
 *
 *  ACKNOWLEDGEMENTS
 *
 *      The idea of lazy evaluation of matches is due to Jan-Mark Wams, and
 *      I found it in 'freeze' written by Leonid Broukhis.
 *      Thanks to many people for bug reports and testing.
 *
 *  REFERENCES
 *
 *      Deutsch, L.P.,"'Deflate' Compressed Data Format Specification".
 *      Available in ftp.uu.net:/pub/archiving/zip/doc/deflate-1.1.doc
 *
 *      A description of the Rabin and Karp algorithm is given in the book
 *         "Algorithms" by R. Sedgewick, Addison-Wesley, p252.
 *
 *      Fiala,E.R., and Greene,D.H.
 *         Data Compression with Finite Windows, Comm.ACM, 32,4 (1989) 490-595
 *
 */

/* $Id: deflate.c,v 1.8 1995/05/03 17:27:08 jloup Exp $ */

#include "deflate.h"

char copyright[] = " deflate Copyright 1995 Jean-loup Gailly ";
/*
  If you use the zlib library in a product, an acknowledgment is welcome
  in the documentation of your product. If for some reason you cannot
  include such an acknowledgment, I would appreciate that you keep this
  copyright string in the executable of your product.
 */

#define NIL 0
/* Tail of hash chains */

#ifndef TOO_FAR
#  define TOO_FAR 4096
#endif
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

#define MIN_LOOKAHEAD (TXBRA_MAX_MATCH+TXBRA_MIN_MATCH+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the TXBRA_MIN_MATCH+1.
 */

/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */

typedef struct TXBRA_config_s {
   TXBRA_ush good_length; /* reduce lazy search above this match length */
   TXBRA_ush max_lazy;    /* do not perform lazy search above this match length */
   TXBRA_ush nice_length; /* quit search above this match length */
   TXBRA_ush max_chain;
} TXBRA_config;

TXBRA_local TXBRA_config configuration_table[10] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0},  /* store only */
/* 1 */ {4,    4,  8,    4},  /* maximum speed, no lazy matches */
/* 2 */ {4,    5, 16,    8},
/* 3 */ {4,    6, 32,   32},

/* 4 */ {4,    4, 16,   16},  /* lazy matches */
/* 5 */ {8,   16, 32,   32},
/* 6 */ {8,   16, 128, 128},
/* 7 */ {8,   32, 128, 256},
/* 8 */ {32, 128, 258, 1024},
/* 9 */ {32, 258, 258, 4096}}; /* maximum compression */

/* Note: the deflate() code requires max_lazy >= TXBRA_MIN_MATCH and max_chain >= 4
 * For deflate_fast() (levels <= 3) good is ignored and lazy has a different
 * meaning.
 */

#define EQUAL 0
/* result of memcmp for equal strings */

struct TXBRA_static_tree_desc_s {int dummy;}; /* for buggy compilers */

/* ===========================================================================
 *  Prototypes for local functions.
 */

TXBRA_local void fill_window   OF((TXBRA_deflate_state *s));
TXBRA_local int  deflate_fast  OF((TXBRA_deflate_state *s, int flush));
TXBRA_local int  deflate_slow  OF((TXBRA_deflate_state *s, int flush));
TXBRA_local void lm_init       OF((TXBRA_deflate_state *s));
TXBRA_local int longest_match  OF((TXBRA_deflate_state *s, IPos cur_match));
TXBRA_local void putShortMSB   OF((TXBRA_deflate_state *s, TXBRA_uInt b));
TXBRA_local void flush_pending OF((TXBRA_z_stream *strm));
TXBRA_local int read_buf       OF((TXBRA_z_stream *strm, TXBRA_charf *buf, unsigned size));
#ifdef ASMV
      void match_init OF((void)); /* asm code initialization */
#endif

#ifdef DEBUG
TXBRA_local  void check_match OF((TXBRA_deflate_state *s, IPos start, IPos match,
                            int length));
#endif


/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to to UPDATE_HASH are made with consecutive
 *    input characters, so that a running hash key can be computed from the
 *    previous key instead of complete recalculation each time.
 */
#define UPDATE_HASH(s,h,c) (h = (((h)<<s->hash_shift) ^ (c)) & s->hash_mask)


/* ===========================================================================
 * Insert string str in the dictionary and set match_head to the previous head
 * of the hash chain (the most recent string with same hash key). Return
 * the previous length of the hash chain.
 * IN  assertion: all calls to to INSERT_STRING are made with consecutive
 *    input characters and the first TXBRA_MIN_MATCH bytes of str are valid
 *    (except for the last TXBRA_MIN_MATCH-1 bytes of the input file).
 */
#define INSERT_STRING(s, str, match_head) \
   (UPDATE_HASH(s, s->ins_h, s->window[(str) + (TXBRA_MIN_MATCH-1)]), \
    s->prev[(str) & s->w_mask] = match_head = s->head[s->ins_h], \
    s->head[s->ins_h] = (str))

/* ===========================================================================
 * Initialize the hash table (avoiding 64K overflow for 16 bit systems).
 * prev[] will be initialized on the fly.
 */
#define CLEAR_HASH(s) \
    s->head[s->hash_size-1] = NIL; \
    TXBRA_zmemzero((TXBRA_charf *)s->head, (unsigned)(s->hash_size-1)*sizeof(*s->head));

/* ========================================================================= */
int TXBRA_deflateInit2 (strm, level, method, windowBits, memLevel, strategy)
    TXBRA_z_stream *strm;
    int  level;
    int  method;
    int  windowBits;
    int  memLevel;
    int  strategy;
{
    TXBRA_deflate_state *s;
    int noheader = 0;

    if (strm == TXBRA_Z_NULL) return TXBRA_Z_STREAM_ERROR;

    strm->msg = TXBRA_Z_NULL;
    if (strm->zalloc == TXBRA_Z_NULL) strm->zalloc = TXBRA_zcalloc;
    if (strm->zfree == TXBRA_Z_NULL) strm->zfree = TXBRA_zcfree;

    if (level == TXBRA_Z_DEFAULT_COMPRESSION) level = 6;

    if (windowBits < 0) { /* undocumented feature: suppress zlib header */
        noheader = 1;
        windowBits = -windowBits;
    }
    if (memLevel < 1 || memLevel > TXBRA_MAX_MEM_LEVEL || method != TXBRA_DEFLATED ||
        windowBits < 8 || windowBits > 16 || level < 1 || level > 9) {
        return TXBRA_Z_STREAM_ERROR;
    }
    s = (TXBRA_deflate_state *) TXBRA_ZALLOC(strm, 1, sizeof(TXBRA_deflate_state));
    if (s == TXBRA_Z_NULL) return TXBRA_Z_MEM_ERROR;
    strm->state = (struct TXBRA_internal_state TXBRA_FAR *)s;
    s->strm = strm;

    s->noheader = noheader;
    s->w_bits = windowBits;
    s->w_size = 0x8000;
    s->w_mask = s->w_size - 1;

    s->hash_bits = memLevel + 6;
    s->hash_size = 1 << s->hash_bits;
    s->hash_mask = s->hash_size - 1;
    s->hash_shift =  ((s->hash_bits+TXBRA_MIN_MATCH-1)/TXBRA_MIN_MATCH);

    s->window = (TXBRA_Bytef *) TXBRA_ZALLOC(strm, s->w_size, 2*sizeof(TXBRA_Byte));
    s->prev   = (Posf *)  TXBRA_ZALLOC(strm, s->w_size, sizeof(Pos));
    s->head   = (Posf *)  TXBRA_ZALLOC(strm, s->hash_size, sizeof(Pos));

    s->lit_bufsize = 1 << (memLevel + 6); /* 16K elements by default */

    s->pending_buf = (TXBRA_uchf *) TXBRA_ZALLOC(strm, s->lit_bufsize, 2*sizeof(TXBRA_ush));

    if (s->window == TXBRA_Z_NULL || s->prev == TXBRA_Z_NULL || s->head == TXBRA_Z_NULL ||
        s->pending_buf == TXBRA_Z_NULL) {
        strm->msg = TXBRA_z_errmsg[1-TXBRA_Z_MEM_ERROR];
        TXBRA_deflateEnd (strm);
        return TXBRA_Z_MEM_ERROR;
    }
    s->d_buf = (TXBRA_ushf *) &(s->pending_buf[s->lit_bufsize]);
    s->l_buf = (TXBRA_uchf *) &(s->pending_buf[3*s->lit_bufsize]);
    /* We overlay pending_buf and d_buf+l_buf. This works since the average
     * output size for (length,distance) codes is <= 32 bits (worst case
     * is 15+15+13=33).
     */

    s->level = level;
    s->strategy = strategy;
    s->method = (TXBRA_Byte)method;

    TXBRA_zmemzero(s->window, s->w_size * 2 * sizeof(TXBRA_Byte));
    TXBRA_zmemzero(s->prev, s->w_size * sizeof(Pos));
    TXBRA_zmemzero(s->head, s->hash_size * sizeof(Pos));
    TXBRA_zmemzero(s->pending_buf, s->lit_bufsize * 2 * sizeof(TXBRA_ush));

    return TXBRA_deflateReset(strm);
}

/* ========================================================================= */
int TXBRA_deflateReset (strm)
    TXBRA_z_stream *strm;
{
    TXBRA_deflate_state *s;
    
    if (strm == TXBRA_Z_NULL || strm->state == TXBRA_Z_NULL ||
        strm->zalloc == TXBRA_Z_NULL || strm->zfree == TXBRA_Z_NULL) return TXBRA_Z_STREAM_ERROR;

    strm->total_in = strm->total_out = 0;
    strm->msg = TXBRA_Z_NULL; /* use zfree if we ever allocate msg dynamically */
    strm->data_type = TXBRA_Z_UNKNOWN;

    s = (TXBRA_deflate_state *)strm->state;
    s->pending = 0;
    s->pending_out = s->pending_buf;

    if (s->noheader < 0) {
        s->noheader = 0; /* was set to -1 by deflate(..., TXBRA_Z_FINISH); */
    }
    s->status = s->noheader ? BUSY_STATE : INIT_STATE;
    s->adler = 1;

    TXBRA_ct_init(s);
    lm_init(s);

    return TXBRA_Z_OK;
}

/* =========================================================================
 * Put a short in the pending buffer. The 16-bit value is put in MSB order.
 * IN assertion: the stream state is correct and there is enough room in
 * pending_buf.
 */
TXBRA_local void putShortMSB (s, b)
    TXBRA_deflate_state *s;
    TXBRA_uInt b;
{
    put_byte(s, (TXBRA_Byte)(b >> 8));
    put_byte(s, (TXBRA_Byte)(b & 0xff));
}   

/* =========================================================================
 * Flush as much pending output as possible.
 */
TXBRA_local void flush_pending(strm)
    TXBRA_z_stream *strm;
{
    unsigned len = strm->state->pending;

    if (len > strm->avail_out) len = strm->avail_out;
    if (len == 0) return;

    TXBRA_zmemcpy(strm->next_out, strm->state->pending_out, len);
    strm->next_out  += len;
    strm->state->pending_out  += len;
    strm->total_out += len;
    strm->avail_out  -= len;
    strm->state->pending -= len;
    if (strm->state->pending == 0) {
        strm->state->pending_out = strm->state->pending_buf;
    }
}

/* ========================================================================= */
int TXBRA_deflate (strm, flush)
    TXBRA_z_stream *strm;
    int flush;
{
    if (strm == TXBRA_Z_NULL || strm->state == TXBRA_Z_NULL) return TXBRA_Z_STREAM_ERROR;
    
    if (strm->next_out == TXBRA_Z_NULL ||
        (strm->next_in == TXBRA_Z_NULL && strm->avail_in != 0)) {
        TXBRA_ERR_RETURN(strm, TXBRA_Z_STREAM_ERROR);
    }
    if (strm->avail_out == 0) TXBRA_ERR_RETURN(strm, TXBRA_Z_BUF_ERROR);

    strm->state->strm = strm; /* just in case */

    /* Write the zlib header */
    if (strm->state->status == INIT_STATE) {

        TXBRA_uInt header = (TXBRA_DEFLATED + ((strm->state->w_bits-8)<<4)) << 8;
        TXBRA_uInt level_flags = (strm->state->level-1) >> 1;

        if (level_flags > 3) level_flags = 3;
        header |= (level_flags << 6);
        header += 31 - (header % 31);

        strm->state->status = BUSY_STATE;
        putShortMSB(strm->state, header);
    }

    /* Flush as much pending output as possible */
    if (strm->state->pending != 0) {
        flush_pending(strm);
        if (strm->avail_out == 0) return TXBRA_Z_OK;
    }

    /* User must not provide more input after the first FINISH: */
    if (strm->state->status == FINISH_STATE && strm->avail_in != 0) {
        TXBRA_ERR_RETURN(strm, TXBRA_Z_BUF_ERROR);
    }

    /* Start a new block or continue the current one.
     */
    if (strm->avail_in != 0 || strm->state->lookahead != 0 ||
        (flush != TXBRA_Z_NO_FLUSH && strm->state->status != FINISH_STATE)) {
        int quit;

        if (flush == TXBRA_Z_FINISH) {
            strm->state->status = FINISH_STATE;
        }
        if (strm->state->level <= 3) {
            quit = deflate_fast(strm->state, flush);
        } else {
            quit = deflate_slow(strm->state, flush);
        }
        if (quit || strm->avail_out == 0) return TXBRA_Z_OK;
        /* If flush != TXBRA_Z_NO_FLUSH && avail_out == 0, the next call
         * of deflate should use the same flush parameter to make sure
         * that the flush is complete. So we don't have to output an
         * empty block here, this will be done at next call. This also
         * ensures that for a very small output buffer, we emit at most
         * one empty block.
         */
        if (flush != TXBRA_Z_OK && flush != TXBRA_Z_FINISH) {
            if (flush == TXBRA_Z_PARTIAL_FLUSH) {
                TXBRA_ct_align(strm->state);
            } else { /* FULL_FLUSH or SYNC_FLUSH */
                TXBRA_ct_stored_block(strm->state, (char*)0, 0L, 0);
                /* For a full flush, this empty block will be recognized
                 * as a special marker by inflate_sync().
                 */
                if (flush == TXBRA_Z_FULL_FLUSH) {
                    CLEAR_HASH(strm->state);             /* forget history */
                }
            }
            flush_pending(strm);
            if (strm->avail_out == 0) return TXBRA_Z_OK;
        }
    }
    TXBRA_Assert(strm->avail_out > 0, "bug2");

    if (flush != TXBRA_Z_FINISH) return TXBRA_Z_OK;
    if (strm->state->noheader) return TXBRA_Z_STREAM_END;

    /* Write the zlib trailer (adler32) */
    putShortMSB(strm->state, (TXBRA_uInt)(strm->state->adler >> 16));
    putShortMSB(strm->state, (TXBRA_uInt)(strm->state->adler & 0xffff));
    flush_pending(strm);
    /* If avail_out is zero, the application will call deflate again
     * to flush the rest.
     */
    strm->state->noheader = -1; /* write the trailer only once! */
    return strm->state->pending != 0 ? TXBRA_Z_OK : TXBRA_Z_STREAM_END;
}

/* ========================================================================= */
int TXBRA_deflateEnd (strm)
    TXBRA_z_stream *strm;
{
    if (strm == TXBRA_Z_NULL || strm->state == TXBRA_Z_NULL) return TXBRA_Z_STREAM_ERROR;

    TXBRA_TRY_FREE(strm, strm->state->window);
    TXBRA_TRY_FREE(strm, strm->state->prev);
    TXBRA_TRY_FREE(strm, strm->state->head);
    TXBRA_TRY_FREE(strm, strm->state->pending_buf);

    TXBRA_ZFREE(strm, strm->state);
    strm->state = TXBRA_Z_NULL;

    return TXBRA_Z_OK;
}

/* ===========================================================================
 * Read a new buffer from the current input stream, update the adler32
 * and total number of bytes read.
 */
TXBRA_local int read_buf(strm, buf, size)
    TXBRA_z_stream *strm;
    TXBRA_charf *buf;
    unsigned size;
{
    unsigned len = strm->avail_in;

    if (len > size) len = size;
    if (len == 0) return 0;

    strm->avail_in  -= len;

    if (!strm->state->noheader) {
        strm->state->adler = TXBRA_adler32(strm->state->adler, strm->next_in, len);
    }
    TXBRA_zmemcpy(buf, strm->next_in, len);
    strm->next_in  += len;
    strm->total_in += len;

    return (int)len;
}

/* ===========================================================================
 * Initialize the "longest match" routines for a new zlib stream
 */
TXBRA_local void lm_init (s)
    TXBRA_deflate_state *s;
{
    s->window_size = (TXBRA_ulg)2L*s->w_size;

    CLEAR_HASH(s);

    /* Set the default configuration parameters:
     */
    s->max_lazy_match   = configuration_table[s->level].max_lazy;
    s->good_match       = configuration_table[s->level].good_length;
    s->nice_match       = configuration_table[s->level].nice_length;
    s->max_chain_length = configuration_table[s->level].max_chain;

    s->strstart = 0;
    s->block_start = 0L;
    s->lookahead = 0;
    s->match_length = TXBRA_MIN_MATCH-1;
    s->match_available = 0;
    s->ins_h = 0;
#ifdef ASMV
    match_init(); /* initialize the asm code */
#endif
}

/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 */
#ifndef ASMV
/* For 80x86 and 680x0, an optimized version will be provided in match.asm or
 * match.S. The code will be functionally equivalent.
 */
TXBRA_local int longest_match(s, cur_match)
    TXBRA_deflate_state *s;
    IPos cur_match;                             /* current match */
{
    unsigned chain_length = s->max_chain_length;/* max hash chain length */
    register TXBRA_Bytef *scan = s->window + s->strstart; /* current string */
    register TXBRA_Bytef *match;                       /* matched string */
    register int len;                           /* length of current match */
    int best_len = s->prev_length;              /* best match length so far */
    IPos limit = s->strstart > (IPos)MAX_DIST(s) ?
        s->strstart - (IPos)MAX_DIST(s) : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */
    Posf *prev = s->prev;
    TXBRA_uInt wmask = s->w_mask;

#ifdef UNALIGNED_OK
    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.
     */
    register TXBRA_Bytef *strend = s->window + s->strstart + TXBRA_MAX_MATCH - 1;
    register TXBRA_ush scan_start = *(TXBRA_ushf*)scan;
    register TXBRA_ush scan_end   = *(TXBRA_ushf*)(scan+best_len-1);
#else
    register TXBRA_Bytef *strend = s->window + s->strstart + TXBRA_MAX_MATCH;
    register TXBRA_Byte scan_end1  = scan[best_len-1];
    register TXBRA_Byte scan_end   = scan[best_len];
#endif

    /* The code is optimized for HASH_BITS >= 8 and TXBRA_MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    TXBRA_Assert(s->hash_bits >= 8 && TXBRA_MAX_MATCH == 258, "Code too clever");

    /* Do not waste too much time if we already have a good match: */
    if (s->prev_length >= s->good_match) {
        chain_length >>= 2;
    }
    TXBRA_Assert((TXBRA_ulg)s->strstart <= s->window_size-MIN_LOOKAHEAD, "need lookahead");

    do {
        TXBRA_Assert(cur_match < s->strstart, "no future");
        match = s->window + cur_match;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2:
         */
#if (defined(UNALIGNED_OK) && TXBRA_MAX_MATCH == 258)
        /* This code assumes sizeof(unsigned short) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */
        if (*(TXBRA_ushf*)(match+best_len-1) != scan_end ||
            *(TXBRA_ushf*)match != scan_start) continue;

        /* It is not necessary to compare scan[2] and match[2] since they are
         * always equal when the other bytes match, given that the hash keys
         * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
         * strstart+3, +5, ... up to strstart+257. We check for insufficient
         * lookahead only every 4th comparison; the 128th check will be made
         * at strstart+257. If TXBRA_MAX_MATCH-2 is not a multiple of 8, it is
         * necessary to put more guard bytes at the end of the window, or
         * to check more often for insufficient lookahead.
         */
        TXBRA_Assert(scan[2] == match[2], "scan[2]?");
        scan++, match++;
        do {
        } while (*(TXBRA_ushf*)(scan+=2) == *(TXBRA_ushf*)(match+=2) &&
                 *(TXBRA_ushf*)(scan+=2) == *(TXBRA_ushf*)(match+=2) &&
                 *(TXBRA_ushf*)(scan+=2) == *(TXBRA_ushf*)(match+=2) &&
                 *(TXBRA_ushf*)(scan+=2) == *(TXBRA_ushf*)(match+=2) &&
                 scan < strend);
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window+strstart+257 */
        TXBRA_Assert(scan <= s->window+(unsigned)(s->window_size-1), "wild scan");
        if (*scan == *match) scan++;

        len = (TXBRA_MAX_MATCH - 1) - (int)(strend-scan);
        scan = strend - (TXBRA_MAX_MATCH-1);

#else /* UNALIGNED_OK */

        if (match[best_len]   != scan_end  ||
            match[best_len-1] != scan_end1 ||
            *match            != *scan     ||
            *++match          != scan[1])      continue;

        /* The check at best_len-1 can be removed because it will be made
         * again later. (This heuristic is not always a win.)
         * It is not necessary to compare scan[2] and match[2] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that HASH_BITS >= 8.
         */
        scan += 2, match++;
        TXBRA_Assert(*scan == *match, "match[2]?");

        /* We check for insufficient lookahead only every 8th comparison;
         * the 256th check will be made at strstart+258.
         */
        do {
        } while (*++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 scan < strend);

        TXBRA_Assert(scan <= s->window+(unsigned)(s->window_size-1), "wild scan");

        len = TXBRA_MAX_MATCH - (int)(strend - scan);
        scan = strend - TXBRA_MAX_MATCH;

#endif /* UNALIGNED_OK */

        if (len > best_len) {
            s->match_start = cur_match;
            best_len = len;
            if (len >= s->nice_match) break;
#ifdef UNALIGNED_OK
            scan_end = *(TXBRA_ushf*)(scan+best_len-1);
#else
            scan_end1  = scan[best_len-1];
            scan_end   = scan[best_len];
#endif
        }
    } while ((cur_match = prev[cur_match & wmask]) > limit
             && --chain_length != 0);

    return best_len;
}
#endif /* ASMV */

#ifdef DEBUG
/* ===========================================================================
 * Check that the match at match_start is indeed a match.
 */
TXBRA_local void check_match(s, start, match, length)
    TXBRA_deflate_state *s;
    IPos start, match;
    int length;
{
    /* check that the match is indeed a match */
    if (memcmp((TXBRA_charf *)s->window + match,
                (TXBRA_charf *)s->window + start, length) != EQUAL) {
        fprintf(stderr,
            " start %u, match %u, length %d\n",
            start, match, length);
        do { fprintf(stderr, "%c%c", s->window[match++],
                     s->window[start++]); } while (--length != 0);
        TXBRA_z_error("invalid match");
    }
    if (TXBRA_verbose > 1) {
        fprintf(stderr,"\\[%d,%d]", start-match, length);
        do { putc(s->window[start++], stderr); } while (--length != 0);
    }
}
#else
#  define check_match(s, start, match, length)
#endif

/* ===========================================================================
 * Fill the window when the lookahead becomes insufficient.
 * Updates strstart and lookahead.
 *
 * IN assertion: lookahead < MIN_LOOKAHEAD
 * OUT assertions: strstart <= window_size-MIN_LOOKAHEAD
 *    At least one byte has been read, or avail_in == 0; reads are
 *    performed for at least two bytes (required for the zip translate_eol
 *    option -- not supported here).
 */
TXBRA_local void fill_window(s)
    TXBRA_deflate_state *s;
{
    register unsigned n, m;
    register Posf *p;
    unsigned more;    /* Amount of free space at the end of the window. */
    TXBRA_uInt wsize = s->w_size;

    do {
        more = (unsigned)(s->window_size -(TXBRA_ulg)s->lookahead -(TXBRA_ulg)s->strstart);

        /* Deal with !@#$% 64K limit: */
        if (more == 0 && s->strstart == 0 && s->lookahead == 0) {
            more = wsize;
        } else if (more == (unsigned)(-1)) {
            /* Very unlikely, but possible on 16 bit machine if strstart == 0
             * and lookahead == 1 (input done one byte at time)
             */
            more--;

        /* If the window is almost full and there is insufficient lookahead,
         * move the upper half to the lower one to make room in the upper half.
         */
        } else if (s->strstart >= wsize+MAX_DIST(s)) {

            /* By the IN assertion, the window is not empty so we can't confuse
             * more == 0 with more == 64K on a 16 bit machine.
             */
            TXBRA_zmemcpy((TXBRA_charf *)s->window, (TXBRA_charf *)s->window+wsize,
                   (unsigned)wsize);
            s->match_start -= wsize;
            s->strstart    -= wsize; /* we now have strstart >= MAX_DIST */

            s->block_start -= (long) wsize;

            /* Slide the hash table (could be avoided with 32 bit values
               at the expense of memory usage):
             */
            n = s->hash_size;
            p = &s->head[n];
            do {
                m = *--p;
                *p = (Pos)(m >= wsize ? m-wsize : NIL);
            } while (--n);

            n = wsize;
            p = &s->prev[n];
            do {
                m = *--p;
                *p = (Pos)(m >= wsize ? m-wsize : NIL);
                /* If n is not on any hash chain, prev[n] is garbage but
                 * its value will never be used.
                 */
            } while (--n);

            more += wsize;
        }
        if (s->strm->avail_in == 0) return;

        /* If there was no sliding:
         *    strstart <= WSIZE+MAX_DIST-1 && lookahead <= MIN_LOOKAHEAD - 1 &&
         *    more == window_size - lookahead - strstart
         * => more >= window_size - (MIN_LOOKAHEAD-1 + WSIZE + MAX_DIST-1)
         * => more >= window_size - 2*WSIZE + 2
         * In the BIG_MEM or MMAP case (not yet supported),
         *   window_size == input_size + MIN_LOOKAHEAD  &&
         *   strstart + s->lookahead <= input_size => more >= MIN_LOOKAHEAD.
         * Otherwise, window_size == 2*WSIZE so more >= 2.
         * If there was sliding, more >= WSIZE. So in all cases, more >= 2.
         */
        TXBRA_Assert(more >= 2, "more < 2");

        n = read_buf(s->strm, (TXBRA_charf *)s->window + s->strstart + s->lookahead,
                     more);
        s->lookahead += n;

        /* Initialize the hash value now that we have some input: */
        if (s->lookahead >= TXBRA_MIN_MATCH) {
            s->ins_h = s->window[s->strstart];
            UPDATE_HASH(s, s->ins_h, s->window[s->strstart+1]);
#if TXBRA_MIN_MATCH != 3
            Call UPDATE_HASH() TXBRA_MIN_MATCH-3 more times
#endif
        }
        /* If the whole input has less than TXBRA_MIN_MATCH bytes, ins_h is garbage,
         * but this is not important since only literal bytes will be emitted.
         */

    } while (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_in != 0);
}

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 */
#define FLUSH_BLOCK_ONLY(s, eof) { \
   TXBRA_ct_flush_block(s, (s->block_start >= 0L ? \
               (TXBRA_charf *)&s->window[(unsigned)s->block_start] : \
               (TXBRA_charf *)TXBRA_Z_NULL), (long)s->strstart - s->block_start, (eof)); \
   s->block_start = s->strstart; \
   flush_pending(s->strm); \
   TXBRA_Tracev((stderr,"[FLUSH]")); \
}

/* Same but force premature exit if necessary. */
#define FLUSH_BLOCK(s, eof) { \
   FLUSH_BLOCK_ONLY(s, eof); \
   if (s->strm->avail_out == 0) return 1; \
}

/* ===========================================================================
 * Compress as much as possible from the input stream, return true if
 * processing was terminated prematurely (no more input or output space).
 * This function does not perform lazy evaluationof matches and inserts
 * new strings in the dictionary only for unmatched strings or for short
 * matches. It is used only for the fast compression options.
 */
TXBRA_local int deflate_fast(s, flush)
    TXBRA_deflate_state *s;
    int flush;
{
    IPos hash_head; /* head of the hash chain */
    int bflush;     /* set if current block must be flushed */

    s->prev_length = TXBRA_MIN_MATCH-1;

    for (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need TXBRA_MAX_MATCH bytes
         * for the next match, plus TXBRA_MIN_MATCH bytes to insert the
         * string following the next match.
         */
        if (s->lookahead < MIN_LOOKAHEAD) {
            fill_window(s);
            if (s->lookahead < MIN_LOOKAHEAD && flush == TXBRA_Z_NO_FLUSH) return 1;

            if (s->lookahead == 0) break; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart+2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        if (s->lookahead >= TXBRA_MIN_MATCH) {
            INSERT_STRING(s, s->strstart, hash_head);
        }

        /* Find the longest match, discarding those <= prev_length.
         * At this point we have always match_length < TXBRA_MIN_MATCH
         */
        if (hash_head != NIL && s->strstart - hash_head <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            if (s->strategy != TXBRA_Z_HUFFMAN_ONLY) {
                s->match_length = longest_match (s, hash_head);
            }
            /* longest_match() sets match_start */

            if (s->match_length > s->lookahead) s->match_length = s->lookahead;
        }
        if (s->match_length >= TXBRA_MIN_MATCH) {
            check_match(s, s->strstart, s->match_start, s->match_length);

            bflush = TXBRA_ct_tally(s, s->strstart - s->match_start,
                              s->match_length - TXBRA_MIN_MATCH);

            s->lookahead -= s->match_length;

            /* Insert new strings in the hash table only if the match length
             * is not too large. This saves time but degrades compression.
             */
            if (s->match_length <= s->max_insert_length &&
                s->lookahead >= TXBRA_MIN_MATCH) {
                s->match_length--; /* string at strstart already in hash table */
                do {
                    s->strstart++;
                    INSERT_STRING(s, s->strstart, hash_head);
                    /* strstart never exceeds WSIZE-TXBRA_MAX_MATCH, so there are
                     * always TXBRA_MIN_MATCH bytes ahead.
                     */
                } while (--s->match_length != 0);
                s->strstart++; 
            } else {
                s->strstart += s->match_length;
                s->match_length = 0;
                s->ins_h = s->window[s->strstart];
                UPDATE_HASH(s, s->ins_h, s->window[s->strstart+1]);
#if TXBRA_MIN_MATCH != 3
                Call UPDATE_HASH() TXBRA_MIN_MATCH-3 more times
#endif
                /* If lookahead < TXBRA_MIN_MATCH, ins_h is garbage, but it does not
                 * matter since it will be recomputed at next deflate call.
                 */
            }
        } else {
            /* No match, output a literal byte */
            TXBRA_Tracevv((stderr,"%c", s->window[s->strstart]));
            bflush = TXBRA_ct_tally (s, 0, s->window[s->strstart]);
            s->lookahead--;
            s->strstart++; 
        }
        if (bflush) FLUSH_BLOCK(s, 0);
    }
    FLUSH_BLOCK(s, flush == TXBRA_Z_FINISH);
    return 0; /* normal exit */
}

/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
TXBRA_local int deflate_slow(s, flush)
    TXBRA_deflate_state *s;
    int flush;
{
    IPos hash_head;          /* head of hash chain */
    int bflush;              /* set if current block must be flushed */

    /* Process the input block. */
    for (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need TXBRA_MAX_MATCH bytes
         * for the next match, plus TXBRA_MIN_MATCH bytes to insert the
         * string following the next match.
         */
        if (s->lookahead < MIN_LOOKAHEAD) {
            fill_window(s);
            if (s->lookahead < MIN_LOOKAHEAD && flush == TXBRA_Z_NO_FLUSH) return 1;

            if (s->lookahead == 0) break; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart+2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        if (s->lookahead >= TXBRA_MIN_MATCH) {
            INSERT_STRING(s, s->strstart, hash_head);
        }

        /* Find the longest match, discarding those <= prev_length.
         */
        s->prev_length = s->match_length, s->prev_match = s->match_start;
        s->match_length = TXBRA_MIN_MATCH-1;

        if (hash_head != NIL && s->prev_length < s->max_lazy_match &&
            s->strstart - hash_head <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            if (s->strategy != TXBRA_Z_HUFFMAN_ONLY) {
                s->match_length = longest_match (s, hash_head);
            }
            /* longest_match() sets match_start */
            if (s->match_length > s->lookahead) s->match_length = s->lookahead;

            if (s->match_length <= 5 && (s->strategy == TXBRA_Z_FILTERED ||
                 (s->match_length == TXBRA_MIN_MATCH &&
                  s->strstart - s->match_start > TOO_FAR))) {

                /* If prev_match is also TXBRA_MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                s->match_length = TXBRA_MIN_MATCH-1;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        if (s->prev_length >= TXBRA_MIN_MATCH && s->match_length <= s->prev_length) {
            TXBRA_uInt max_insert = s->strstart + s->lookahead - TXBRA_MIN_MATCH;
            /* Do not insert strings in hash table beyond this. */

            check_match(s, s->strstart-1, s->prev_match, s->prev_length);

            bflush = TXBRA_ct_tally(s, s->strstart -1 - s->prev_match,
                              s->prev_length - TXBRA_MIN_MATCH);

            /* Insert in hash table all strings up to the end of the match.
             * strstart-1 and strstart are already inserted. If there is not
             * enough lookahead, the last two strings are not inserted in
             * the hash table.
             */
            s->lookahead -= s->prev_length-1;
            s->prev_length -= 2;
            do {
                if (++s->strstart <= max_insert) {
                    INSERT_STRING(s, s->strstart, hash_head);
                }
            } while (--s->prev_length != 0);
            s->match_available = 0;
            s->match_length = TXBRA_MIN_MATCH-1;
            s->strstart++;

            if (bflush) FLUSH_BLOCK(s, 0);

        } else if (s->match_available) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            TXBRA_Tracevv((stderr,"%c", s->window[s->strstart-1]));
            if (TXBRA_ct_tally (s, 0, s->window[s->strstart-1])) {
                FLUSH_BLOCK_ONLY(s, 0);
            }
            s->strstart++;
            s->lookahead--;
            if (s->strm->avail_out == 0) return 1;
        } else {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            s->match_available = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    TXBRA_Assert (flush != TXBRA_Z_NO_FLUSH, "no flush?");
    if (s->match_available) {
        TXBRA_Tracevv((stderr,"%c", s->window[s->strstart-1]));
        TXBRA_ct_tally (s, 0, s->window[s->strstart-1]);
        s->match_available = 0;
    }
    FLUSH_BLOCK(s, flush == TXBRA_Z_FINISH);
    return 0;
}
