/*
 * VC-1 and WMV3 decoder
 * copyright (c) 2006 Konstantin Shishkov
 * (c) 2005 anonymous, Alex Beregszaszi, Michael Niedermayer
 *
 * This file is part of Libav.
 *
 * Libav is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Libav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Libav; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * VC-1 tables.
 */

#include "avcodec.h"
#include "vc1.h"
#include "vc1data.h"

/** Table for conversion between TTBLK and TTMB */
const int ff_vc1_ttblk_to_tt[3][8] = {
  { TT_8X4, TT_4X8, TT_8X8, TT_4X4, TT_8X4_TOP, TT_8X4_BOTTOM, TT_4X8_RIGHT, TT_4X8_LEFT },
  { TT_8X8, TT_4X8_RIGHT, TT_4X8_LEFT, TT_4X4, TT_8X4, TT_4X8, TT_8X4_BOTTOM, TT_8X4_TOP },
  { TT_8X8, TT_4X8, TT_4X4, TT_8X4_BOTTOM, TT_4X8_RIGHT, TT_4X8_LEFT, TT_8X4, TT_8X4_TOP }
};

const int ff_vc1_ttfrm_to_tt[4] = { TT_8X8, TT_8X4, TT_4X8, TT_4X4 };

/** MV P mode - the 5th element is only used for mode 1 */
const uint8_t ff_vc1_mv_pmode_table[2][5] = {
  { MV_PMODE_1MV_HPEL_BILIN, MV_PMODE_1MV, MV_PMODE_1MV_HPEL, MV_PMODE_INTENSITY_COMP, MV_PMODE_MIXED_MV },
  { MV_PMODE_1MV, MV_PMODE_MIXED_MV, MV_PMODE_1MV_HPEL, MV_PMODE_INTENSITY_COMP, MV_PMODE_1MV_HPEL_BILIN }
};
const uint8_t ff_vc1_mv_pmode_table2[2][4] = {
  { MV_PMODE_1MV_HPEL_BILIN, MV_PMODE_1MV, MV_PMODE_1MV_HPEL, MV_PMODE_MIXED_MV },
  { MV_PMODE_1MV, MV_PMODE_MIXED_MV, MV_PMODE_1MV_HPEL, MV_PMODE_1MV_HPEL_BILIN }
};

const int ff_vc1_fps_nr[5] = { 24, 25, 30, 50, 60 },
  ff_vc1_fps_dr[2] = { 1000, 1001 };
const uint8_t ff_vc1_pquant_table[3][32] = {
  {  /* Implicit quantizer */
     0,  1,  2,  3,  4,  5,  6,  7,  8,  6,  7,  8,  9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 27, 29, 31
  },
  {  /* Explicit quantizer, pquantizer uniform */
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
  },
  {  /* Explicit quantizer, pquantizer non-uniform */
     0,  1,  1,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13,
    14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 29, 31
  }
};

/** @name VC-1 VLC tables and defines
 *  @todo TODO move this into the context
 */
//@{
#define VC1_BFRACTION_VLC_BITS 7
VLC ff_vc1_bfraction_vlc;
#define VC1_IMODE_VLC_BITS 4
VLC ff_vc1_imode_vlc;
#define VC1_NORM2_VLC_BITS 3
VLC ff_vc1_norm2_vlc;
#define VC1_NORM6_VLC_BITS 9
VLC ff_vc1_norm6_vlc;
/* Could be optimized, one table only needs 8 bits */
#define VC1_TTMB_VLC_BITS 9 //12
VLC ff_vc1_ttmb_vlc[3];
#define VC1_MV_DIFF_VLC_BITS 9 //15
VLC ff_vc1_mv_diff_vlc[4];
#define VC1_CBPCY_P_VLC_BITS 9 //14
VLC ff_vc1_cbpcy_p_vlc[4];
#define VC1_4MV_BLOCK_PATTERN_VLC_BITS 6
VLC ff_vc1_4mv_block_pattern_vlc[4];
#define VC1_TTBLK_VLC_BITS 5
VLC ff_vc1_ttblk_vlc[3];
#define VC1_SUBBLKPAT_VLC_BITS 6
VLC ff_vc1_subblkpat_vlc[3];

VLC ff_vc1_ac_coeff_table[8];
//@}


