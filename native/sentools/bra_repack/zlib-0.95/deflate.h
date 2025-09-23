// This is a modified zlib v0.95 that will produce bit-exact output matching
// whatever tool was originally used to compress the *.bra files in the PC
// version of Tokyo Xanadu eX+. Do not use it for any other purpose, you're
// better off just using standard zlib instead.

/* deflate.h -- internal compression state
 * Copyright (C) 1995 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* $Id: deflate.h,v 1.5 1995/05/03 17:27:09 jloup Exp $ */

#include "zutil.h"

/* ===========================================================================
 * Internal compression state.
 */

/* Data type */
#define BINARY  0
#define ASCII   1
#define UNKNOWN 2

#define LENGTH_CODES 29
/* number of length codes, not counting the special END_BLOCK code */

#define LITERALS  256
/* number of literal bytes 0..255 */

#define L_CODES (LITERALS+1+LENGTH_CODES)
/* number of Literal or Length codes, including the END_BLOCK code */

#define D_CODES   30
/* number of distance codes */

#define BL_CODES  19
/* number of codes used to transfer the bit lengths */

#define HEAP_SIZE (2*L_CODES+1)
/* maximum heap size */

#define MAX_BITS 15
/* All codes must not exceed MAX_BITS bits */

#define INIT_STATE    42
#define BUSY_STATE   113
#define FINISH_STATE 666
/* Stream status */


/* Data structure describing a single value and its code string. */
typedef struct TXBRA_ct_data_s {
    union {
        TXBRA_ush  freq;       /* frequency count */
        TXBRA_ush  code;       /* bit string */
    } fc;
    union {
        TXBRA_ush  dad;        /* father node in Huffman tree */
        TXBRA_ush  len;        /* length of bit string */
    } dl;
} TXBRA_FAR TXBRA_ct_data;

#define Freq fc.freq
#define Code fc.code
#define Dad  dl.dad
#define Len  dl.len

typedef struct TXBRA_static_tree_desc_s  TXBRA_static_tree_desc;

typedef struct TXBRA_tree_desc_s {
    TXBRA_ct_data *dyn_tree;           /* the dynamic tree */
    int     max_code;            /* largest code with non zero frequency */
    TXBRA_static_tree_desc *stat_desc; /* the corresponding static tree */
} TXBRA_FAR TXBRA_tree_desc;

typedef TXBRA_ush Pos;
typedef Pos TXBRA_FAR Posf;
typedef unsigned IPos;

/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */

