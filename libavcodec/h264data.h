/*
 * H26L/H264/AVC/JVT/14496-10/... encoder/decoder
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/**
 * @file h264data.h
 * @brief 
 *     H264 / AVC / MPEG4 part10 codec data table
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#define VERT_PRED             0
#define HOR_PRED              1
#define DC_PRED               2
#define DIAG_DOWN_LEFT_PRED   3
#define DIAG_DOWN_RIGHT_PRED  4
#define VERT_RIGHT_PRED       5
#define HOR_DOWN_PRED         6
#define VERT_LEFT_PRED        7
#define HOR_UP_PRED           8

#define LEFT_DC_PRED          9
#define TOP_DC_PRED           10
#define DC_128_PRED           11


#define DC_PRED8x8            0
#define HOR_PRED8x8           1
#define VERT_PRED8x8          2
#define PLANE_PRED8x8         3

#define LEFT_DC_PRED8x8       4
#define TOP_DC_PRED8x8        5
#define DC_128_PRED8x8        6

#define EXTENDED_SAR          255

static const uint16_t pixel_aspect[16][2]={
 {0, 0},
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
 {160,99},
};

static const uint8_t golomb_to_pict_type[5]=
{P_TYPE, B_TYPE, I_TYPE, SP_TYPE, SI_TYPE};

static const uint8_t pict_type_to_golomb[7]=
{-1, 2, 0, 1, -1, 4, 3};

static const uint8_t chroma_qp[52]={
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,
   12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
   28,29,29,30,31,32,32,33,34,34,35,35,36,36,37,37,
   37,38,38,38,39,39,39,39

};

static const uint8_t golomb_to_intra4x4_cbp[48]={
 47, 31, 15,  0, 23, 27, 29, 30,  7, 11, 13, 14, 39, 43, 45, 46,
 16,  3,  5, 10, 12, 19, 21, 26, 28, 35, 37, 42, 44,  1,  2,  4,
  8, 17, 18, 20, 24,  6,  9, 22, 25, 32, 33, 34, 36, 40, 38, 41
};
 
static const uint8_t golomb_to_inter_cbp[48]={
  0, 16,  1,  2,  4,  8, 32,  3,  5, 10, 12, 15, 47,  7, 11, 13,
 14,  6,  9, 31, 35, 37, 42, 44, 33, 34, 36, 40, 39, 43, 45, 46,
 17, 18, 20, 24, 19, 21, 26, 28, 23, 27, 29, 30, 22, 25, 38, 41
};

static const uint8_t intra4x4_cbp_to_golomb[48]={
  3, 29, 30, 17, 31, 18, 37,  8, 32, 38, 19,  9, 20, 10, 11,  2,
 16, 33, 34, 21, 35, 22, 39,  4, 36, 40, 23,  5, 24,  6,  7,  1,
 41, 42, 43, 25, 44, 26, 46, 12, 45, 47, 27, 13, 28, 14, 15,  0
};
 
static const uint8_t inter_cbp_to_golomb[48]={
  0,  2,  3,  7,  4,  8, 17, 13,  5, 18,  9, 14, 10, 15, 16, 11,
  1, 32, 33, 36, 34, 37, 44, 40, 35, 45, 38, 41, 39, 42, 43, 19,
  6, 24, 25, 20, 26, 21, 46, 28, 27, 47, 22, 29, 23, 30, 31, 12
};

static const uint8_t chroma_dc_coeff_token_len[4*5]={
 2, 0, 0, 0,
 6, 1, 0, 0,
 6, 6, 3, 0,
 6, 7, 7, 6,
 6, 8, 8, 7,
};

static const uint8_t chroma_dc_coeff_token_bits[4*5]={
 1, 0, 0, 0,
 7, 1, 0, 0,
 4, 6, 1, 0,
 3, 3, 2, 5,
 2, 3, 2, 0,
};

static const uint8_t coeff_token_len[4][4*17]={
{
     1, 0, 0, 0,
     6, 2, 0, 0,     8, 6, 3, 0,     9, 8, 7, 5,    10, 9, 8, 6,
    11,10, 9, 7,    13,11,10, 8,    13,13,11, 9,    13,13,13,10,
    14,14,13,11,    14,14,14,13,    15,15,14,14,    15,15,15,14,
    16,15,15,15,    16,16,16,15,    16,16,16,16,    16,16,16,16,
},
{
     2, 0, 0, 0,
     6, 2, 0, 0,     6, 5, 3, 0,     7, 6, 6, 4,     8, 6, 6, 4,
     8, 7, 7, 5,     9, 8, 8, 6,    11, 9, 9, 6,    11,11,11, 7,
    12,11,11, 9,    12,12,12,11,    12,12,12,11,    13,13,13,12,
    13,13,13,13,    13,14,13,13,    14,14,14,13,    14,14,14,14,
},
{
     4, 0, 0, 0,
     6, 4, 0, 0,     6, 5, 4, 0,     6, 5, 5, 4,     7, 5, 5, 4,
     7, 5, 5, 4,     7, 6, 6, 4,     7, 6, 6, 4,     8, 7, 7, 5,
     8, 8, 7, 6,     9, 8, 8, 7,     9, 9, 8, 8,     9, 9, 9, 8,
    10, 9, 9, 9,    10,10,10,10,    10,10,10,10,    10,10,10,10,
},
{
     6, 0, 0, 0,
     6, 6, 0, 0,     6, 6, 6, 0,     6, 6, 6, 6,     6, 6, 6, 6,
     6, 6, 6, 6,     6, 6, 6, 6,     6, 6, 6, 6,     6, 6, 6, 6,
     6, 6, 6, 6,     6, 6, 6, 6,     6, 6, 6, 6,     6, 6, 6, 6,
     6, 6, 6, 6,     6, 6, 6, 6,     6, 6, 6, 6,     6, 6, 6, 6,
}
};

static const uint8_t coeff_token_bits[4][4*17]={
{
     1, 0, 0, 0,
     5, 1, 0, 0,     7, 4, 1, 0,     7, 6, 5, 3,     7, 6, 5, 3,
     7, 6, 5, 4,    15, 6, 5, 4,    11,14, 5, 4,     8,10,13, 4,
    15,14, 9, 4,    11,10,13,12,    15,14, 9,12,    11,10,13, 8,
    15, 1, 9,12,    11,14,13, 8,     7,10, 9,12,     4, 6, 5, 8,
},
{
     3, 0, 0, 0,
    11, 2, 0, 0,     7, 7, 3, 0,     7,10, 9, 5,     7, 6, 5, 4,
     4, 6, 5, 6,     7, 6, 5, 8,    15, 6, 5, 4,    11,14,13, 4,
    15,10, 9, 4,    11,14,13,12,     8,10, 9, 8,    15,14,13,12,
    11,10, 9,12,     7,11, 6, 8,     9, 8,10, 1,     7, 6, 5, 4,
},
{
    15, 0, 0, 0,
    15,14, 0, 0,    11,15,13, 0,     8,12,14,12,    15,10,11,11,
    11, 8, 9,10,     9,14,13, 9,     8,10, 9, 8,    15,14,13,13,
    11,14,10,12,    15,10,13,12,    11,14, 9,12,     8,10,13, 8,
    13, 7, 9,12,     9,12,11,10,     5, 8, 7, 6,     1, 4, 3, 2,
},
{
     3, 0, 0, 0,
     0, 1, 0, 0,     4, 5, 6, 0,     8, 9,10,11,    12,13,14,15,
    16,17,18,19,    20,21,22,23,    24,25,26,27,    28,29,30,31,
    32,33,34,35,    36,37,38,39,    40,41,42,43,    44,45,46,47,
    48,49,50,51,    52,53,54,55,    56,57,58,59,    60,61,62,63,
}
};

static const uint8_t total_zeros_len[16][16]= {
    {1,3,3,4,4,5,5,6,6,7,7,8,8,9,9,9},  
    {3,3,3,3,3,4,4,4,4,5,5,6,6,6,6},  
    {4,3,3,3,4,4,3,3,4,5,5,6,5,6},  
    {5,3,4,4,3,3,3,4,3,4,5,5,5},  
    {4,4,4,3,3,3,3,3,4,5,4,5},  
    {6,5,3,3,3,3,3,3,4,3,6},  
    {6,5,3,3,3,2,3,4,3,6},  
    {6,4,5,3,2,2,3,3,6},  
    {6,6,4,2,2,3,2,5},  
    {5,5,3,2,2,2,4},  
    {4,4,3,3,1,3},  
    {4,4,2,1,3},  
    {3,3,1,2},  
    {2,2,1},  
    {1,1},  
};

static const uint8_t total_zeros_bits[16][16]= {
    {1,3,2,3,2,3,2,3,2,3,2,3,2,3,2,1},
    {7,6,5,4,3,5,4,3,2,3,2,3,2,1,0},
    {5,7,6,5,4,3,4,3,2,3,2,1,1,0},
    {3,7,5,4,6,5,4,3,3,2,2,1,0},
    {5,4,3,7,6,5,4,3,2,1,1,0},
    {1,1,7,6,5,4,3,2,1,1,0},
    {1,1,5,4,3,3,2,1,1,0},
    {1,1,1,3,3,2,2,1,0},
    {1,0,1,3,2,1,1,1},
    {1,0,1,3,2,1,1},
    {0,1,1,2,1,3},
    {0,1,1,1,1},
    {0,1,1,1},
    {0,1,1},
    {0,1},
};

static const uint8_t chroma_dc_total_zeros_len[3][4]= {
    { 1, 2, 3, 3,},
    { 1, 2, 2, 0,},
    { 1, 1, 0, 0,}, 
};

static const uint8_t chroma_dc_total_zeros_bits[3][4]= {
    { 1, 1, 1, 0,},
    { 1, 1, 0, 0,},
    { 1, 0, 0, 0,},
};

static const uint8_t run_len[7][16]={
    {1,1},
    {1,2,2},
    {2,2,2,2},
    {2,2,2,3,3},
    {2,2,3,3,3,3},
    {2,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,4,5,6,7,8,9,10,11},
};

static const uint8_t run_bits[7][16]={
    {1,0},
    {1,1,0},
    {3,2,1,0},
    {3,2,1,1,0},
    {3,2,3,2,1,0},
    {3,0,1,3,2,5,4},
    {7,6,5,4,3,2,1,1,1,1,1,1,1,1,1},
};

/*
o-o o-o
 / / /
o-o o-o
 ,---'
o-o o-o
 / / /
o-o o-o
*/