#if B_FRACTION_DEN==840 //original bfraction from vc9data.h, not conforming to standard
/* bfraction is fractional, we scale to the GCD 3*5*7*8 = 840 */
const int16_t ff_vc1_bfraction_lut[23] = {
  420 /*1/2*/, 280 /*1/3*/, 560 /*2/3*/, 210 /*1/4*/,
  630 /*3/4*/, 168 /*1/5*/, 336 /*2/5*/,
  504 /*3/5*/, 672 /*4/5*/, 140 /*1/6*/, 700 /*5/6*/,
  120 /*1/7*/, 240 /*2/7*/, 360 /*3/7*/, 480 /*4/7*/,
  600 /*5/7*/, 720 /*6/7*/, 105 /*1/8*/, 315 /*3/8*/,
  525 /*5/8*/, 735 /*7/8*/,
  -1 /*inv.*/, 0 /*BI fm*/
};
#else
/* pre-computed scales for all bfractions and base=256 */
const int16_t ff_vc1_bfraction_lut[23] = {
  128 /*1/2*/,  85 /*1/3*/, 170 /*2/3*/,  64 /*1/4*/,
  192 /*3/4*/,  51 /*1/5*/, 102 /*2/5*/,
  153 /*3/5*/, 204 /*4/5*/,  43 /*1/6*/, 215 /*5/6*/,
   37 /*1/7*/,  74 /*2/7*/, 111 /*3/7*/, 148 /*4/7*/,
  185 /*5/7*/, 222 /*6/7*/,  32 /*1/8*/,  96 /*3/8*/,
  160 /*5/8*/, 224 /*7/8*/,
  -1 /*inv.*/, 0 /*BI fm*/
};
#endif

const uint8_t ff_vc1_bfraction_bits[23] = {
    3, 3, 3, 3,
    3, 3, 3,
    7, 7, 7, 7,
    7, 7, 7, 7,
    7, 7, 7, 7,
    7, 7,
    7, 7
};
const uint8_t ff_vc1_bfraction_codes[23] = {
     0,   1,   2,   3,
     4,   5,   6,
   112, 113, 114, 115,
   116, 117, 118, 119,
   120, 121, 122, 123,
   124, 125,
   126, 127
};

//Same as H.264
const AVRational ff_vc1_pixel_aspect[16]={
 {0, 1},
 {1, 1},
 {12, 11},
 {10, 11},
 {16, 11},
 {40, 33},
 {24, 11},
 {20, 11},
 {32, 11},
 {80, 33},
 {18, 11},
 {15, 11},
 {64, 33},
 {160, 99},
 {0, 1},
 {0, 1}
};

/* BitPlane IMODE - such a small table... */
const uint8_t ff_vc1_imode_codes[7] = {
  0, 2, 1, 3, 1, 2, 3
};
const uint8_t ff_vc1_imode_bits[7] = {
  4, 2, 3, 2, 4, 3, 3
};

/* Normal-2 imode */
const uint8_t ff_vc1_norm2_codes[4] = {
  0, 4, 5, 3
};
const uint8_t ff_vc1_norm2_bits[4] = {
  1, 3, 3, 2
};

const uint16_t ff_vc1_norm6_codes[64] = {
0x001, 0x002, 0x003, 0x000, 0x004, 0x001, 0x002, 0x047, 0x005, 0x003, 0x004, 0x04B, 0x005, 0x04D, 0x04E, 0x30E,
0x006, 0x006, 0x007, 0x053, 0x008, 0x055, 0x056, 0x30D, 0x009, 0x059, 0x05A, 0x30C, 0x05C, 0x30B, 0x30A, 0x037,
0x007, 0x00A, 0x00B, 0x043, 0x00C, 0x045, 0x046, 0x309, 0x00D, 0x049, 0x04A, 0x308, 0x04C, 0x307, 0x306, 0x036,
0x00E, 0x051, 0x052, 0x305, 0x054, 0x304, 0x303, 0x035, 0x058, 0x302, 0x301, 0x034, 0x300, 0x033, 0x032, 0x007,
};

const uint8_t ff_vc1_norm6_bits[64] = {
 1,  4,  4,  8,  4,  8,  8, 10,  4,  8,  8, 10,  8, 10, 10, 13,
 4,  8,  8, 10,  8, 10, 10, 13,  8, 10, 10, 13, 10, 13, 13,  9,
 4,  8,  8, 10,  8, 10, 10, 13,  8, 10, 10, 13, 10, 13, 13,  9,
 8, 10, 10, 13, 10, 13, 13,  9, 10, 13, 13,  9, 13,  9,  9,  6,
};

