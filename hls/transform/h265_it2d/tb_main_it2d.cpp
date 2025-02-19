/*****************************************************************************
 * This file is part of uvgKvazaarHW.
 *
 * Copyright (c) 2025, Tampere University, ITU/ISO/IEC, project contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * * Neither the name of the Tampere University or ITU/ISO/IEC nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * INCLUDING NEGLIGENCE OR OTHERWISE ARISING IN ANY WAY OUT OF THE USE OF THIS
 ****************************************************************************/

#include "global.h"
#include <iostream>
#include <mc_scverify.h>

using namespace std;

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define CLIP(low, high, value) MAX((low), MIN((high), (value)))

const int_16 kvz_g_dst_4[4][4] = {{29, 55, 74, 84}, {74, 74, 0, -74}, {84, -29, -74, 55}, {55, -84, 74, -29}};

const int_16 kvz_g_dct_4[4][4] = {{64, 64, 64, 64}, {83, 36, -36, -83}, {64, -64, -64, 64}, {36, -83, 83, -36}};

const int_16 kvz_g_dct_8[8][8] = {{64, 64, 64, 64, 64, 64, 64, 64},
                                  {89, 75, 50, 18, -18, -50, -75, -89},
                                  {83, 36, -36, -83, -83, -36, 36, 83},
                                  {75, -18, -89, -50, 50, 89, 18, -75},
                                  {64, -64, -64, 64, 64, -64, -64, 64},
                                  {50, -89, 18, 75, -75, -18, 89, -50},
                                  {36, -83, 83, -36, -36, 83, -83, 36},
                                  {18, -50, 75, -89, 89, -75, 50, -18}};

const int_16 kvz_g_dct_16[16][16] = {{64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},
                                     {90, 87, 80, 70, 57, 43, 25, 9, -9, -25, -43, -57, -70, -80, -87, -90},
                                     {89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89},
                                     {87, 57, 9, -43, -80, -90, -70, -25, 25, 70, 90, 80, 43, -9, -57, -87},
                                     {83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83},
                                     {80, 9, -70, -87, -25, 57, 90, 43, -43, -90, -57, 25, 87, 70, -9, -80},
                                     {75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75},
                                     {70, -43, -87, 9, 90, 25, -80, -57, 57, 80, -25, -90, -9, 87, 43, -70},
                                     {64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64},
                                     {57, -80, -25, 90, -9, -87, 43, 70, -70, -43, 87, 9, -90, 25, 80, -57},
                                     {50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50},
                                     {43, -90, 57, 25, -87, 70, 9, -80, 80, -9, -70, 87, -25, -57, 90, -43},
                                     {36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36},
                                     {25, -70, 90, -80, 43, 9, -57, 87, -87, 57, -9, -43, 80, -90, 70, -25},
                                     {18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18},
                                     {9, -25, 43, -57, 70, -80, 87, -90, 90, -87, 80, -70, 57, -43, 25, -9}};