static const uint8_t scan8[16 + 2*4]={
 4+1*8, 5+1*8, 4+2*8, 5+2*8,
 6+1*8, 7+1*8, 6+2*8, 7+2*8,
 4+3*8, 5+3*8, 4+4*8, 5+4*8,
 6+3*8, 7+3*8, 6+4*8, 7+4*8,
 1+1*8, 2+1*8,
 1+2*8, 2+2*8,
 1+4*8, 2+4*8,
 1+5*8, 2+5*8,
};

static const uint8_t zigzag_scan[16]={
 0+0*4, 1+0*4, 0+1*4, 0+2*4, 
 1+1*4, 2+0*4, 3+0*4, 2+1*4, 
 1+2*4, 0+3*4, 1+3*4, 2+2*4, 
 3+1*4, 3+2*4, 2+3*4, 3+3*4, 
};

static const uint8_t field_scan[16]={
 0+0*4, 0+1*4, 1+0*4, 0+2*4, 
 0+3*4, 1+1*4, 1+2*4, 1+3*4,
 2+0*4, 2+1*4, 2+2*4, 2+3*4, 
 3+0*4, 3+1*4, 3+2*4, 3+3*4,
};

static const uint8_t luma_dc_zigzag_scan[16]={
 0*16 + 0*64, 1*16 + 0*64, 2*16 + 0*64, 0*16 + 2*64,
 3*16 + 0*64, 0*16 + 1*64, 1*16 + 1*64, 2*16 + 1*64,
 1*16 + 2*64, 2*16 + 2*64, 3*16 + 2*64, 0*16 + 3*64,
 3*16 + 1*64, 1*16 + 3*64, 2*16 + 3*64, 3*16 + 3*64,
};