/* 4MV Block pattern VLC tables */
const uint8_t ff_vc1_4mv_block_pattern_codes[4][16] = {
  { 14, 58, 59, 25, 12, 26, 15, 15, 13, 24, 27,  0, 28,  1,  2,  2},
  {  8, 18, 19,  4, 20,  5, 30, 11, 21, 31,  6, 12,  7, 13, 14,  0},
  { 15,  6,  7,  2,  8,  3, 28,  9, 10, 29,  4, 11,  5, 12, 13,  0},
  {  0, 11, 12,  4, 13,  5, 30, 16, 14, 31,  6, 17,  7, 18, 19, 10}
};
const uint8_t ff_vc1_4mv_block_pattern_bits[4][16] = {
  { 5, 6, 6, 5, 5, 5, 5, 4, 5, 5, 5, 3, 5, 3, 3, 2},
  { 4, 5, 5, 4, 5, 4, 5, 4, 5, 5, 4, 4, 4, 4, 4, 2},
  { 4, 4, 4, 4, 4, 4, 5, 4, 4, 5, 4, 4, 4, 4, 4, 3},
  { 2, 4, 4, 4, 4, 4, 5, 5, 4, 5, 4, 5, 4, 5, 5, 4}
};

const uint8_t wmv3_dc_scale_table[32]={
    0, 2, 4, 8, 8, 8, 9, 9,10,10,11,11,12,12,13,13,14,14,15,15,16,16,17,17,18,18,19,19,20,20,21,21
};

/* P-Picture CBPCY VLC tables */
// Looks like original tables are not conforming to standard at all. Are they used for old WMV?
const uint16_t ff_vc1_cbpcy_p_codes[4][64] = {
  {
      0,   6,  15,  13,  13,  11,   3,  13,   5,   8,  49,  10,  12, 114, 102, 119,
      1,  54,  96,   8,  10, 111,   5,  15,  12,  10,   2,  12,  13, 115,  53,  63,
      1,   7,   1,   7,  14,  12,   4,  14,   1,   9,  97,  11,   7,  58,  52,  62,
      4, 103,   1,   9,  11,  56, 101, 118,   4, 110, 100,  30,   2,   5,   4,   3
  },
  {
      0,   9,   1,  18,   5,  14, 237,  26,   3, 121,   3,  22,  13,  16,   6,  30,
      2,  10,   1,  20,  12, 241,   5,  28,  16,  12,   3,  24,  28, 124, 239, 247,
      1, 240,   1,  19,  18,  15,   4,  27,   1, 122,   2,  23,   1,  17,   7,  31,
      1,  11,   2,  21,  19, 246, 238,  29,  17,  13, 236,  25,  58,  63,   8, 125
  },
  {
      0, 201,  25, 231,   5, 221,   1,   3,   2, 414,   2, 241,  16, 225, 195, 492,
      2, 412,   1, 240,   7, 224,  98, 245,   1, 220,  96,   5,   9, 230, 101, 247,
      1, 102,   1, 415,  24,   3,   2, 244,   3,  54,   3, 484,  17, 114, 200, 493,
      3, 413,   1,   4,  13, 113,  99, 485,   4, 111, 194, 243,   5,  29,  26,  31
  },
  {
      0,  28,  12,  44,   3,  36,  20,  52,   2,  32,  16,  48,   8,  40,  24,  28,
      1,  30,  14,  46,   6,  38,  22,  54,   3,  34,  18,  50,  10,  42,  26,  30,
      1,  29,  13,  45,   5,  37,  21,  53,   2,  33,  17,  49,   9,  41,  25,  29,
      1,  31,  15,  47,   7,  39,  23,  55,   4,  35,  19,  51,  11,  43,  27,  31
   }
};