const int_16 kvz_g_dct_32[32][32] = {{64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},
                                     {90, 90, 88, 85, 82, 78, 73, 67, 61, 54, 46, 38, 31, 22, 13, 4, -4, -13, -22, -31, -38, -46, -54, -61, -67, -73, -78, -82, -85, -88, -90, -90},
                                     {90, 87, 80, 70, 57, 43, 25, 9, -9, -25, -43, -57, -70, -80, -87, -90, -90, -87, -80, -70, -57, -43, -25, -9, 9, 25, 43, 57, 70, 80, 87, 90},
                                     {90, 82, 67, 46, 22, -4, -31, -54, -73, -85, -90, -88, -78, -61, -38, -13, 13, 38, 61, 78, 88, 90, 85, 73, 54, 31, 4, -22, -46, -67, -82, -90},
                                     {89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89, 89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89},
                                     {88, 67, 31, -13, -54, -82, -90, -78, -46, -4, 38, 73, 90, 85, 61, 22, -22, -61, -85, -90, -73, -38, 4, 46, 78, 90, 82, 54, 13, -31, -67, -88},
                                     {87, 57, 9, -43, -80, -90, -70, -25, 25, 70, 90, 80, 43, -9, -57, -87, -87, -57, -9, 43, 80, 90, 70, 25, -25, -70, -90, -80, -43, 9, 57, 87},
                                     {85, 46, -13, -67, -90, -73, -22, 38, 82, 88, 54, -4, -61, -90, -78, -31, 31, 78, 90, 61, 4, -54, -88, -82, -38, 22, 73, 90, 67, 13, -46, -85},
                                     {83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83},
                                     {82, 22, -54, -90, -61, 13, 78, 85, 31, -46, -90, -67, 4, 73, 88, 38, -38, -88, -73, -4, 67, 90, 46, -31, -85, -78, -13, 61, 90, 54, -22, -82},
                                     {80, 9, -70, -87, -25, 57, 90, 43, -43, -90, -57, 25, 87, 70, -9, -80, -80, -9, 70, 87, 25, -57, -90, -43, 43, 90, 57, -25, -87, -70, 9, 80},
                                     {78, -4, -82, -73, 13, 85, 67, -22, -88, -61, 31, 90, 54, -38, -90, -46, 46, 90, 38, -54, -90, -31, 61, 88, 22, -67, -85, -13, 73, 82, 4, -78},
                                     {75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75, 75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75},
                                     {73, -31, -90, -22, 78, 67, -38, -90, -13, 82, 61, -46, -88, -4, 85, 54, -54, -85, 4, 88, 46, -61, -82, 13, 90, 38, -67, -78, 22, 90, 31, -73},
                                     {70, -43, -87, 9, 90, 25, -80, -57, 57, 80, -25, -90, -9, 87, 43, -70, -70, 43, 87, -9, -90, -25, 80, 57, -57, -80, 25, 90, 9, -87, -43, 70},
                                     {67, -54, -78, 38, 85, -22, -90, 4, 90, 13, -88, -31, 82, 46, -73, -61, 61, 73, -46, -82, 31, 88, -13, -90, -4, 90, 22, -85, -38, 78, 54, -67},
                                     {64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64},
                                     {61, -73, -46, 82, 31, -88, -13, 90, -4, -90, 22, 85, -38, -78, 54, 67, -67, -54, 78, 38, -85, -22, 90, 4, -90, 13, 88, -31, -82, 46, 73, -61},
                                     {57, -80, -25, 90, -9, -87, 43, 70, -70, -43, 87, 9, -90, 25, 80, -57, -57, 80, 25, -90, 9, 87, -43, -70, 70, 43, -87, -9, 90, -25, -80, 57},
                                     {54, -85, -4, 88, -46, -61, 82, 13, -90, 38, 67, -78, -22, 90, -31, -73, 73, 31, -90, 22, 78, -67, -38, 90, -13, -82, 61, 46, -88, 4, 85, -54},
                                     {50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50, 50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50},
                                     {46, -90, 38, 54, -90, 31, 61, -88, 22, 67, -85, 13, 73, -82, 4, 78, -78, -4, 82, -73, -13, 85, -67, -22, 88, -61, -31, 90, -54, -38, 90, -46},
                                     {43, -90, 57, 25, -87, 70, 9, -80, 80, -9, -70, 87, -25, -57, 90, -43, -43, 90, -57, -25, 87, -70, -9, 80, -80, 9, 70, -87, 25, 57, -90, 43},
                                     {38, -88, 73, -4, -67, 90, -46, -31, 85, -78, 13, 61, -90, 54, 22, -82, 82, -22, -54, 90, -61, -13, 78, -85, 31, 46, -90, 67, 4, -73, 88, -38},
                                     {36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36},
                                     {31, -78, 90, -61, 4, 54, -88, 82, -38, -22, 73, -90, 67, -13, -46, 85, -85, 46, 13, -67, 90, -73, 22, 38, -82, 88, -54, -4, 61, -90, 78, -31},
                                     {25, -70, 90, -80, 43, 9, -57, 87, -87, 57, -9, -43, 80, -90, 70, -25, -25, 70, -90, 80, -43, -9, 57, -87, 87, -57, 9, 43, -80, 90, -70, 25},
                                     {22, -61, 85, -90, 73, -38, -4, 46, -78, 90, -82, 54, -13, -31, 67, -88, 88, -67, 31, 13, -54, 82, -90, 78, -46, 4, 38, -73, 90, -85, 61, -22},
                                     {18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18, 18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18},
                                     {13, -38, 61, -78, 88, -90, 85, -73, 54, -31, 4, 22, -46, 67, -82, 90, -90, 82, -67, 46, -22, -4, 31, -54, 73, -85, 90, -88, 78, -61, 38, -13},
                                     {9, -25, 43, -57, 70, -80, 87, -90, 90, -87, 80, -70, 57, -43, 25, -9, -9, 25, -43, 57, -70, 80, -87, 90, -90, 87, -80, 70, -57, 43, -25, 9},
                                     {4, -13, 22, -31, 38, -46, 54, -61, 67, -73, 78, -82, 85, -88, 90, -90, 90, -90, 88, -85, 82, -78, 73, -67, 61, -54, 46, -38, 31, -22, 13, -4}};