static const uint8_t luma_dc_field_scan[16]={
 0*16 + 0*64, 2*16 + 0*64, 1*16 + 0*64, 0*16 + 2*64, 
 2*16 + 2*64, 3*16 + 0*64, 1*16 + 2*64, 3*16 + 2*64, 
 0*16 + 1*64, 2*16 + 1*64, 0*16 + 3*64, 2*16 + 3*64, 
 1*16 + 1*64, 3*16 + 1*64, 1*16 + 3*64, 3*16 + 3*64,
};

static const uint8_t chroma_dc_scan[4]={
 (0+0*2)*16, (1+0*2)*16, 
 (0+1*2)*16, (1+1*2)*16,  //FIXME
};

typedef struct IMbInfo{
    uint16_t type;
    uint8_t pred_mode;
    uint8_t cbp;
} IMbInfo;

static const IMbInfo i_mb_type_info[26]={
{MB_TYPE_INTRA4x4  , -1, -1},
{MB_TYPE_INTRA16x16,  2,  0},
{MB_TYPE_INTRA16x16,  1,  0},
{MB_TYPE_INTRA16x16,  0,  0},
{MB_TYPE_INTRA16x16,  3,  0},
{MB_TYPE_INTRA16x16,  2,  16},
{MB_TYPE_INTRA16x16,  1,  16},
{MB_TYPE_INTRA16x16,  0,  16},
{MB_TYPE_INTRA16x16,  3,  16},
{MB_TYPE_INTRA16x16,  2,  32},
{MB_TYPE_INTRA16x16,  1,  32},
{MB_TYPE_INTRA16x16,  0,  32},
{MB_TYPE_INTRA16x16,  3,  32},
{MB_TYPE_INTRA16x16,  2,  15+0},
{MB_TYPE_INTRA16x16,  1,  15+0},
{MB_TYPE_INTRA16x16,  0,  15+0},
{MB_TYPE_INTRA16x16,  3,  15+0},
{MB_TYPE_INTRA16x16,  2,  15+16},
{MB_TYPE_INTRA16x16,  1,  15+16},
{MB_TYPE_INTRA16x16,  0,  15+16},
{MB_TYPE_INTRA16x16,  3,  15+16},
{MB_TYPE_INTRA16x16,  2,  15+32},
{MB_TYPE_INTRA16x16,  1,  15+32},
{MB_TYPE_INTRA16x16,  0,  15+32},
{MB_TYPE_INTRA16x16,  3,  15+32},
{MB_TYPE_INTRA_PCM , -1, -1},
};