const uint8_t ff_vc1_cbpcy_p_bits[4][64] = {
  {
    13,  13,   7,  13,   7,  13,  13,  12,   6,  13,   7,  12,   6,   8,   8,   8,
     5,   7,   8,  12,   6,   8,  13,  12,   7,  13,  13,  12,   6,   8,   7,   7,
     6,  13,   8,  12,   7,  13,  13,  12,   7,  13,   8,  12,   5,   7,   7,   7,
     6,   8,  13,  12,   6,   7,   8,   8,   5,   8,   8,   6,   3,   3,   3,   2
  },
  {
    14,  13,   8,  13,   3,  13,   8,  13,   3,   7,   8,  13,   4,  13,  13,  13,
     3,  13,  13,  13,   4,   8,  13,  13,   5,  13,  13,  13,   5,   7,   8,   8,
     3,   8,  14,  13,   5,  13,  13,  13,   4,   7,  13,  13,   6,  13,  13,  13,
     5,  13,   8,  13,   5,   8,   8,  13,   5,  13,   8,  13,   6,   6,  13,   7
  },
  {
    13,   8,   6,   8,   4,   8,  13,  12,   4,   9,   8,   8,   5,   8,   8,   9,
     5,   9,  10,   8,   4,   8,   7,   8,   6,   8,   7,  13,   4,   8,   7,   8,
     5,   7,   8,   9,   6,  13,  13,   8,   4,   6,   8,   9,   5,   7,   8,   9,
     5,   9,   9,  13,   5,   7,   7,   9,   4,   7,   8,   8,   3,   5,   5,   5
  },
  {
     9,   9,   9,   9,   2,   9,   9,   9,   2,   9,   9,   9,   9,   9,   9,   8,
     3,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   8,
     2,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   8,
     9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   8
  }
};

/* MacroBlock Transform Type: 7.1.3.11, p89
 * 8x8:B
 * 8x4:B:btm  8x4:B:top  8x4:B:both,
 * 4x8:B:right  4x8:B:left  4x8:B:both
 * 4x4:B  8x8:MB
 * 8x4:MB:btm  8x4:MB:top  8x4,MB,both
 * 4x8,MB,right  4x8,MB,left
 * 4x4,MB                               */
const uint16_t ff_vc1_ttmb_codes[3][16] = {
  {
    0x0003,
    0x002E, 0x005F, 0x0000,
    0x0016, 0x0015, 0x0001,
    0x0004, 0x0014,
    0x02F1, 0x0179, 0x017B,
    0x0BC0, 0x0BC1, 0x05E1,
    0x017A
  },
  {
    0x0006,
    0x0006, 0x0003, 0x0007,
    0x000F, 0x000E, 0x0000,
    0x0002, 0x0002,
    0x0014, 0x0011, 0x000B,
    0x0009, 0x0021, 0x0015,
    0x0020
  },
  {
    0x0006,
    0x0000, 0x000E, 0x0005,
    0x0002, 0x0003, 0x0003,
    0x000F, 0x0002,
    0x0081, 0x0021, 0x0009,
    0x0101, 0x0041, 0x0011,
    0x0100
  }
};

const uint8_t ff_vc1_ttmb_bits[3][16] = {
  {
     2,
     6,  7,  2,
     5,  5,  2,
     3,  5,
    10,  9,  9,
    12, 12, 11,
     9
  },
  {
    3,
    4, 4, 4,
    4, 4, 3,
    3, 2,
    7, 7, 6,
    6, 8, 7,
    8
  },
  {
     3,
     3, 4, 5,
     3, 3, 4,
     4, 2,
    10, 8, 6,
    11, 9, 7,
    11
  }
};

/* TTBLK (Transform Type per Block) tables */
const uint8_t ff_vc1_ttblk_codes[3][8] = {
  {  0,  1,  3,  5, 16, 17, 18, 19},
  {  3,  0,  1,  2,  3,  5,  8,  9},
  {  1,  0,  1,  4,  6,  7, 10, 11}
};
const uint8_t ff_vc1_ttblk_bits[3][8] = {
  {  2,  2,  2,  3,  5,  5,  5,  5},
  {  2,  3,  3,  3,  3,  3,  4,  4},
  {  2,  3,  3,  3,  3,  3,  4,  4}
};

/* SUBBLKPAT tables, p93-94, reordered */
const uint8_t ff_vc1_subblkpat_codes[3][15] = {
  { 14, 12,  7, 11,  9, 26,  2, 10, 27,  8,  0,  6,  1, 15,  1},
  { 14,  0,  8, 15, 10,  4, 23, 13,  5,  9, 25,  3, 24, 22,  1},
  {  5,  6,  2,  2,  8,  0, 28,  3,  1,  3, 29,  1, 19, 18, 15}
};
const uint8_t ff_vc1_subblkpat_bits[3][15] = {
  {  5,  5,  5,  5,  5,  6,  4,  5,  6,  5,  4,  5,  4,  5,  1},
  {  4,  3,  4,  4,  4,  5,  5,  4,  5,  4,  5,  4,  5,  5,  2},
  {  3,  3,  4,  3,  4,  5,  5,  3,  5,  4,  5,  4,  5,  5,  4}
};