const int_8 kvz_g_convert_to_bit[LCU_WIDTH + 1] = {-1, -1, -1, -1, 0,  -1, -1, -1, 1,  -1, -1, -1, -1, -1, -1, -1, 2,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3,
                                                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4};

static void fast_inverse_dst_4_generic(const int_16 *src, int_16 *dst, int_32 shift) {
  int i, c[4];
  int_32 rnd_factor = 1 << (shift - 1);

  for (i = 0; i < 4; i++) {
    // Intermediate Variables
    c[0] = src[i] + src[8 + i];
    c[1] = src[8 + i] + src[12 + i];
    c[2] = src[i] - src[12 + i];
    c[3] = 74 * src[4 + i];

    dst[4 * i + 0] = (int_16)CLIP((int_32)-32768, (int_32)32767, (int_32)((29 * c[0] + 55 * c[1] + c[3] + rnd_factor) >> shift));
    dst[4 * i + 1] = (int_16)CLIP((int_32)-32768, (int_32)32767, (int_32)((55 * c[2] - 29 * c[1] + c[3] + rnd_factor) >> shift));
    dst[4 * i + 2] = (int_16)CLIP((int_32)-32768, (int_32)32767, (int_32)((74 * (src[i] - src[8 + i] + src[12 + i]) + rnd_factor) >> shift));
    dst[4 * i + 3] = (int_16)CLIP((int_32)-32768, (int_32)32767, (int_32)((55 * c[0] + 29 * c[2] - c[3] + rnd_factor) >> shift));
  }
}

static void partial_butterfly_inverse_4_generic(const int_16 *src, int_16 *dst, int shift) {
  int j;
  int e[2], o[2];
  int add = 1 << (shift - 1);
  const int_32 line = 4;

  for (j = 0; j < line; j++) {
    // Utilizing symmetry properties to the maximum to minimize the number of multiplications
    o[0] = kvz_g_dct_4[1][0] * src[line] + kvz_g_dct_4[3][0] * src[3 * line];
    o[1] = kvz_g_dct_4[1][1] * src[line] + kvz_g_dct_4[3][1] * src[3 * line];
    e[0] = kvz_g_dct_4[0][0] * src[0] + kvz_g_dct_4[2][0] * src[2 * line];
    e[1] = kvz_g_dct_4[0][1] * src[0] + kvz_g_dct_4[2][1] * src[2 * line];

    // Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector
    dst[0] = (int_16)CLIP((int_32)-32768, (int_32)32767, (int_32)((e[0] + o[0] + add) >> shift));
    dst[1] = (int_16)CLIP((int_32)-32768, (int_32)32767, (int_32)((e[1] + o[1] + add) >> shift));
    dst[2] = (int_16)CLIP((int_32)-32768, (int_32)32767, (int_32)((e[1] - o[1] + add) >> shift));
    dst[3] = (int_16)CLIP((int_32)-32768, (int_32)32767, (int_32)((e[0] - o[0] + add) >> shift));

    src++;
    dst += 4;
  }
}

