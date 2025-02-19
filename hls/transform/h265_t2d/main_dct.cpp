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
static const int_8 kvz_dct_8[4][4][4] = {{
                                             {89, 75, 50, 18},
                                             {75, -18, -89, -50},
                                             {50, -89, 18, 75},
                                             {18, -50, 75, -89},
                                         },
                                         {
                                             {89, 75, 50, 18},
                                             {75, -18, -89, -50},
                                             {50, -89, 18, 75},
                                             {18, -50, 75, -89},
                                         },
                                         {
                                             {89, 75, 50, 18},
                                             {75, -18, -89, -50},
                                             {50, -89, 18, 75},
                                             {18, -50, 75, -89},
                                         },
                                         {
                                             {64, 64, 0, 0},
                                             {64, -64, 0, 0},
                                             {0, 0, 83, 36},
                                             {0, 0, 36, -83},
                                         }};

static const int_8 kvz_dct_16[4][8][8] = {{
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
                                              {64, 64, 0, 0, 0, 0, 0, 0},
                                              {64, -64, 0, 0, 0, 0, 0, 0},
                                              {0, 0, 83, 36, 0, 0, 0, 0},
                                              {0, 0, 36, -83, 0, 0, 0, 0},
                                              {0, 0, 0, 0, 89, 75, 50, 18},
                                              {0, 0, 0, 0, 75, -18, -89, -50},
                                              {0, 0, 0, 0, 50, -89, 18, 75},
                                              {0, 0, 0, 0, 18, -50, 75, -89},
                                          },
                                          {
                                              {64, 64, 0, 0, 0, 0, 0, 0},
                                              {64, -64, 0, 0, 0, 0, 0, 0},
                                              {0, 0, 83, 36, 0, 0, 0, 0},
                                              {0, 0, 36, -83, 0, 0, 0, 0},
                                              {0, 0, 0, 0, 64, 64, 0, 0},
                                              {0, 0, 0, 0, 64, -64, 0, 0},
                                              {0, 0, 0, 0, 0, 0, 83, 36},
                                              {0, 0, 0, 0, 0, 0, 36, -83},
                                          }};

static const int_8 kvz_dct_32[4][16][16] = {{
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
                                                {64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {64, -64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 83, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 36, -83, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 89, 75, 50, 18, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 75, -18, -89, -50, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 50, -89, 18, 75, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 18, -50, 75, -89, 0, 0, 0, 0, 0, 0, 0, 0},
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
                                                {64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {64, -64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 83, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 36, -83, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 89, 75, 50, 18, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 75, -18, -89, -50, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 50, -89, 18, 75, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 18, -50, 75, -89, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 0, 0, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 0, 0, 0, 0, 64, -64, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 36, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 36, -83, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 89, 75, 50, 18},
                                                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 75, -18, -89, -50},
                                                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, -89, 18, 75},
                                                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, -50, 75, -89},
                                            },
                                            {
                                                {64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {64, -64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 83, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 36, -83, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 64, -64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 0, 0, 83, 36, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 0, 0, 36, -83, 0, 0, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 0, 0, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 0, 0, 0, 0, 64, -64, 0, 0, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 36, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 36, -83, 0, 0, 0, 0},
                                                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 64, 0, 0},
                                                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, -64, 0, 0},
                                                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 36},
                                                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 36, -83},
                                            }};

template < unsigned width, bool sign, typename T, typename S > ac_int< width, sign > shift_round(const T &a, S i) {
  ac_int< width + 1, sign > r = a >> S(i - 1);
  return (r + 1) >> 1;
}