/* MV differential tables, p265 */
const uint16_t ff_vc1_mv_diff_codes[4][73] = {
  {
       0,    2,    3,    8,  576,    3,    2,    6,
       5,  577,  578,    7,    8,    9,   40,   19,
      37,   82,   21,   22,   23,  579,  580,  166,
      96,  167,   49,  194,  195,  581,  582,  583,
     292,  293,  294,   13,    2,    7,   24,   50,
     102,  295,   13,    7,    8,   18,   50,  103,
      38,   20,   21,   22,   39,  204,  103,   23,
      24,   25,  104,  410,  105,  106,  107,  108,
     109,  220,  411,  442,  222,  443,  446,  447,
       7 /* 73 elements */
  },
  {
       0,    4,    5,    3,    4,    3,    4,    5,
      20,    6,   21,   44,   45,   46, 3008,   95,
     112,  113,   57, 3009, 3010,  116,  117, 3011,
     118, 3012, 3013, 3014, 3015, 3016, 3017, 3018,
    3019, 3020, 3021, 3022,    1,    4,   15,  160,
     161,   41,    6,   11,   42,  162,   43,  119,
      56,   57,   58,  163,  236,  237, 3023,  119,
     120,  242,  122,  486, 1512,  487,  246,  494,
    1513,  495, 1514, 1515, 1516, 1517, 1518, 1519,
      31 /* 73 elements */
  },
  {
       0,  512,  513,  514,  515,    2,    3,  258,
     259,  260,  261,  262,  263,  264,  265,  266,
     267,  268,  269,  270,  271,  272,  273,  274,
     275,  276,  277,  278,  279,  280,  281,  282,
     283,  284,  285,  286,    1,    5,  287,  288,
     289,  290,    6,    7,  291,  292,  293,  294,
     295,  296,  297,  298,  299,  300,  301,  302,
     303,  304,  305,  306,  307,  308,  309,  310,
     311,  312,  313,  314,  315,  316,  317,  318,
     319 /* 73 elements */
  },
  {
       0,    1,    1,    2,    3,    4,    1,    5,
       4,    3,    5,    8,    6,    9,   10,   11,
      12,    7,  104,   14,  105,    4,   10,   15,
      11,    6,   14,    8,  106,  107,  108,   15,
     109,    9,   55,   10,    1,    2,    1,    2,
       3,   12,    6,    2,    6,    7,   28,    7,
      15,    8,    5,   18,   29,  152,   77,   24,
      25,   26,   39,  108,   13,  109,   55,   56,
      57,  116,   11,  153,  234,  235,  118,  119,
      15 /* 73 elements */
  }
};
const uint8_t ff_vc1_mv_diff_bits[4][73] = {
  {
     6,  7,  7,  8, 14,  6,  5,  6,  7, 14, 14,  6,  6,  6,  8,  9,
    10,  9,  7,  7,  7, 14, 14, 10,  9, 10,  8, 10, 10, 14, 14, 14,
    13, 13, 13,  6,  3,  5,  6,  8,  9, 13,  5,  4,  4,  5,  7,  9,
     6,  5,  5,  5,  6,  9,  8,  5,  5,  5,  7, 10,  7,  7,  7,  7,
     7,  8, 10,  9,  8,  9,  9,  9,  3 /* 73 elements */
  },
  {
     5,  7,  7,  6,  6,  5,  5,  6,  7,  5,  7,  8,  8,  8, 14,  9,
     9,  9,  8, 14, 14,  9,  9, 14,  9, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14,  2,  3,  6,  8,  8,  6,  3,  4,  6,  8,  6,  9,
     6,  6,  6,  8,  8,  8, 14,  7,  7,  8,  7,  9, 13,  9,  8,  9,
    13,  9, 13, 13, 13, 13, 13, 13,  5 /* 73 elements */

  },
  {
     3, 12, 12, 12, 12,  3,  4, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11,  1,  5, 11, 11, 11, 11,  4,  4, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11 /* 73 elements */
  },
  {
    15, 11, 15, 15, 15, 15, 12, 15, 12, 11, 12, 12, 15, 12, 12, 12,
    12, 15, 15, 12, 15, 10, 11, 12, 11, 10, 11, 10, 15, 15, 15, 11,
    15, 10, 14, 10,  4,  4,  5,  7,  8,  9,  5,  3,  4,  5,  6,  8,
     5,  4,  3,  5,  6,  8,  7,  5,  5,  5,  6,  7,  9,  7,  6,  6,
     6,  7, 10,  8,  8,  8,  7,  7,  4 /* 73 elements */
  }
};

