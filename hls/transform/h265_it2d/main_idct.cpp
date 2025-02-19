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
#include <ac_dont_touch.h>

// Transform matrices
static const int_8 kvz_idct_4[4][2][2] = {{
                                              {83, 36},
                                              {36, -83},
                                          },

                                          {
                                              {64, 64},
                                              {64, -64},
                                          },
                                          {
                                              {64, 64},
                                              {64, -64},
                                          },
                                          {
                                              {64, 64},
                                              {64, -64},
                                          }};

static const int_8 kvz_idct_8[4][4][4] = {{
                                              {89, 75, 50, 18},
                                              {75, -18, -89, -50},
                                              {50, -89, 18, 75},
                                              {18, -50, 75, -89},
                                          },
                                          {
                                              {83, 36, 0, 0},
                                              {36, -83, 0, 0},
                                              {0, 0, 83, 36},
                                              {0, 0, 36, -83},
                                          },
                                          {
                                              {64, 64, 0, 0},
                                              {64, -64, 0, 0},
                                              {0, 0, 64, 64},
                                              {0, 0, 64, -64},
                                          },
                                          {
                                              {64, 64, 0, 0},
                                              {64, -64, 0, 0},
                                              {0, 0, 64, 64},
                                              {0, 0, 64, -64},
                                          }};

static const int_8 kvz_idct_16[4][8][8] = {{
                                               {90, 87, 80, 70, 57, 43, 25, 9},
                                               {87, 57, 9, -43, -80, -90, -70, -25},
                                               {80, 9, -70, -87, -25, 57, 90, 43},
                                               {70, -43, -87, 9, 90, 25, -80, -57},
                                               {57, -80, -25, 90, -9, -87, 43, 70},
                                               {43, -90, 57, 25, -87, 70, 9, -80},
                                               {25, -70, 90, -80, 43, 9, -57, 87},
                                               {9, -25, 43, -57, 70, -80, 87, -90},
                                           },
                                           {
                                               {89, 75, 50, 18, 0, 0, 0, 0},
                                               {75, -18, -89, -50, 0, 0, 0, 0},
                                               {50, -89, 18, 75, 0, 0, 0, 0},
                                               {18, -50, 75, -89, 0, 0, 0, 0},
                                               {0, 0, 0, 0, 89, 75, 50, 18},
                                               {0, 0, 0, 0, 75, -18, -89, -50},
                                               {0, 0, 0, 0, 50, -89, 18, 75},
                                               {0, 0, 0, 0, 18, -50, 75, -89},
                                           },
                                           {
                                               {83, 36, 0, 0, 0, 0, 0, 0},
                                               {36, -83, 0, 0, 0, 0, 0, 0},
                                               {0, 0, 83, 36, 0, 0, 0, 0},
                                               {0, 0, 36, -83, 0, 0, 0, 0},
                                               {0, 0, 0, 0, 83, 36, 0, 0},
                                               {0, 0, 0, 0, 36, -83, 0, 0},
                                               {0, 0, 0, 0, 0, 0, 83, 36},
                                               {0, 0, 0, 0, 0, 0, 36, -83},
                                           },
                                           {
                                               {64, 64, 0, 0, 0, 0, 0, 0},
                                               {64, -64, 0, 0, 0, 0, 0, 0},
                                               {0, 0, 64, 64, 0, 0, 0, 0},
                                               {0, 0, 64, -64, 0, 0, 0, 0},
                                               {0, 0, 0, 0, 64, 64, 0, 0},
                                               {0, 0, 0, 0, 64, -64, 0, 0},
                                               {0, 0, 0, 0, 0, 0, 64, 64},
                                               {0, 0, 0, 0, 0, 0, 64, -64},
                                           }};