typedef struct PMbInfo{
    uint16_t type;
    uint8_t partition_count;
} PMbInfo;

static const PMbInfo p_mb_type_info[5]={
{MB_TYPE_16x16|MB_TYPE_P0L0             , 1},
{MB_TYPE_16x8 |MB_TYPE_P0L0|MB_TYPE_P1L0, 2},
{MB_TYPE_8x16 |MB_TYPE_P0L0|MB_TYPE_P1L0, 2},
{MB_TYPE_8x8                            , 4},
{MB_TYPE_8x8  |MB_TYPE_REF0             , 4},
};

static const PMbInfo p_sub_mb_type_info[4]={
{MB_TYPE_16x16|MB_TYPE_P0L0             , 1},
{MB_TYPE_16x8 |MB_TYPE_P0L0             , 2},
{MB_TYPE_8x16 |MB_TYPE_P0L0             , 2},
{MB_TYPE_8x8  |MB_TYPE_P0L0             , 4},
};

static const PMbInfo b_mb_type_info[23]={
{MB_TYPE_DIRECT                                                   , 1, },
{MB_TYPE_16x16|MB_TYPE_P0L0                                       , 1, },
{MB_TYPE_16x16             |MB_TYPE_P0L1                          , 1, },
{MB_TYPE_16x16|MB_TYPE_P0L0|MB_TYPE_P0L1                          , 1, },
{MB_TYPE_16x8 |MB_TYPE_P0L0             |MB_TYPE_P1L0             , 2, },
{MB_TYPE_8x16 |MB_TYPE_P0L0             |MB_TYPE_P1L0             , 2, },
{MB_TYPE_16x8              |MB_TYPE_P0L1             |MB_TYPE_P1L1, 2, },
{MB_TYPE_8x16              |MB_TYPE_P0L1             |MB_TYPE_P1L1, 2, },
{MB_TYPE_16x8 |MB_TYPE_P0L0                          |MB_TYPE_P1L1, 2, },
{MB_TYPE_8x16 |MB_TYPE_P0L0                          |MB_TYPE_P1L1, 2, },
{MB_TYPE_16x8              |MB_TYPE_P0L1|MB_TYPE_P1L0             , 2, },
{MB_TYPE_8x16              |MB_TYPE_P0L1|MB_TYPE_P1L0             , 2, },
{MB_TYPE_16x8 |MB_TYPE_P0L0             |MB_TYPE_P1L0|MB_TYPE_P1L1, 2, },
{MB_TYPE_8x16 |MB_TYPE_P0L0             |MB_TYPE_P1L0|MB_TYPE_P1L1, 2, },
{MB_TYPE_16x8              |MB_TYPE_P0L1|MB_TYPE_P1L0|MB_TYPE_P1L1, 2, },
{MB_TYPE_8x16              |MB_TYPE_P0L1|MB_TYPE_P1L0|MB_TYPE_P1L1, 2, },
{MB_TYPE_16x8 |MB_TYPE_P0L0|MB_TYPE_P0L1|MB_TYPE_P1L0             , 2, },
{MB_TYPE_8x16 |MB_TYPE_P0L0|MB_TYPE_P0L1|MB_TYPE_P1L0             , 2, },
{MB_TYPE_16x8 |MB_TYPE_P0L0|MB_TYPE_P0L1             |MB_TYPE_P1L1, 2, },
{MB_TYPE_8x16 |MB_TYPE_P0L0|MB_TYPE_P0L1             |MB_TYPE_P1L1, 2, },
{MB_TYPE_16x8 |MB_TYPE_P0L0|MB_TYPE_P0L1|MB_TYPE_P1L0|MB_TYPE_P1L1, 2, },
{MB_TYPE_8x16 |MB_TYPE_P0L0|MB_TYPE_P0L1|MB_TYPE_P1L0|MB_TYPE_P1L1, 2, },
{MB_TYPE_8x8                                                      , 4, },
};