static void partial_butterfly_inverse_8_generic(const int_16 *src, int_16 *dst, int_32 shift) {
  int_32 j, k;
  int_32 e[4], o[4];
  int_32 ee[2], eo[2];
  int_32 add = 1 << (shift - 1);
  const int_32 line = 8;

  for (j = 0; j < line; j++) {
    // Utilizing symmetry properties to the maximum to minimize the number of multiplications
    for (k = 0; k < 4; k++) {
      o[k] = kvz_g_dct_8[1][k] * src[line] + kvz_g_dct_8[3][k] * src[3 * line] + kvz_g_dct_8[5][k] * src[5 * line] + kvz_g_dct_8[7][k] * src[7 * line];
    }

    eo[0] = kvz_g_dct_8[2][0] * src[2 * line] + kvz_g_dct_8[6][0] * src[6 * line];
    eo[1] = kvz_g_dct_8[2][1] * src[2 * line] + kvz_g_dct_8[6][1] * src[6 * line];
    ee[0] = kvz_g_dct_8[0][0] * src[0] + kvz_g_dct_8[4][0] * src[4 * line];
    ee[1] = kvz_g_dct_8[0][1] * src[0] + kvz_g_dct_8[4][1] * src[4 * line];

    // Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector
    e[0] = ee[0] + eo[0];
    e[3] = ee[0] - eo[0];
    e[1] = ee[1] + eo[1];
    e[2] = ee[1] - eo[1];

    for (k = 0; k < 4; k++) {
      dst[k] = (int_16)MAX((int_32)-32768, MIN((int_32)32767, (int_32)((e[k] + o[k] + add) >> shift)));
      dst[k + 4] = (int_16)MAX((int_32)-32768, MIN((int_32)32767, (int_32)((e[3 - k] - o[3 - k] + add) >> shift)));
    }
    src++;
    dst += 8;
  }
}

static void partial_butterfly_inverse_16_generic(const int_16 *src, int_16 *dst, int_32 shift) {
  int_32 j, k;
  int_32 e[8], o[8];
  int_32 ee[4], eo[4];
  int_32 eee[2], eeo[2];
  int_32 add = 1 << (shift - 1);
  const int_32 line = 16;

  for (j = 0; j < line; j++) {
    // Utilizing symmetry properties to the maximum to minimize the number of multiplications
    for (k = 0; k < 8; k++) {
      o[k] = kvz_g_dct_16[1][k] * src[line] + kvz_g_dct_16[3][k] * src[3 * line] + kvz_g_dct_16[5][k] * src[5 * line] + kvz_g_dct_16[7][k] * src[7 * line] + kvz_g_dct_16[9][k] * src[9 * line] +
             kvz_g_dct_16[11][k] * src[11 * line] + kvz_g_dct_16[13][k] * src[13 * line] + kvz_g_dct_16[15][k] * src[15 * line];
    }
    for (k = 0; k < 4; k++) {
      eo[k] = kvz_g_dct_16[2][k] * src[2 * line] + kvz_g_dct_16[6][k] * src[6 * line] + kvz_g_dct_16[10][k] * src[10 * line] + kvz_g_dct_16[14][k] * src[14 * line];
    }
    eeo[0] = kvz_g_dct_16[4][0] * src[4 * line] + kvz_g_dct_16[12][0] * src[12 * line];
    eee[0] = kvz_g_dct_16[0][0] * src[0] + kvz_g_dct_16[8][0] * src[8 * line];
    eeo[1] = kvz_g_dct_16[4][1] * src[4 * line] + kvz_g_dct_16[12][1] * src[12 * line];
    eee[1] = kvz_g_dct_16[0][1] * src[0] + kvz_g_dct_16[8][1] * src[8 * line];

    // Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector
    for (k = 0; k < 2; k++) {
      ee[k] = eee[k] + eeo[k];
      ee[k + 2] = eee[1 - k] - eeo[1 - k];
    }
    for (k = 0; k < 4; k++) {
      e[k] = ee[k] + eo[k];
      e[k + 4] = ee[3 - k] - eo[3 - k];
    }

    for (k = 0; k < 8; k++) {
      dst[k] = (int_16)MAX((int_32)-32768, MIN((int_32)32767, (int_32)((e[k] + o[k] + add) >> shift)));
      dst[k + 8] = (int_16)MAX((int_32)-32768, MIN((int_32)32767, (int_32)((e[7 - k] - o[7 - k] + add) >> shift)));
    }
    src++;
    dst += 16;
  }
}