static const int_8 kvz_idct_32[4][16][16] = {{
                                                 {90, 90, 88, 85, 82, 78, 73, 67, 61, 54, 46, 38, 31, 22, 13, 4},
                                                 {90, 82, 67, 46, 22, -4, -31, -54, -73, -85, -90, -88, -78, -61, -38, -13},
                                                 {88, 67, 31, -13, -54, -82, -90, -78, -46, -4, 38, 73, 90, 85, 61, 22},
                                                 {85, 46, -13, -67, -90, -73, -22, 38, 82, 88, 54, -4, -61, -90, -78, -31},
                                                 {82, 22, -54, -90, -61, 13, 78, 85, 31, -46, -90, -67, 4, 73, 88, 38},
                                                 {78, -4, -82, -73, 13, 85, 67, -22, -88, -61, 31, 90, 54, -38, -90, -46},
                                                 {73, -31, -90, -22, 78, 67, -38, -90, -13, 82, 61, -46, -88, -4, 85, 54},
                                                 {67, -54, -78, 38, 85, -22, -90, 4, 90, 13, -88, -31, 82, 46, -73, -61},
                                                 {61, -73, -46, 82, 31, -88, -13, 90, -4, -90, 22, 85, -38, -78, 54, 67},
                                                 {54, -85, -4, 88, -46, -61, 82, 13, -90, 38, 67, -78, -22, 90, -31, -73},
                                                 {46, -90, 38, 54, -90, 31, 61, -88, 22, 67, -85, 13, 73, -82, 4, 78},
                                                 {38, -88, 73, -4, -67, 90, -46, -31, 85, -78, 13, 61, -90, 54, 22, -82},
                                                 {31, -78, 90, -61, 4, 54, -88, 82, -38, -22, 73, -90, 67, -13, -46, 85},
                                                 {22, -61, 85, -90, 73, -38, -4, 46, -78, 90, -82, 54, -13, -31, 67, -88},
                                                 {13, -38, 61, -78, 88, -90, 85, -73, 54, -31, 4, 22, -46, 67, -82, 90},
                                                 {4, -13, 22, -31, 38, -46, 54, -61, 67, -73, 78, -82, 85, -88, 90, -90},
                                             },
                                             {
                                                 {90, 87, 80, 70, 57, 43, 25, 9, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {87, 57, 9, -43, -80, -90, -70, -25, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {80, 9, -70, -87, -25, 57, 90, 43, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {70, -43, -87, 9, 90, 25, -80, -57, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {57, -80, -25, 90, -9, -87, 43, 70, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {43, -90, 57, 25, -87, 70, 9, -80, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {25, -70, 90, -80, 43, 9, -57, 87, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {9, -25, 43, -57, 70, -80, 87, -90, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 90, 87, 80, 70, 57, 43, 25, 9},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 87, 57, 9, -43, -80, -90, -70, -25},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 80, 9, -70, -87, -25, 57, 90, 43},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 70, -43, -87, 9, 90, 25, -80, -57},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 57, -80, -25, 90, -9, -87, 43, 70},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 43, -90, 57, 25, -87, 70, 9, -80},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 25, -70, 90, -80, 43, 9, -57, 87},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 9, -25, 43, -57, 70, -80, 87, -90},
                                             },
                                             {
                                                 {89, 75, 50, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {75, -18, -89, -50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {50, -89, 18, 75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {18, -50, 75, -89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 89, 75, 50, 18, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 75, -18, -89, -50, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 50, -89, 18, 75, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 18, -50, 75, -89, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 89, 75, 50, 18, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 75, -18, -89, -50, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 50, -89, 18, 75, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 18, -50, 75, -89, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 75, 50, 18},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 75, -18, -89, -50},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, -89, 18, 75},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, -50, 75, -89},
                                             },
                                             {
                                                 {83, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {36, -83, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 83, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 36, -83, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 83, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 36, -83, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 83, 36, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 36, -83, 0, 0, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 83, 36, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 36, -83, 0, 0, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 36, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 36, -83, 0, 0, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 36, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 36, -83, 0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 36},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 36, -83},
                                             }};

template < int w_i, int w_o > ac_int< w_o, true > clip_signed(ac_int< w_i, true > a) {
  static const ac_int< w_o, true > int_min = 1 << (w_o - 1);
  static const ac_int< w_o, true > int_max = ac_int< w_o, false >(-1) >> 1;

  ac_int< w_o, true > r = a;

  one_bit MSB = a[w_i - 1];
  ac_int< w_i - w_o - 1, true > top_bits = a >> (w_o - 1);

  if (MSB) {
    // Signed
    if (top_bits != -1)
      r = int_min;
  } else {
    // Unsigned
    if (top_bits)
      r = int_max;
  }

  return r;
}