static const PMbInfo b_sub_mb_type_info[13]={
{MB_TYPE_DIRECT                                                   , 1, },
{MB_TYPE_16x16|MB_TYPE_P0L0                                       , 1, },
{MB_TYPE_16x16             |MB_TYPE_P0L1                          , 1, },
{MB_TYPE_16x16|MB_TYPE_P0L0|MB_TYPE_P0L1                          , 1, },
{MB_TYPE_16x8 |MB_TYPE_P0L0             |MB_TYPE_P1L0             , 2, },
{MB_TYPE_8x16 |MB_TYPE_P0L0             |MB_TYPE_P1L0             , 2, },
{MB_TYPE_16x8              |MB_TYPE_P0L1             |MB_TYPE_P1L1, 2, },
{MB_TYPE_8x16              |MB_TYPE_P0L1             |MB_TYPE_P1L1, 2, },
{MB_TYPE_16x8 |MB_TYPE_P0L0|MB_TYPE_P0L1|MB_TYPE_P1L0|MB_TYPE_P1L1, 2, },
{MB_TYPE_8x16 |MB_TYPE_P0L0|MB_TYPE_P0L1|MB_TYPE_P1L0|MB_TYPE_P1L1, 2, },
{MB_TYPE_8x8  |MB_TYPE_P0L0             |MB_TYPE_P1L0             , 4, },
{MB_TYPE_8x8               |MB_TYPE_P0L1             |MB_TYPE_P1L1, 4, },
{MB_TYPE_8x8  |MB_TYPE_P0L0|MB_TYPE_P0L1|MB_TYPE_P1L0|MB_TYPE_P1L1, 4, },
};


static const uint8_t rem6[52]={
0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 
};

static const uint8_t div6[52]={
0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8,
};