static void partial_butterfly_inverse_32_generic(const int_16 *src, int_16 *dst, int_32 shift) {
  int_32 j, k;
  int_32 e[16], o[16];
  int_32 ee[8], eo[8];
  int_32 eee[4], eeo[4];
  int_32 eeee[2], eeeo[2];
  int_32 add = 1 << (shift - 1);
  const int_32 line = 32;

  for (j = 0; j < line; j++) {
    // Utilizing symmetry properties to the maximum to minimize the number of multiplications
    for (k = 0; k < 16; k++) {
      o[k] = kvz_g_dct_32[1][k] * src[line] + kvz_g_dct_32[3][k] * src[3 * line] + kvz_g_dct_32[5][k] * src[5 * line] + kvz_g_dct_32[7][k] * src[7 * line] + kvz_g_dct_32[9][k] * src[9 * line] +
             kvz_g_dct_32[11][k] * src[11 * line] + kvz_g_dct_32[13][k] * src[13 * line] + kvz_g_dct_32[15][k] * src[15 * line] + kvz_g_dct_32[17][k] * src[17 * line] +
             kvz_g_dct_32[19][k] * src[19 * line] + kvz_g_dct_32[21][k] * src[21 * line] + kvz_g_dct_32[23][k] * src[23 * line] + kvz_g_dct_32[25][k] * src[25 * line] +
             kvz_g_dct_32[27][k] * src[27 * line] + kvz_g_dct_32[29][k] * src[29 * line] + kvz_g_dct_32[31][k] * src[31 * line];
    }
    for (k = 0; k < 8; k++) {
      eo[k] = kvz_g_dct_32[2][k] * src[2 * line] + kvz_g_dct_32[6][k] * src[6 * line] + kvz_g_dct_32[10][k] * src[10 * line] + kvz_g_dct_32[14][k] * src[14 * line] +
              kvz_g_dct_32[18][k] * src[18 * line] + kvz_g_dct_32[22][k] * src[22 * line] + kvz_g_dct_32[26][k] * src[26 * line] + kvz_g_dct_32[30][k] * src[30 * line];
    }
    for (k = 0; k < 4; k++) {
      eeo[k] = kvz_g_dct_32[4][k] * src[4 * line] + kvz_g_dct_32[12][k] * src[12 * line] + kvz_g_dct_32[20][k] * src[20 * line] + kvz_g_dct_32[28][k] * src[28 * line];
    }
    eeeo[0] = kvz_g_dct_32[8][0] * src[8 * line] + kvz_g_dct_32[24][0] * src[24 * line];
    eeeo[1] = kvz_g_dct_32[8][1] * src[8 * line] + kvz_g_dct_32[24][1] * src[24 * line];
    eeee[0] = kvz_g_dct_32[0][0] * src[0] + kvz_g_dct_32[16][0] * src[16 * line];
    eeee[1] = kvz_g_dct_32[0][1] * src[0] + kvz_g_dct_32[16][1] * src[16 * line];

    // Combining even and odd terms at each hierarchy levels to calculate the final spatial domain vector
    eee[0] = eeee[0] + eeeo[0];
    eee[3] = eeee[0] - eeeo[0];
    eee[1] = eeee[1] + eeeo[1];
    eee[2] = eeee[1] - eeeo[1];
    for (k = 0; k < 4; k++) {
      ee[k] = eee[k] + eeo[k];
      ee[k + 4] = eee[3 - k] - eeo[3 - k];
    }
    for (k = 0; k < 8; k++) {
      e[k] = ee[k] + eo[k];
      e[k + 8] = ee[7 - k] - eo[7 - k];
    }

    for (k = 0; k < 16; k++) {
      dst[k] = (int_16)MAX((int_32)-32768, MIN((int_32)32767, (int_32)((e[k] + o[k] + add) >> shift)));
      dst[k + 16] = (int_16)MAX((int_32)-32768, MIN((int_32)32767, (int_32)((e[15 - k] - o[15 - k] + add) >> shift)));
    }
    src++;
    dst += 32;
  }
}