// Size 6
template < int N, typename T > static int_16 adder_tree_x32(const coeff_slice_t &input, T *o, T *e, int i, two_bit size) {
  static const int slc_pos[4][32] = {{0, 31, 15, 16, 7, 24, 8, 23, 3, 28, 12, 19, 4, 27, 11, 20, 1, 30, 14, 17, 6, 25, 9, 22, 2, 29, 13, 18, 5, 26, 10, 21},
                                     {0, 15, 7, 8, 3, 12, 4, 11, 1, 14, 6, 9, 2, 13, 5, 10, 16, 31, 23, 24, 19, 28, 20, 27, 17, 30, 22, 25, 18, 29, 21, 26},
                                     {0, 7, 3, 4, 1, 6, 2, 5, 8, 15, 11, 12, 9, 14, 10, 13, 16, 23, 19, 20, 17, 22, 18, 21, 24, 31, 27, 28, 25, 30, 26, 29},
                                     {0, 3, 1, 2, 4, 7, 5, 6, 8, 11, 9, 10, 12, 15, 13, 14, 16, 19, 17, 18, 20, 23, 21, 22, 24, 27, 25, 26, 28, 31, 29, 30}};

  int_16 out[4];
  out[0] = input.slc< 16 >(slc_pos[0][i] * 16);
  out[1] = input.slc< 16 >(slc_pos[1][i] * 16);
  out[2] = input.slc< 16 >(slc_pos[2][i] * 16);
  out[3] = input.slc< 16 >(slc_pos[3][i] * 16);

  return out[size];
}

// Size 4
template < int N, typename T > static T adder_tree_x16(const coeff_slice_t &input, T *o, T *e, int i, two_bit size) {
  T m0 = adder_tree_x32< N - 1 >(input, o, e, i, size);
  T m1 = adder_tree_x32< N - 1 >(input, o, e, i + (1 << (N - 1)), size);
  T e_ = m0 + m1;
  T o_ = m0 - m1;

  o[14 + (i >> N)] = o_;
  e[14 + (i >> N)] = e_;
  return e_;
}

// Size 3
template < int N, typename T > static T adder_tree_x8(const coeff_slice_t &input, T *o, T *e, int i, two_bit size) {
  T m0 = adder_tree_x16< N - 1 >(input, o, e, i, size);
  T m1 = adder_tree_x16< N - 1 >(input, o, e, i + (1 << (N - 1)), size);
  T e_ = m0 + m1;
  T o_ = m0 - m1;

  o[6 + (i >> N)] = o_;
  e[6 + (i >> N)] = e_;
  return e_;
}

// Size 2
template < int N, typename T > static T adder_tree_x4(const coeff_slice_t &input, T *o, T *e, int i, two_bit size) {
  T m0 = adder_tree_x8< N - 1 >(input, o, e, i, size);
  T m1 = adder_tree_x8< N - 1 >(input, o, e, i + (1 << (N - 1)), size);
  T e_ = m0 + m1;
  T o_ = m0 - m1;

  o[2 + (i >> N)] = o_;
  e[2 + (i >> N)] = e_;
  return e_;
}

// Size 1
template < int N, typename T > static void adder_tree_x2(const coeff_slice_t &input, T *o, T *e, int i, two_bit size) {
  T m0 = adder_tree_x4< N - 1 >(input, o, e, i, size);
  T m1 = adder_tree_x4< N - 1 >(input, o, e, i + (1 << (N - 1)), size);
  T e_ = m0 + m1;
  T o_ = m0 - m1;

  o[i >> N] = o_;
  e[i >> N] = e_;
}

// Size 0
template < typename T > void adder_tree(const coeff_slice_t &input, T *o, T *e, two_bit size) {
  adder_tree_x2< 4 >(input, o, e, 0, size);
  adder_tree_x2< 4 >(input, o, e, 1 << 4, size);
}