static const uint16_t dequant_coeff[52][16]={
{  10,  13,  10,  13,   13,  16,  13,  16,   10,  13,  10,  13,   13,  16,  13,  16, },
{  11,  14,  11,  14,   14,  18,  14,  18,   11,  14,  11,  14,   14,  18,  14,  18, },
{  13,  16,  13,  16,   16,  20,  16,  20,   13,  16,  13,  16,   16,  20,  16,  20, },
{  14,  18,  14,  18,   18,  23,  18,  23,   14,  18,  14,  18,   18,  23,  18,  23, },
{  16,  20,  16,  20,   20,  25,  20,  25,   16,  20,  16,  20,   20,  25,  20,  25, },
{  18,  23,  18,  23,   23,  29,  23,  29,   18,  23,  18,  23,   23,  29,  23,  29, },
{  20,  26,  20,  26,   26,  32,  26,  32,   20,  26,  20,  26,   26,  32,  26,  32, },
{  22,  28,  22,  28,   28,  36,  28,  36,   22,  28,  22,  28,   28,  36,  28,  36, },
{  26,  32,  26,  32,   32,  40,  32,  40,   26,  32,  26,  32,   32,  40,  32,  40, },
{  28,  36,  28,  36,   36,  46,  36,  46,   28,  36,  28,  36,   36,  46,  36,  46, },
{  32,  40,  32,  40,   40,  50,  40,  50,   32,  40,  32,  40,   40,  50,  40,  50, },
{  36,  46,  36,  46,   46,  58,  46,  58,   36,  46,  36,  46,   46,  58,  46,  58, },
{  40,  52,  40,  52,   52,  64,  52,  64,   40,  52,  40,  52,   52,  64,  52,  64, },
{  44,  56,  44,  56,   56,  72,  56,  72,   44,  56,  44,  56,   56,  72,  56,  72, },
{  52,  64,  52,  64,   64,  80,  64,  80,   52,  64,  52,  64,   64,  80,  64,  80, },
{  56,  72,  56,  72,   72,  92,  72,  92,   56,  72,  56,  72,   72,  92,  72,  92, },
{  64,  80,  64,  80,   80, 100,  80, 100,   64,  80,  64,  80,   80, 100,  80, 100, },
{  72,  92,  72,  92,   92, 116,  92, 116,   72,  92,  72,  92,   92, 116,  92, 116, },
{  80, 104,  80, 104,  104, 128, 104, 128,   80, 104,  80, 104,  104, 128, 104, 128, },
{  88, 112,  88, 112,  112, 144, 112, 144,   88, 112,  88, 112,  112, 144, 112, 144, },
{ 104, 128, 104, 128,  128, 160, 128, 160,  104, 128, 104, 128,  128, 160, 128, 160, },
{ 112, 144, 112, 144,  144, 184, 144, 184,  112, 144, 112, 144,  144, 184, 144, 184, },
{ 128, 160, 128, 160,  160, 200, 160, 200,  128, 160, 128, 160,  160, 200, 160, 200, },
{ 144, 184, 144, 184,  184, 232, 184, 232,  144, 184, 144, 184,  184, 232, 184, 232, },
{ 160, 208, 160, 208,  208, 256, 208, 256,  160, 208, 160, 208,  208, 256, 208, 256, },
{ 176, 224, 176, 224,  224, 288, 224, 288,  176, 224, 176, 224,  224, 288, 224, 288, },
{ 208, 256, 208, 256,  256, 320, 256, 320,  208, 256, 208, 256,  256, 320, 256, 320, },
{ 224, 288, 224, 288,  288, 368, 288, 368,  224, 288, 224, 288,  288, 368, 288, 368, },
{ 256, 320, 256, 320,  320, 400, 320, 400,  256, 320, 256, 320,  320, 400, 320, 400, },
{ 288, 368, 288, 368,  368, 464, 368, 464,  288, 368, 288, 368,  368, 464, 368, 464, },
{ 320, 416, 320, 416,  416, 512, 416, 512,  320, 416, 320, 416,  416, 512, 416, 512, },
{ 352, 448, 352, 448,  448, 576, 448, 576,  352, 448, 352, 448,  448, 576, 448, 576, },
{ 416, 512, 416, 512,  512, 640, 512, 640,  416, 512, 416, 512,  512, 640, 512, 640, },
{ 448, 576, 448, 576,  576, 736, 576, 736,  448, 576, 448, 576,  576, 736, 576, 736, },
{ 512, 640, 512, 640,  640, 800, 640, 800,  512, 640, 512, 640,  640, 800, 640, 800, },
{ 576, 736, 576, 736,  736, 928, 736, 928,  576, 736, 576, 736,  736, 928, 736, 928, },
{ 640, 832, 640, 832,  832,1024, 832,1024,  640, 832, 640, 832,  832,1024, 832,1024, },
{ 704, 896, 704, 896,  896,1152, 896,1152,  704, 896, 704, 896,  896,1152, 896,1152, },
{ 832,1024, 832,1024, 1024,1280,1024,1280,  832,1024, 832,1024, 1024,1280,1024,1280, },
{ 896,1152, 896,1152, 1152,1472,1152,1472,  896,1152, 896,1152, 1152,1472,1152,1472, },
{1024,1280,1024,1280, 1280,1600,1280,1600, 1024,1280,1024,1280, 1280,1600,1280,1600, },
{1152,1472,1152,1472, 1472,1856,1472,1856, 1152,1472,1152,1472, 1472,1856,1472,1856, },
{1280,1664,1280,1664, 1664,2048,1664,2048, 1280,1664,1280,1664, 1664,2048,1664,2048, },
{1408,1792,1408,1792, 1792,2304,1792,2304, 1408,1792,1408,1792, 1792,2304,1792,2304, },
{1664,2048,1664,2048, 2048,2560,2048,2560, 1664,2048,1664,2048, 2048,2560,2048,2560, },
{1792,2304,1792,2304, 2304,2944,2304,2944, 1792,2304,1792,2304, 2304,2944,2304,2944, },
{2048,2560,2048,2560, 2560,3200,2560,3200, 2048,2560,2048,2560, 2560,3200,2560,3200, },
{2304,2944,2304,2944, 2944,3712,2944,3712, 2304,2944,2304,2944, 2944,3712,2944,3712, },
{2560,3328,2560,3328, 3328,4096,3328,4096, 2560,3328,2560,3328, 3328,4096,3328,4096, },
{2816,3584,2816,3584, 3584,4608,3584,4608, 2816,3584,2816,3584, 3584,4608,3584,4608, },
{3328,4096,3328,4096, 4096,5120,4096,5120, 3328,4096,3328,4096, 4096,5120,4096,5120, },
{3584,4608,3584,4608, 4608,5888,4608,5888, 3584,4608,3584,4608, 4608,5888,4608,5888, },
//{4096,5120,4096,5120, 5120,6400,5120,6400, 4096,5120,4096,5120, 5120,6400,5120,6400, },
//{4608,5888,4608,5888, 5888,7424,5888,7424, 4608,5888,4608,5888, 5888,7424,5888,7424, },
};