void it2d_nxn_generic(int_16 *input, int_16 *output, two_bit depth, two_bit color) {
  const int size = (color && depth != 3) ? (two_bit)(depth + 1) : (two_bit)depth;
  const int width = (LCU_WIDTH / 2) >> size;

  int_16 tmp[32 * 32];

  int_32 shift_1st = 7;
  int_32 shift_2nd = 12 - (8 - 8);

  switch (width) {
  case 32:
    partial_butterfly_inverse_32_generic(input, tmp, shift_1st);
    partial_butterfly_inverse_32_generic(tmp, output, shift_2nd);
    break;
  case 16:
    partial_butterfly_inverse_16_generic(input, tmp, shift_1st);
    partial_butterfly_inverse_16_generic(tmp, output, shift_2nd);
    break;
  case 8:
    partial_butterfly_inverse_8_generic(input, tmp, shift_1st);
    partial_butterfly_inverse_8_generic(tmp, output, shift_2nd);
    break;
  case 4:
    if (color) {
      partial_butterfly_inverse_4_generic(input, tmp, shift_1st);
      partial_butterfly_inverse_4_generic(tmp, output, shift_2nd);
    } else {
      fast_inverse_dst_4_generic(input, tmp, shift_1st);
      fast_inverse_dst_4_generic(tmp, output, shift_2nd);
    }
    break;
  default:
    std::cout << "ERROR Transform Block Size" << std::endl;
    break;
  }
}

class Input {
public:
  Input() { reset(); }

  void operator<<(const int_16 &rhs) {
    src[w_id].set_slc(slc * 16, rhs);

    slc++;
    if (!slc)
      w_id++;
  }

  ac_int< 16 * 32, false > get() { return src[r_id++]; }

  bool empty() { return r_id == w_id; }
  void reset() { slc = r_id = w_id = 0; }

private:
  ac_int< 5, false > slc;
  ac_int< 6, false > r_id;
  ac_int< 6, false > w_id;
  ac_int< 16 * 32, false > src[64];
};

class Output {
public:
  Output() { reset(); }

  void operator<<(const ac_int< 16 * 32, false > &rhs) { src[w_id++] = rhs; }

  int_16 get() {
    int_16 s = src[r_id].slc< 16 >(16 * slc++);
    if (!slc)
      r_id++;

    return s;
  }

  bool empty() { return r_id == w_id; }
  void reset() { slc = r_id = w_id = 0; }

private:
  ac_int< 5, false > slc;
  ac_int< 6, false > r_id;
  ac_int< 6, false > w_id;
  ac_int< 16 * 32, false > src[64];
};

void it2d(coeff_port_t &coeff_in, coeff_port_t &coeff_out, int_16 trans_push[SIZE_MULT * MAX_WIDTH][MAX_WIDTH], int_16 trans_pull[SIZE_MULT * MAX_WIDTH][MAX_WIDTH]);