/* DC differentials low+hi-mo, p217 are the same as in msmpeg4data .h */

/* Table 232 */
const int8_t ff_vc1_simple_progressive_4x4_zz [16] =
{
       0,     8,    16,     1,
       9,    24,    17,     2,
      10,    18,    25,     3,
      11,    26,    19,    27
};

const int8_t ff_vc1_adv_progressive_8x4_zz [32] = /* Table 233 */
{
       0,     8,     1,    16,     2,     9,    10,     3,
      24,    17,     4,    11,    18,    12,     5,    19,
      25,    13,    20,    26,    27,     6,    21,    28,
      14,    22,    29,     7,    30,    15,    23,    31
};

const int8_t ff_vc1_adv_progressive_4x8_zz [32] = /* Table 234 */
{
       0,     1,     8,     2,
       9,    16,    17,    24,
      10,    32,    25,    18,
      40,     3,    33,    26,
      48,    11,    56,    41,
      34,    49,    57,    42,
      19,    50,    27,    58,
      35,    43,    51,    59
};

const int8_t ff_vc1_adv_interlaced_8x8_zz [64] = /* Table 235 */
{
       0,     8,     1,    16,    24,     9,     2,    32,
      40,    48,    56,    17,    10,     3,    25,    18,
      11,     4,    33,    41,    49,    57,    26,    34,
      42,    50,    58,    19,    12,     5,    27,    20,
      13,     6,    35,    28,    21,    14,     7,    15,
      22,    29,    36,    43,    51,    59,    60,    52,
      44,    37,    30,    23,    31,    38,    45,    53,
      61,    62,    54,    46,    39,    47,    55,    63
};

const int8_t ff_vc1_adv_interlaced_8x4_zz [32] = /* Table 236 */
{
       0,     8,    16,    24,     1,     9,     2,    17,
      25,    10,     3,    18,    26,     4,    11,    19,
      12,     5,    13,    20,    27,     6,    21,    28,
      14,    22,    29,     7,    30,    15,    23,    31
};

const int8_t ff_vc1_adv_interlaced_4x8_zz [32] = /* Table 237 */
{
       0,     1,     2,     8,
      16,     9,    24,    17,
      10,     3,    32,    40,
      48,    56,    25,    18,
      33,    26,    41,    34,
      49,    57,    11,    42,
      19,    50,    27,    58,
      35,    43,    51,    59
};

const int8_t ff_vc1_adv_interlaced_4x4_zz [16] = /* Table 238 */
{
       0,     8,    16,    24,
       1,     9,    17,     2,
      25,    10,    18,     3,
      26,    11,    19,    27
};


/* DQScale as specified in 8.1.3.9 - almost identical to 0x40000/i */
const int32_t ff_vc1_dqscale[63] = {
0x40000, 0x20000, 0x15555, 0x10000, 0xCCCD, 0xAAAB, 0x9249, 0x8000,
    0x71C7, 0x6666, 0x5D17, 0x5555, 0x4EC5, 0x4925, 0x4444, 0x4000,
    0x3C3C, 0x38E4, 0x35E5, 0x3333, 0x30C3, 0x2E8C, 0x2C86, 0x2AAB,
    0x28F6, 0x2762, 0x25ED, 0x2492, 0x234F, 0x2222, 0x2108, 0x2000,
    0x1F08, 0x1E1E, 0x1D42, 0x1C72, 0x1BAD, 0x1AF3, 0x1A42, 0x199A,
    0x18FA, 0x1862, 0x17D0, 0x1746, 0x16C1, 0x1643, 0x15CA, 0x1555,
    0x14E6, 0x147B, 0x1414, 0x13B1, 0x1352, 0x12F7, 0x129E, 0x1249,
    0x11F7, 0x11A8, 0x115B, 0x1111, 0x10C9, 0x1084, 0x1000
};