typedef struct TXBRA_internal_state {
    TXBRA_z_stream *strm;      /* pointer back to this zlib stream */
    int   status;        /* as the name implies */
    TXBRA_Bytef *pending_buf;  /* output still pending */
    TXBRA_Bytef *pending_out;  /* next pending byte to output to the stream */
    int   pending;       /* nb of bytes in the pending buffer */
    TXBRA_uLong adler;         /* adler32 of uncompressed data */
    int   noheader;      /* suppress zlib header and adler32 */
    TXBRA_Byte  data_type;     /* UNKNOWN, BINARY or ASCII */
    TXBRA_Byte  method;        /* STORED (for zip only) or TXBRA_DEFLATED */

                /* used by deflate.c: */

    TXBRA_uInt  w_size;        /* LZ77 window size (32K by default) */
    TXBRA_uInt  w_bits;        /* log2(w_size)  (8..16) */
    TXBRA_uInt  w_mask;        /* w_size - 1 */

    TXBRA_Bytef *window;
    /* Sliding window. Input bytes are read into the second half of the window,
     * and move to the first half later to keep a dictionary of at least wSize
     * bytes. With this organization, matches are limited to a distance of
     * wSize-TXBRA_MAX_MATCH bytes, but this ensures that IO is always
     * performed with a length multiple of the block size. Also, it limits
     * the window size to 64K, which is quite useful on MSDOS.
     * To do: use the user input buffer as sliding window.
     */

    TXBRA_ulg window_size;
    /* Actual size of window: 2*wSize, except when the user input buffer
     * is directly used as sliding window.
     */

    Posf *prev;
    /* Link to older string with same hash index. To limit the size of this
     * array to 64K, this link is maintained only for the last 32K strings.
     * An index in this array is thus a window index modulo 32K.
     */

    Posf *head; /* Heads of the hash chains or NIL. */

    TXBRA_uInt  ins_h;          /* hash index of string to be inserted */
    TXBRA_uInt  hash_size;      /* number of elements in hash table */
    TXBRA_uInt  hash_bits;      /* log2(hash_size) */
    TXBRA_uInt  hash_mask;      /* hash_size-1 */

    TXBRA_uInt  hash_shift;
    /* Number of bits by which ins_h must be shifted at each input
     * step. It must be such that after TXBRA_MIN_MATCH steps, the oldest
     * byte no longer takes part in the hash key, that is:
     *   hash_shift * TXBRA_MIN_MATCH >= hash_bits
     */

    long block_start;
    /* Window position at the beginning of the current output block. Gets
     * negative when the window is moved backwards.
     */

    TXBRA_uInt match_length;           /* length of best match */
    IPos prev_match;             /* previous match */
    int match_available;         /* set if previous match exists */
    TXBRA_uInt strstart;               /* start of string to insert */
    TXBRA_uInt match_start;            /* start of matching string */
    TXBRA_uInt lookahead;              /* number of valid bytes ahead in window */

    TXBRA_uInt prev_length;
    /* Length of the best match at previous step. Matches not greater than this
     * are discarded. This is used in the lazy match evaluation.
     */

    TXBRA_uInt max_chain_length;
    /* To speed up deflation, hash chains are never searched beyond this
     * length.  A higher limit improves compression ratio but degrades the
     * speed.
     */

    TXBRA_uInt max_lazy_match;
    /* Attempt to find a better match only when the current match is strictly
     * smaller than this value. This mechanism is used only for compression
     * levels >= 4.
     */
#   define max_insert_length  max_lazy_match
    /* Insert new strings in the hash table only if the match length is not
     * greater than this length. This saves time but degrades compression.
     * max_insert_length is used only for compression levels <= 3.
     */

    int level;    /* compression level (1..9) */
    int strategy; /* favor or force Huffman coding*/

    TXBRA_uInt good_match;
    /* Use a faster search when the previous match is longer than this */

     int nice_match; /* Stop searching when current match exceeds this */

                /* used by trees.c: */
    /* Didn't use TXBRA_ct_data typedef below to supress compiler warning */
    struct TXBRA_ct_data_s dyn_ltree[HEAP_SIZE];   /* literal and length tree */
    struct TXBRA_ct_data_s dyn_dtree[2*D_CODES+1]; /* distance tree */
    struct TXBRA_ct_data_s bl_tree[2*BL_CODES+1];  /* Huffman tree for bit lengths */

    struct TXBRA_tree_desc_s l_desc;               /* desc. for literal tree */
    struct TXBRA_tree_desc_s d_desc;               /* desc. for distance tree */
    struct TXBRA_tree_desc_s bl_desc;              /* desc. for bit length tree */

    TXBRA_ush bl_count[MAX_BITS+1];
    /* number of codes at each bit length for an optimal tree */

    int heap[2*L_CODES+1];      /* heap used to build the Huffman trees */
    int heap_len;               /* number of elements in the heap */
    int heap_max;               /* element of largest frequency */
    /* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
     * The same heap array is used to build all trees.
     */

    TXBRA_uch depth[2*L_CODES+1];
    /* Depth of each subtree used as tie breaker for trees of equal frequency
     */

    TXBRA_uchf *l_buf;          /* buffer for literals or lengths */

    TXBRA_uInt  lit_bufsize;
    /* Size of match buffer for literals/lengths.  There are 4 reasons for
     * limiting lit_bufsize to 64K:
     *   - frequencies can be kept in 16 bit counters
     *   - if compression is not successful for the first block, all input
     *     data is still in the window so we can still emit a stored block even
     *     when input comes from standard input.  (This can also be done for
     *     all blocks if lit_bufsize is not greater than 32K.)
     *   - if compression is not successful for a file smaller than 64K, we can
     *     even emit a stored file instead of a stored block (saving 5 bytes).
     *     This is applicable only for zip (not gzip or zlib).
     *   - creating new Huffman trees less frequently may not provide fast
     *     adaptation to changes in the input data statistics. (Take for
     *     example a binary file with poorly compressible code followed by
     *     a highly compressible string table.) Smaller buffer sizes give
     *     fast adaptation but have of course the overhead of transmitting
     *     trees more frequently.
     *   - I can't count above 4
     */

    TXBRA_uInt last_lit;      /* running index in l_buf */

    TXBRA_ushf *d_buf;
    /* Buffer for distances. To simplify the code, d_buf and l_buf have
     * the same number of elements. To use different lengths, an extra flag
     * array would be necessary.
     */

    TXBRA_ulg opt_len;        /* bit length of current block with optimal trees */
    TXBRA_ulg static_len;     /* bit length of current block with static trees */
    TXBRA_ulg compressed_len; /* total bit length of compressed file */
    TXBRA_uInt matches;       /* number of string matches in current block */
    int last_eob_len;   /* bit length of EOB code for last block */

#ifdef DEBUG
    TXBRA_ulg bits_sent;      /* bit length of the compressed data */
#endif

    TXBRA_ush bi_buf;
    /* Output buffer. bits are inserted starting at the bottom (least
     * significant bits).
     */
    int bi_valid;
    /* Number of valid bits in bi_buf.  All bits above the last valid bit
     * are always zero.
     */

} TXBRA_FAR TXBRA_deflate_state;

/* Output a byte on the stream.
 * IN assertion: there is enough room in pending_buf.
 */
#define put_byte(s, c) {s->pending_buf[s->pending++] = (c);}


#define MIN_LOOKAHEAD (TXBRA_MAX_MATCH+TXBRA_MIN_MATCH+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the TXBRA_MIN_MATCH+1.
 */

#define MAX_DIST(s)  ((s)->w_size-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

        /* in trees.c */
void TXBRA_ct_init       OF((TXBRA_deflate_state *s));
int  TXBRA_ct_tally      OF((TXBRA_deflate_state *s, int dist, int lc));
TXBRA_ulg TXBRA_ct_flush_block OF((TXBRA_deflate_state *s, TXBRA_charf *buf, TXBRA_ulg stored_len, int eof));
void TXBRA_ct_align      OF((TXBRA_deflate_state *s));
void TXBRA_ct_stored_block OF((TXBRA_deflate_state *s, TXBRA_charf *buf, TXBRA_ulg stored_len,
                          int eof));