CCS_MAIN(int argc, char *argv[]) {
  static coeff_port_t coeff_in;
  static coeff_port_t coeff_out;

  static int_16 trans[SIZE_MULT * MAX_WIDTH][MAX_WIDTH];

  int_16 src[32 * 32];
  int_16 ref_dst[32 * 32];
  int_16 duv_dst[32 * 32];
  one_bit simulation_successful = 1;

  bool print = false;

  int i;
  for (i = 0; i < 32; ++i) {
    srand(i);

    // Random Input
    for (uint_6 y = 0; y < 32; y++) {
      for (uint_6 x = 0; x < 32; x++) {
        src[y * 32 + x] = int_16((rand() % 65535) - 32768);
      }
    }

    uint_4 depth = 0, color = 0;
    for (depth = 0; depth < 4; ++depth) {
      for (color = 0; color < 3; ++color) {
        const int size = (color && depth != 3) ? (two_bit)(depth + 1) : (two_bit)depth;
        const int width = (LCU_WIDTH / 2) >> size;

        Input slice_i;
        Output slice_o;
        cout << "Depth: " << depth << " Color: " << color << " Width: " << width << " Rand: " << i << endl;

        for (int y = 0; y < width; y++) {
          for (int x = 0; x < width; x++) {
            slice_i << src[x * width + y];
          }
        }

        if (print) {
          std::cout << "SRC:\n";
          for (int y = 0; y < width; y++) {
            for (int x = 0; x < width; x++) {
              std::cout << src[y * width + x] << ", ";
            }
            std::cout << std::endl;
          }
        }

        // Add zeros for 4x4 blocks
        if (size == 3) {
          for (int x = 0; x < 16; x++)
            slice_i << 0;
        }

        uint_8 conf = 0;
        conf.set_slc(4, two_bit(depth));
        conf.set_slc(6, two_bit(color));

        coeff_in.write(conf);
        while (!slice_i.empty()) {
          coeff_in.write(slice_i.get());
        }

        it2d_nxn_generic(src, ref_dst, depth, color);
        CCS_DESIGN(it2d)
        (coeff_in, coeff_out, trans, trans);

        coeff_out.read();                // Clear config
        while (coeff_out.available(1)) { // Read output
          slice_o << coeff_out.read();
        }

        for (int y = 0; y < width; ++y) {
          for (int x = 0; x < width; ++x) {
            duv_dst[y * width + x] = slice_o.get();
          }
        }

        for (int y = 0; y < width; ++y) {
          for (int x = 0; x < width; ++x) {
            if (duv_dst[y * width + x] != ref_dst[y * width + x])
              simulation_successful = 0;
          }
        }

        if (print) {
          std::cout << "REF: \n";
          for (int y = 0; y < width; y++) {
            for (int x = 0; x < width; x++) {
              std::cout << ref_dst[y * width + x] << ", ";
            }
            std::cout << std::endl;
          }
          std::cout << "DUV: \n";
          for (int y = 0; y < width; y++) {
            for (int x = 0; x < width; x++) {
              std::cout << duv_dst[y * width + x] << ", ";
            }
            std::cout << std::endl;
          }
        }

        // Remove extra zeros from 4x4 blocks
        if (size == 3) {
          for (int x = 0; x < 16; x++)
            slice_o.get();
        }

        if (!slice_o.empty()) {
          simulation_successful = 0;
          std::cout << "ERROR Output channel not empty" << std::endl;
        }

        if (!simulation_successful)
          break;
      }
      if (!simulation_successful)
        break;
    }
    if (!simulation_successful)
      break;
  }

  if (simulation_successful == 1) {
    cout << "## SIMULATION PASSED ##" << endl;
  } else {
    cout << "## SIMULATION FAILED: srand(" << i << ") ##" << endl;
  }

  CCS_RETURN(0);
}