template < typename T > static void slice_e_o(const T *e, const T *o, two_bit size, T *o_) {
  static const int slc_o[4][32] = {
      {0, 0, 0, 1, 2, 4, 5, 3, 6, 10, 12, 8, 9, 13, 11, 7, 14, 22, 26, 18, 20, 28, 24, 16, 17, 25, 29, 21, 19, 27, 23, 15},
      {0, 0, 2, 3, 6, 8, 9, 7, 14, 18, 20, 16, 17, 21, 19, 15, 0, 0, 4, 5, 10, 12, 13, 11, 22, 26, 28, 24, 25, 29, 27, 23},
      {0, 0, 6, 7, 14, 16, 17, 15, 0, 0, 8, 9, 18, 20, 21, 19, 0, 0, 10, 11, 22, 24, 25, 23, 0, 0, 12, 13, 26, 28, 29, 27},
      {0, 0, 14, 15, 0, 0, 16, 17, 0, 0, 18, 19, 0, 0, 20, 21, 0, 0, 22, 23, 0, 0, 24, 25, 0, 0, 26, 27, 0, 0, 28, 29},
  };

  static const int slc_e[4][32] = {
      {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {6, 7, 0, 0, 0, 0, 0, 0, 8, 9, 0, 0, 0, 0, 0, 0, 10, 11, 0, 0, 0, 0, 0, 0, 12, 13, 0, 0, 0, 0, 0},
      {14, 15, 0, 0, 16, 17, 0, 0, 18, 19, 0, 0, 20, 21, 0, 0, 22, 23, 0, 0, 24, 25, 0, 0, 26, 27, 0, 0, 28, 29, 0, 0},
  };

  static const int use_e[4][32] = {
      {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
      {1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0},
  };

sort_e_o:
#pragma hls_unroll yes
  for (int i = 0; i < MAX_WIDTH; i++) {
    T out[4];

    out[0] = use_e[0][i] ? e[slc_e[0][i]] : o[slc_o[0][i]];
    out[1] = use_e[1][i] ? e[slc_e[1][i]] : o[slc_o[1][i]];
    out[2] = use_e[2][i] ? e[slc_e[2][i]] : o[slc_o[2][i]];
    out[3] = use_e[3][i] ? e[slc_e[3][i]] : o[slc_o[3][i]];

    o_[i] = out[size];
  }
}

int_18 get_o(int_20 o) {
  // return ac::dont_touch<int_18>(o.slc<18>(0),ac::dt_until_dsp);
  return o.slc< 18 >(0);
}

int_8 get_lut(int_8 lut) {
  // return ac::dont_touch<int_8>(lut,ac::dt_until_dsp);
  return lut;
}

void main_dct(dct_port_t &input_port, coeff_port_t &output_1st, coeff_port_t &output_2nd) {
  int_20 _o[30], _e[30];
  int_20 o[32];
  // int_20 o_4[4];

#pragma hls_pipeline_init_interval 1
  for (int y = 0; y < MAX_WIDTH; ++y) {
    // Read data row by row
    const slice_t input = input_port.read();
    coeff_slice_t output[4], slice;

    // Get configuration bits
    const two_bit size = input.slc< 2 >(MAX_WIDTH * 16);
    const one_bit dual = input.slc< 2 >(MAX_WIDTH * 16 + 2);
    const uint_4 shift = input.slc< 4 >(MAX_WIDTH * 16 + 3);

    // Select output port
    coeff_port_t *const output_port = input[MAX_WIDTH * 16 + 7] ? &output_1st : &output_2nd;

    // Calculate Even and Odd terms
    adder_tree(input, _o, _e, size);
    //        slice_e_o(_e, _o, size, o, o_4);
    slice_e_o(_e, _o, size, o);

    // Multiplications
    slice.set_slc(0, shift_round< 16, true >(64 * o[0] + 64 * o[1], shift));
    slice.set_slc(16, shift_round< 16, true >(64 * o[0] - 64 * o[1], shift));
    slice.set_slc(32, shift_round< 16, true >(83 * o[2] + 36 * o[3], shift));
    slice.set_slc(48, shift_round< 16, true >(36 * o[2] - 83 * o[3], shift));

  Mult8x8:
#pragma hls_unroll yes
    for (int i = 0; i < 4; ++i) {
      int_26 temp = 0;
#if 0
            // Unoptimized version for reference.
            /*for (int j = 0; j < 4; ++j) {
                temp += kvz_dct_8[size][i][j] * o[4 + j];
            }*/
            /*temp = kvz_dct_8[size][i][0] * o[4] + kvz_dct_8[size][i][1] * o[4 + 1] + 
                   kvz_dct_8[size][i][2] * o[4 + 2] + kvz_dct_8[size][i][3] * o[4 + 3];*/
            //temp = temp1 + temp2;
#endif
#ifdef DSP_8
      // No DSP omtimization as one DPS has 4 multipliers on newer FPGAs
      temp += get_lut(kvz_dct_8[size][i][0]) * get_o(o[4]) + get_lut(kvz_dct_8[size][i][1]) * get_o(o[5]);
      temp += get_lut(kvz_dct_8[size][i][2]) * get_o(o[6]) + get_lut(kvz_dct_8[size][i][3]) * get_o(o[7]);

#else // No DSPs
#pragma hls_unroll yes
      for (int j = 0; j < 4; ++j) {
        int_27 d[4];
        d[0] = kvz_dct_8[0][i][j] * o[4 + j];
        d[1] = kvz_dct_8[1][i][j] * o[4 + j];
        d[2] = kvz_dct_8[2][i][j] * o[4 + j];
        d[3] = kvz_dct_8[3][i][j] * o[4 + j];

        temp += d[size];
      }
#endif
      slice.set_slc(64 + 16 * i, shift_round< 16, true >(temp, shift));
    }

  Mult16x16:
#pragma hls_unroll yes
    for (int i = 0; i < 8; ++i) {
      int_27 temp = 0;
#if 0
            // Unoptimized version for reference.
            /*for (int j = 0; j < 8; ++j) {
                temp += kvz_dct_16[size][i][j] * o[8 + j];
            }*/
            /*temp = kvz_dct_16[size][i][0] * o[8] + kvz_dct_16[size][i][1] * o[8 + 1] +
                   kvz_dct_16[size][i][2] * o[8 + 2] + kvz_dct_16[size][i][3] * o[8 + 3] + 
                   kvz_dct_16[size][i][4] * o[8 + 4] + kvz_dct_16[size][i][5] * o[8 + 5] + 
                   kvz_dct_16[size][i][6] * o[8 + 6] + kvz_dct_16[size][i][7] * o[8 + 7] +          */  
            //temp = temp1 + temp2 + temp3 + temp4;
#endif
#ifdef DSP_16
#ifndef OPTIMIZE_DSP_USAGE

      temp += get_lut(kvz_dct_16[size][i][0]) * get_o(o[8]) + get_lut(kvz_dct_16[size][i][1]) * get_o(o[9]) + get_lut(kvz_dct_16[size][i][2]) * get_o(o[10]) +
              get_lut(kvz_dct_16[size][i][3]) * get_o(o[11]) + get_lut(kvz_dct_16[size][i][4]) * get_o(o[12]) + get_lut(kvz_dct_16[size][i][5]) * get_o(o[13]) +
              get_lut(kvz_dct_16[size][i][6]) * get_o(o[14]) + get_lut(kvz_dct_16[size][i][7]) * get_o(o[15]);

#else // Optimized version*/
#pragma hls_unroll yes
      for (int j = 0; j < 8; j += 4) {
        if ((i < 4 && j >= 4) || (i >= 4 && j < 4)) {
          // Don't use DSPs if only one multiplier is nonzero
          one_bit is_zero = size == 2 || size == 3;
          temp += is_zero ? (int_27)0 : (int_27)(kvz_dct_16[0][i][j] * o[8 + j]);
          temp += is_zero ? (int_27)0 : (int_27)(kvz_dct_16[0][i][j + 1] * o[9 + j]);
          temp += is_zero ? (int_27)0 : (int_27)(kvz_dct_16[0][i][j + 2] * o[10 + j]);
          temp += is_zero ? (int_27)0 : (int_27)(kvz_dct_16[0][i][j + 3] * o[11 + j]);
        } else {
          temp += get_lut(kvz_dct_16[size][i][j]) * get_o(o[8 + j]) + get_lut(kvz_dct_16[size][i][j + 1]) * get_o(o[9 + j]);
          temp += get_lut(kvz_dct_16[size][i][j + 2]) * get_o(o[10 + j]) + get_lut(kvz_dct_16[size][i][j + 3]) * get_o(o[11 + j]);
        }
      }
#endif
#else // No DSPs
#pragma hls_unroll yes
      for (int j = 0; j < 8; ++j) {
        int_27 d[4];
        d[0] = kvz_dct_16[0][i][j] * o[8 + j];
        d[1] = kvz_dct_16[1][i][j] * o[8 + j];
        d[2] = kvz_dct_16[2][i][j] * o[8 + j];
        d[3] = kvz_dct_16[3][i][j] * o[8 + j];

        temp += d[size];
      }
#endif
      slice.set_slc(128 + 16 * i, shift_round< 16, true >(temp, shift));
    }

  Mult32x32:
#pragma hls_unroll yes
    for (int i = 0; i < 16; ++i) {
      int_28 temp = 0;
#if 0
            // Unoptimized version for reference.
            /*for (int j = 0; j < 16; ++j) 
            {
                temp += kvz_dct_32[size][i][j] * o[16 + j];
            }*/
            
            /*temp += mult_add_4(kvz_dct_32[size][i][0], kvz_dct_32[size][i][1], kvz_dct_32[size][i][2], kvz_dct_32[size][i][3], o[16], o[17], o[18], o[19]);
            temp += mult_add_4(kvz_dct_32[size][i][4], kvz_dct_32[size][i][5], kvz_dct_32[size][i][6], kvz_dct_32[size][i][7], o[20], o[21], o[22], o[23]);
            temp += mult_add_4(kvz_dct_32[size][i][8], kvz_dct_32[size][i][9], kvz_dct_32[size][i][10], kvz_dct_32[size][i][11], o[24], o[25], o[26], o[27]);
            temp += mult_add_4(kvz_dct_32[size][i][12], kvz_dct_32[size][i][13], kvz_dct_32[size][i][14], kvz_dct_32[size][i][15], o[28], o[29], o[30], o[31]);*/
            
            /*temp = kvz_dct_32[size][i][0] * o[16] + kvz_dct_32[size][i][1] * o[16 + 1] + 
                   kvz_dct_32[size][i][2] * o[16 + 2] + kvz_dct_32[size][i][3] * o[16 + 3] + 
                   kvz_dct_32[size][i][4] * o[16 + 4] + kvz_dct_32[size][i][5] * o[16 + 5] + 
                   kvz_dct_32[size][i][6] * o[16 + 6] + kvz_dct_32[size][i][7] * o[16 + 7] + 
                   kvz_dct_32[size][i][8] * o[16 + 8] + kvz_dct_32[size][i][9] * o[16 + 9] + 
                   kvz_dct_32[size][i][10] * o[16 + 10] + kvz_dct_32[size][i][11] * o[16 + 11] + 
                   kvz_dct_32[size][i][12] * o[16 + 12] + kvz_dct_32[size][i][13] * o[16 + 13] + 
                   kvz_dct_32[size][i][14] * o[16 + 14] + kvz_dct_32[size][i][15] * o[16 + 15];*/
#endif
#ifdef DSP_32
#ifndef OPTIMIZE_DSP_USAGE
            temp += (get_lut(kvz_dct_32[size][i][0])  * get_o(o[16]) + get_lut(kvz_dct_32[size][i][1])  * get_o(o[17]);
            temp += (get_lut(kvz_dct_32[size][i][2])  * get_o(o[18]) + get_lut(kvz_dct_32[size][i][3])  * get_o(o[19]);
            temp += (get_lut(kvz_dct_32[size][i][4])  * get_o(o[20]) + get_lut(kvz_dct_32[size][i][5])  * get_o(o[21]);
            temp += (get_lut(kvz_dct_32[size][i][6])  * get_o(o[22]) + get_lut(kvz_dct_32[size][i][7])  * get_o(o[23]);
            temp += (get_lut(kvz_dct_32[size][i][8])  * get_o(o[24]) + get_lut(kvz_dct_32[size][i][9])  * get_o(o[25]);  
            temp += (get_lut(kvz_dct_32[size][i][10]) * get_o(o[26]) + get_lut(kvz_dct_32[size][i][11]) * get_o(o[27]);
            temp += (get_lut(kvz_dct_32[size][i][12]) * get_o(o[28]) + get_lut(kvz_dct_32[size][i][13]) * get_o(o[29]);
            temp += (get_lut(kvz_dct_32[size][i][14]) * get_o(o[30]) + get_lut(kvz_dct_32[size][i][15]) * get_o(o[31]);

#else // Optimized version
#pragma hls_unroll yes
      for (int j = 0; j < 16; j += 4) {
        if ((i < 4 && j >= 4) || (i < 8 && j >= 8) || (i >= 4 && j < 4) || (i >= 8 && j < 8)) {
          // Don't use DSPs if only one multiplier is nonzero
          one_bit is_zero = size != 0;
          temp += is_zero ? (int_28)0 : (int_28)(kvz_dct_32[0][i][j] * o[16 + j]);
          temp += is_zero ? (int_28)0 : (int_28)(kvz_dct_32[0][i][j + 1] * o[17 + j]);
          temp += is_zero ? (int_28)0 : (int_28)(kvz_dct_32[0][i][j + 2] * o[18 + j]);
          temp += is_zero ? (int_28)0 : (int_28)(kvz_dct_32[0][i][j + 3] * o[19 + j]);

        } else {
          // For others use DSPs
          temp += get_lut(kvz_dct_32[size][i][j]) * get_o(o[16 + j]) + get_lut(kvz_dct_32[size][i][j + 1]) * get_o(o[17 + j]);
          temp += get_lut(kvz_dct_32[size][i][j + 2]) * get_o(o[18 + j]) + get_lut(kvz_dct_32[size][i][j + 3]) * get_o(o[19 + j]);
        }
      }
#endif
#else // No DSPs
#pragma hls_unroll yes
      for (int j = 0; j < 16; ++j) {
        int_27 d[4];
        d[0] = kvz_dct_32[0][i][j] * o[16 + j];
        d[1] = kvz_dct_32[1][i][j] * o[16 + j];
        d[2] = kvz_dct_32[2][i][j] * o[16 + j];
        d[3] = kvz_dct_32[3][i][j] * o[16 + j];

        temp += d[size];
      }

#endif
            slice.set_slc(256 + 16 * i, shift_round<16, true>(temp, shift));
    }

    const static int out_slc[4][32] = {{0, 16, 8, 17, 4, 18, 9, 19, 2, 20, 10, 21, 5, 22, 11, 23, 1, 24, 12, 25, 6, 26, 13, 27, 3, 28, 14, 29, 7, 30, 15, 31},
                                       {0, 8, 4, 9, 2, 10, 5, 11, 1, 12, 6, 13, 3, 14, 7, 15, 16, 24, 20, 25, 18, 26, 21, 27, 17, 28, 22, 29, 19, 30, 23, 31},
                                       {0, 4, 2, 5, 1, 6, 3, 7, 8, 12, 10, 13, 9, 14, 11, 15, 16, 20, 18, 21, 17, 22, 19, 23, 24, 28, 26, 29, 25, 30, 27, 31},
                                       {0, 2, 1, 3, 4, 6, 5, 7, 8, 10, 9, 11, 12, 14, 13, 15, 16, 18, 17, 19, 20, 22, 21, 23, 24, 26, 25, 27, 28, 30, 29, 31}};

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