#define QUANT_SHIFT 22

static const int quant_coeff[52][16]={
    { 419430,258111,419430,258111,258111,167772,258111,167772,419430,258111,419430,258111,258111,167772,258111,167772,},
    { 381300,239675,381300,239675,239675,149131,239675,149131,381300,239675,381300,239675,239675,149131,239675,149131,},
    { 322639,209715,322639,209715,209715,134218,209715,134218,322639,209715,322639,209715,209715,134218,209715,134218,},
    { 299593,186414,299593,186414,186414,116711,186414,116711,299593,186414,299593,186414,186414,116711,186414,116711,},
    { 262144,167772,262144,167772,167772,107374,167772,107374,262144,167772,262144,167772,167772,107374,167772,107374,},
    { 233017,145889,233017,145889,145889, 92564,145889, 92564,233017,145889,233017,145889,145889, 92564,145889, 92564,},
    { 209715,129056,209715,129056,129056, 83886,129056, 83886,209715,129056,209715,129056,129056, 83886,129056, 83886,},
    { 190650,119837,190650,119837,119837, 74565,119837, 74565,190650,119837,190650,119837,119837, 74565,119837, 74565,},
    { 161319,104858,161319,104858,104858, 67109,104858, 67109,161319,104858,161319,104858,104858, 67109,104858, 67109,},
    { 149797, 93207,149797, 93207, 93207, 58356, 93207, 58356,149797, 93207,149797, 93207, 93207, 58356, 93207, 58356,},
    { 131072, 83886,131072, 83886, 83886, 53687, 83886, 53687,131072, 83886,131072, 83886, 83886, 53687, 83886, 53687,},
    { 116508, 72944,116508, 72944, 72944, 46282, 72944, 46282,116508, 72944,116508, 72944, 72944, 46282, 72944, 46282,},
    { 104858, 64528,104858, 64528, 64528, 41943, 64528, 41943,104858, 64528,104858, 64528, 64528, 41943, 64528, 41943,},
    {  95325, 59919, 95325, 59919, 59919, 37283, 59919, 37283, 95325, 59919, 95325, 59919, 59919, 37283, 59919, 37283,},
    {  80660, 52429, 80660, 52429, 52429, 33554, 52429, 33554, 80660, 52429, 80660, 52429, 52429, 33554, 52429, 33554,},
    {  74898, 46603, 74898, 46603, 46603, 29178, 46603, 29178, 74898, 46603, 74898, 46603, 46603, 29178, 46603, 29178,},
    {  65536, 41943, 65536, 41943, 41943, 26844, 41943, 26844, 65536, 41943, 65536, 41943, 41943, 26844, 41943, 26844,},
    {  58254, 36472, 58254, 36472, 36472, 23141, 36472, 23141, 58254, 36472, 58254, 36472, 36472, 23141, 36472, 23141,},
    {  52429, 32264, 52429, 32264, 32264, 20972, 32264, 20972, 52429, 32264, 52429, 32264, 32264, 20972, 32264, 20972,},
    {  47663, 29959, 47663, 29959, 29959, 18641, 29959, 18641, 47663, 29959, 47663, 29959, 29959, 18641, 29959, 18641,},
    {  40330, 26214, 40330, 26214, 26214, 16777, 26214, 16777, 40330, 26214, 40330, 26214, 26214, 16777, 26214, 16777,},
    {  37449, 23302, 37449, 23302, 23302, 14589, 23302, 14589, 37449, 23302, 37449, 23302, 23302, 14589, 23302, 14589,},
    {  32768, 20972, 32768, 20972, 20972, 13422, 20972, 13422, 32768, 20972, 32768, 20972, 20972, 13422, 20972, 13422,},
    {  29127, 18236, 29127, 18236, 18236, 11570, 18236, 11570, 29127, 18236, 29127, 18236, 18236, 11570, 18236, 11570,},
    {  26214, 16132, 26214, 16132, 16132, 10486, 16132, 10486, 26214, 16132, 26214, 16132, 16132, 10486, 16132, 10486,},
    {  23831, 14980, 23831, 14980, 14980,  9321, 14980,  9321, 23831, 14980, 23831, 14980, 14980,  9321, 14980,  9321,},
    {  20165, 13107, 20165, 13107, 13107,  8389, 13107,  8389, 20165, 13107, 20165, 13107, 13107,  8389, 13107,  8389,},
    {  18725, 11651, 18725, 11651, 11651,  7294, 11651,  7294, 18725, 11651, 18725, 11651, 11651,  7294, 11651,  7294,},
    {  16384, 10486, 16384, 10486, 10486,  6711, 10486,  6711, 16384, 10486, 16384, 10486, 10486,  6711, 10486,  6711,},
    {  14564,  9118, 14564,  9118,  9118,  5785,  9118,  5785, 14564,  9118, 14564,  9118,  9118,  5785,  9118,  5785,},
    {  13107,  8066, 13107,  8066,  8066,  5243,  8066,  5243, 13107,  8066, 13107,  8066,  8066,  5243,  8066,  5243,},
    {  11916,  7490, 11916,  7490,  7490,  4660,  7490,  4660, 11916,  7490, 11916,  7490,  7490,  4660,  7490,  4660,},
    {  10082,  6554, 10082,  6554,  6554,  4194,  6554,  4194, 10082,  6554, 10082,  6554,  6554,  4194,  6554,  4194,},
    {   9362,  5825,  9362,  5825,  5825,  3647,  5825,  3647,  9362,  5825,  9362,  5825,  5825,  3647,  5825,  3647,},
    {   8192,  5243,  8192,  5243,  5243,  3355,  5243,  3355,  8192,  5243,  8192,  5243,  5243,  3355,  5243,  3355,},
    {   7282,  4559,  7282,  4559,  4559,  2893,  4559,  2893,  7282,  4559,  7282,  4559,  4559,  2893,  4559,  2893,},
    {   6554,  4033,  6554,  4033,  4033,  2621,  4033,  2621,  6554,  4033,  6554,  4033,  4033,  2621,  4033,  2621,},
    {   5958,  3745,  5958,  3745,  3745,  2330,  3745,  2330,  5958,  3745,  5958,  3745,  3745,  2330,  3745,  2330,},
    {   5041,  3277,  5041,  3277,  3277,  2097,  3277,  2097,  5041,  3277,  5041,  3277,  3277,  2097,  3277,  2097,},
    {   4681,  2913,  4681,  2913,  2913,  1824,  2913,  1824,  4681,  2913,  4681,  2913,  2913,  1824,  2913,  1824,},
    {   4096,  2621,  4096,  2621,  2621,  1678,  2621,  1678,  4096,  2621,  4096,  2621,  2621,  1678,  2621,  1678,},
    {   3641,  2280,  3641,  2280,  2280,  1446,  2280,  1446,  3641,  2280,  3641,  2280,  2280,  1446,  2280,  1446,},
    {   3277,  2016,  3277,  2016,  2016,  1311,  2016,  1311,  3277,  2016,  3277,  2016,  2016,  1311,  2016,  1311,},
    {   2979,  1872,  2979,  1872,  1872,  1165,  1872,  1165,  2979,  1872,  2979,  1872,  1872,  1165,  1872,  1165,},
    {   2521,  1638,  2521,  1638,  1638,  1049,  1638,  1049,  2521,  1638,  2521,  1638,  1638,  1049,  1638,  1049,},
    {   2341,  1456,  2341,  1456,  1456,   912,  1456,   912,  2341,  1456,  2341,  1456,  1456,   912,  1456,   912,},
    {   2048,  1311,  2048,  1311,  1311,   839,  1311,   839,  2048,  1311,  2048,  1311,  1311,   839,  1311,   839,},
    {   1820,  1140,  1820,  1140,  1140,   723,  1140,   723,  1820,  1140,  1820,  1140,  1140,   723,  1140,   723,},
    {   1638,  1008,  1638,  1008,  1008,   655,  1008,   655,  1638,  1008,  1638,  1008,  1008,   655,  1008,   655,},
    {   1489,   936,  1489,   936,   936,   583,   936,   583,  1489,   936,  1489,   936,   936,   583,   936,   583,},
    {   1260,   819,  1260,   819,   819,   524,   819,   524,  1260,   819,  1260,   819,   819,   524,   819,   524,},
    {   1170,   728,  1170,   728,   728,   456,   728,   456,  1170,   728,  1170,   728,   728,   456,   728,   456,},
};