template < unsigned width, bool sign, typename T, typename S > ac_int< width, sign > shift_round(const T &a, S i) {
  ac_int< width + 1, sign > r = a >> S(i - 1);
  return (r + 1) >> 1;
}

int_16 get_src(int_16 src) {
  // return ac::dont_touch<int_16>(src, ac::dt_until_dsp);
  return src;
}

int_8 get_lut(int_8 lut) {
  // return ac::dont_touch<int_8>(lut, ac::dt_until_dsp);
  return lut;
}

void main_idct(dct_port_t &input_port, coeff_port_t &output_1st, coeff_port_t &output_2nd) {
  int_26 o[16], e[16];
  int_25 eo[8], ee[8];
  int_24 eeo[8], eee[4];

  int_26 _e[16];
  int_16 src[32];

#pragma hls_pipeline_init_interval 1
  for (int y = 0; y < MAX_WIDTH; ++y) {
    // Read configuration data
    const slice_t input = input_port.read();
    coeff_slice_t output[4], slice;

    // Get configuration bits
    const two_bit size = input.slc< 2 >(MAX_WIDTH * 16);
    const one_bit dual = input.slc< 2 >(MAX_WIDTH * 16 + 2);
    const uint_4 shift = input.slc< 4 >(MAX_WIDTH * 16 + 3);

    // Select output port
    coeff_port_t *const output_port = input[MAX_WIDTH * 16 + 7] ? &output_1st : &output_2nd;

    static const int slc_pos[4][32] = {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31},
                                       {16, 1, 2, 3, 4, 5, 6, 7, 0, 9, 10, 11, 12, 13, 14, 15, 24, 17, 18, 19, 20, 21, 22, 23, 8, 25, 26, 27, 28, 29, 30, 31},
                                       {24, 1, 2, 3, 0, 5, 6, 7, 16, 9, 10, 11, 4, 13, 14, 15, 28, 17, 18, 19, 8, 21, 22, 23, 20, 25, 26, 27, 12, 29, 30, 31},
                                       {28, 1, 0, 3, 16, 5, 2, 7, 24, 9, 4, 11, 18, 13, 6, 15, 30, 17, 8, 19, 20, 21, 10, 23, 26, 25, 12, 27, 22, 29, 14, 31}};

  sort_even_terms:
#pragma hls_unroll yes
    for (uint_6 i = 0; i < MAX_WIDTH; ++i) {
      int_16 slc[4];
      slc[0] = input.slc< 16 >(slc_pos[0][i] * 16);
      slc[1] = input.slc< 16 >(slc_pos[1][i] * 16);
      slc[2] = input.slc< 16 >(slc_pos[2][i] * 16);
      slc[3] = input.slc< 16 >(slc_pos[3][i] * 16);

      // Odd terms are constant
      src[i] = i[0] ? (int_16)input.slc< 16 >(i * 16) : (int_16)slc[size];
    }

  Mult32x32:
#pragma hls_unroll yes
    for (int i = 0; i < 16; ++i) {
      int_26 temp = 0; // Max: 30211789, Min: -30212096
#if 0
            // Unoptimized version for reference.
            for (int j = 0; j < 16; ++j) {
                temp += kvz_idct_32[size][i][j] * src[2 * j + 1];
            }
#endif
#ifdef DSP_32
#ifndef OPTIMIZE_DSP_USAGE
      temp = ac::dont_touch< int_26 >(get_lut(kvz_idct_32[size][i][0]) * get_src(src[1]) + get_lut(kvz_idct_32[size][i][1]) * get_src(src[3]), ac::dt_until_dsp) +
             ac::dont_touch< int_26 >(get_lut(kvz_idct_32[size][i][2]) * get_src(src[5]) + get_lut(kvz_idct_32[size][i][3]) * get_src(src[7]), ac::dt_until_dsp) +
             ac::dont_touch< int_26 >(get_lut(kvz_idct_32[size][i][4]) * get_src(src[9]) + get_lut(kvz_idct_32[size][i][5]) * get_src(src[11]), ac::dt_until_dsp) +
             ac::dont_touch< int_26 >(get_lut(kvz_idct_32[size][i][6]) * get_src(src[13]) + get_lut(kvz_idct_32[size][i][7]) * get_src(src[15]), ac::dt_until_dsp) +
             ac::dont_touch< int_26 >(get_lut(kvz_idct_32[size][i][8]) * get_src(src[17]) + get_lut(kvz_idct_32[size][i][9]) * get_src(src[19]), ac::dt_until_dsp) +
             ac::dont_touch< int_26 >(get_lut(kvz_idct_32[size][i][10]) * get_src(src[21]) + get_lut(kvz_idct_32[size][i][11]) * get_src(src[23]), ac::dt_until_dsp) +
             ac::dont_touch< int_26 >(get_lut(kvz_idct_32[size][i][12]) * get_src(src[25]) + get_lut(kvz_idct_32[size][i][13]) * get_src(src[27]), ac::dt_until_dsp) +
             ac::dont_touch< int_26 >(get_lut(kvz_idct_32[size][i][14]) * get_src(src[29]) + get_lut(kvz_idct_32[size][i][15]) * get_src(src[31]), ac::dt_until_dsp);

#else // Optimized version
#pragma hls_unroll yes
      for (int j = 0; j < 16; j += 4) {
        if ((i < 8 && j >= 8) || (i >= 8 && j < 8)) {
          // Don't use DSPs if only one multiplier is nonzero
          one_bit is_zero = size != 0;
          if (is_zero == 0) {
            temp += kvz_idct_32[0][i][j] * src[2 * j + 1] + kvz_idct_32[0][i][j + 1] * src[2 * j + 3] + kvz_idct_32[0][i][j + 2] * src[2 * j + 5] + kvz_idct_32[0][i][j + 3] * src[2 * j + 7];
          }
        } else {
          // For others use DSPs
          temp += ac::dont_touch< int_26 >(get_lut(kvz_idct_32[size][i][j]) * get_src(src[2 * j + 1]) + get_lut(kvz_idct_32[size][i][j + 1]) * get_src(src[2 * j + 3]), ac::dt_until_dsp) +
                  ac::dont_touch< int_26 >(get_lut(kvz_idct_32[size][i][j + 2]) * get_src(src[2 * j + 5]) + get_lut(kvz_idct_32[size][i][j + 3]) * get_src(src[2 * j + 7]), ac::dt_until_dsp);
        }
      }
#endif
#else // No DSPs
#pragma hls_unroll yes
      for (int j = 0; j < 16; ++j) {
        int_24 d[4];
        d[0] = kvz_idct_32[0][i][j] * src[2 * j + 1];
        d[1] = kvz_idct_32[1][i][j] * src[2 * j + 1];
        d[2] = kvz_idct_32[2][i][j] * src[2 * j + 1];
        d[3] = kvz_idct_32[3][i][j] * src[2 * j + 1];

        temp += d[size];
      }
#endif
      o[i] = temp;
    }

  Mult16x16:
#pragma hls_unroll yes
    for (int i = 0; i < 8; ++i) {
      int_25 temp = 0; // Max: 15105895, Min: -15106048
#if 0
            // Unoptimized version for reference.
            for (int j = 0; j < 8; ++j) {
                temp += kvz_idct_16[size][i][j] * src[4 * j + 2];
            }
#endif
#ifdef DSP_16
#ifndef OPTIMIZE_DSP_USAGE
      temp = ac::dont_touch< int_26 >(get_lut(kvz_idct_16[size][i][0]) * get_src(src[2]) + get_lut(kvz_idct_16[size][i][1]) * get_src(src[6]), ac::dt_until_dsp) +
             ac::dont_touch< int_26 >(get_lut(kvz_idct_16[size][i][2]) * get_src(src[10]) + get_lut(kvz_idct_16[size][i][3]) * get_src(src[14]), ac::dt_until_dsp) +
             ac::dont_touch< int_26 >(get_lut(kvz_idct_16[size][i][4]) * get_src(src[18]) + get_lut(kvz_idct_16[size][i][5]) * get_src(src[22]), ac::dt_until_dsp) +
             ac::dont_touch< int_26 >(get_lut(kvz_idct_16[size][i][6]) * get_src(src[26]) + get_lut(kvz_idct_16[size][i][7]) * get_src(src[30]), ac::dt_until_dsp);

#else // Optimized version
#pragma hls_unroll yes
      for (int j = 0; j < 8; j += 4) {
        if ((i < 4 && j >= 4) || (i >= 4 && j < 4)) {
          // Don't use DSPs if only one multiplier is nonzero
          one_bit is_zero = size != 0;
          if (is_zero == 0) {
            temp += kvz_idct_16[0][i][j] * src[4 * j + 2] + kvz_idct_16[0][i][j + 1] * src[4 * j + 6] + kvz_idct_16[0][i][j + 2] * src[4 * j + 10] + kvz_idct_16[0][i][j + 3] * src[4 * j + 14];
          }
        } else {
          // For others use DSPs
          temp += ac::dont_touch< int_26 >(get_lut(kvz_idct_16[size][i][j]) * get_src(src[4 * j + 2]) + get_lut(kvz_idct_16[size][i][j + 1]) * get_src(src[4 * j + 6]), ac::dt_until_dsp) +
                  ac::dont_touch< int_26 >(get_lut(kvz_idct_16[size][i][j + 2]) * get_src(src[4 * j + 10]) + get_lut(kvz_idct_16[size][i][j + 3]) * get_src(src[4 * j + 14]), ac::dt_until_dsp);
        }
      }
#endif
#else // No DSPs
#pragma hls_unroll yes
      for (int j = 0; j < 8; ++j) {
        int_24 d[4];
        d[0] = kvz_idct_16[0][i][j] * src[4 * j + 2];
        d[1] = kvz_idct_16[1][i][j] * src[4 * j + 2];
        d[2] = kvz_idct_16[2][i][j] * src[4 * j + 2];
        d[3] = kvz_idct_16[3][i][j] * src[4 * j + 2];

        temp += d[size];
      }
#endif
      eo[i] = temp;
    }

  Mult8x8:
#pragma hls_unroll yes
    for (int i = 0; i < 4; ++i) {
      int_24 temp = 0;
#if 0
            // Unoptimized version for reference.
            for (int j = 0; j < 4; ++j) {
                temp += kvz_idct_8[size][i][j] * src[8 * j + 4];
            }
#endif
#ifdef DSP_8
      // No DSP omtimization as one DPS has 4 multipliers on newer FPGAs
      temp = ac::dont_touch< int_26 >(get_lut(kvz_idct_8[size][i][0]) * get_src(src[4]) + get_lut(kvz_idct_8[size][i][1]) * get_src(src[12]), ac::dt_until_dsp) +
             ac::dont_touch< int_26 >(get_lut(kvz_idct_8[size][i][2]) * get_src(src[20]) + get_lut(kvz_idct_8[size][i][3]) * get_src(src[28]), ac::dt_until_dsp);

#else
#pragma hls_unroll yes
      for (int j = 0; j < 4; ++j) {
        int_24 d[4];
        d[0] = kvz_idct_8[0][i][j] * src[8 * j + 4];
        d[1] = kvz_idct_8[1][i][j] * src[8 * j + 4];
        d[2] = kvz_idct_8[2][i][j] * src[8 * j + 4];
        d[3] = kvz_idct_8[3][i][j] * src[8 * j + 4];

        temp += d[size];
      }
#endif
      eeo[i] = temp;
    }

#ifdef DSP_4
    eeo[4] = ac::dont_touch< int_26 >(get_lut(kvz_idct_4[size][0][0]) * get_src(src[8]) + get_lut(kvz_idct_4[size][0][1]) * get_src(src[24]), ac::dt_until_dsp);
    eeo[5] = ac::dont_touch< int_26 >(get_lut(kvz_idct_4[size][1][0]) * get_src(src[8]) + get_lut(kvz_idct_4[size][1][1]) * get_src(src[24]), ac::dt_until_dsp);
#else
    eeo[4] = kvz_idct_4[size][0][0] * src[8] + kvz_idct_4[size][0][1] * src[24];
    eeo[5] = kvz_idct_4[size][1][0] * src[8] + kvz_idct_4[size][1][1] * src[24];
#endif
    // Even terms
    eeo[6] = 64 * src[0] + 64 * src[16];
    eeo[7] = 64 * src[0] - 64 * src[16];

    // Calc EEE
    eee[0] = eeo[6] + eeo[4];
    eee[3] = eeo[6] - eeo[4];
    eee[1] = eeo[7] + eeo[5];
    eee[2] = eeo[7] - eeo[5];

  calc_ee:
#pragma hls_unroll yes
    for (int i = 0; i < 4; ++i) {
      int_24 e0 = size[0] ? eeo[4 + i] : eee[i];
      int_24 e1 = size[0] ? eeo[7 - i] : eee[3 - i];

      ee[i] = e0 + eeo[i];
      ee[i + 4] = e1 - eeo[3 - i];
    }

  calc_e:
#pragma hls_unroll yes
    for (int i = 0; i < 8; ++i) {
      static const int o_pos[8] = {0, 1, 4, 5, 6, 7, 2, 3};

      int_25 e0 = size[1] ? (int_25)eeo[i] : (int_25)ee[i];
      int_25 e1 = size[1] ? (int_25)eeo[7 - i] : (int_25)ee[7 - i];
      int_25 o0 = size[0] ? (int_25)eo[o_pos[i]] : (int_25)eo[i];
      int_25 o1 = size[0] ? (int_25)eo[o_pos[7 - i]] : (int_25)eo[7 - i];

      _e[0 + i] = e0 + o0;
      _e[8 + i] = e1 - o1;
    }

    static const int e_pos[3][16] = {
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, {0, 1, 6, 7, 8, 9, 14, 15, 2, 3, 4, 5, 10, 11, 12, 13}, {0, 1, 14, 15, 2, 3, 12, 13, 4, 5, 10, 11, 6, 7, 8, 9}};

  sort_e:
#pragma hls_unroll yes
    for (int i = 0; i < MAX_WIDTH / 2; ++i) {
      int_26 tmp[4];
      tmp[0] = _e[e_pos[0][i]];
      tmp[1] = _e[e_pos[1][i]];
      tmp[2] = _e[e_pos[2][i]];
      tmp[3] = (i < 8) ? (int_26)eo[i] : (int_26)eeo[i - 8];

      e[i] = tmp[size];
    }

  combine_e_o:
#pragma hls_unroll yes
    for (int i = 0; i < MAX_WIDTH / 2; ++i) {
      int_21 val0 = shift_round< 21, true >((e[i] + o[i]) >> 6, uint_3(shift - 6));
      int_21 val1 = shift_round< 21, true >((e[i] - o[i]) >> 6, uint_3(shift - 6));

      slice.set_slc(16 * i, clip_signed< 21, 16 >(val0));
      slice.set_slc(16 * (MAX_WIDTH - 1 - i), clip_signed< 21, 16 >(val1));
    }

    const static int out_slc[4][32] = {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31},
                                       {0, 1, 2, 3, 4, 5, 6, 7, 24, 25, 26, 27, 28, 29, 30, 31, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23},
                                       {0, 1, 2, 3, 28, 29, 30, 31, 4, 5, 6, 7, 24, 25, 26, 27, 8, 9, 10, 11, 20, 21, 22, 23, 12, 13, 14, 15, 16, 17, 18, 19},
                                       {0, 1, 30, 31, 2, 3, 28, 29, 4, 5, 26, 27, 6, 7, 24, 25, 8, 9, 22, 23, 10, 11, 20, 21, 12, 13, 18, 19, 14, 15, 16, 17}};

  sort_output:
#pragma hls_unroll yes
    for (int j = 0; j < MAX_WIDTH; ++j) {
      output[0].set_slc(16 * j, slice.slc< 16 >(16 * out_slc[0][j]));
      output[1].set_slc(16 * j, slice.slc< 16 >(16 * out_slc[1][j]));
      output[2].set_slc(16 * j, slice.slc< 16 >(16 * out_slc[2][j]));
      output[3].set_slc(16 * j, slice.slc< 16 >(16 * out_slc[3][j]));
    }

    output_port->write(output[size]);
    if (y == loops[dual][size])
      break;
  }
}
