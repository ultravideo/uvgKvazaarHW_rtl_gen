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

#include "cabac.h"
#include <mc_scverify.h>
#include <string.h>

void binary_coding(uint_8 cabac_states[192 * LCU_COUNT], ac_int< 32, false > low, ac_int< 16, false > range, ac_int< 8, false > buffered_byte, ac_int< 8, false > num_buffered_bytes,
                   ac_int< 8, false > bits_left, uint_8 stream_data, uint_8 stream_cur_bit, uint_8 stream_zercount, cabac_chan &cabac_write, cabac_chan &cabac_write_encode,
                   cabac_chan &cabac_write_coding_tree, ac_channel< ac_int< 5, false > > &ctu_done, uint_8 bitstream_last_data[KVZ_DATA_CHUNK_SIZE * LCU_COUNT],
                   ac_int< 128, false > cabac_metas[LCU_COUNT], ac_int< 1, false > stall_coding_tree);

uint_8 cabac_states_golden1[184] = {
    14, 29, 3,  29, 49, 11, 40, 2,  1,  1,  1,  1,  1, 14, 18, 18, 21, 31, 1,  1,  18, 18, 22, 1,  1,  1,  0,  0,  11, 1,  1,  1,  62, 37, 82, 29, 21, 21, 9, 5,  5,  14, 6,  26, 6,  42,
    9,  29, 70, 14, 9,  42, 9,  29, 70, 14, 9,  42, 9, 29, 70, 14, 9,  13, 2,  22, 22, 30, 50, 30, 50, 14, 50, 2,  21, 50, 2,  21, 5,  5,  6,  9,  13, 14, 9, 41, 13, 10, 21, 61, 41, 21,
    18, 26, 22, 38, 1,  1,  1,  1,  1,  1,  1,  1,  1, 1,  1,  1,  5,  5,  6,  9,  13, 14, 9,  41, 13, 10, 21, 61, 41, 21, 18, 26, 22, 38, 1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  1,  1,
    13, 46, 34, 18, 13, 30, 18, 2,  14, 98, 78, 46, 2, 42, 38, 30, 13, 70, 42, 22, 13, 10, 38, 18, 18, 14, 50, 26, 30, 30, 1,  1,  1,  1,  1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  2,  2,
};

uint_8 cabac_states_golden2[184] = {
    14, 29,  3,  125, 125, 125, 124, 2,  1,  1,  1,  1,  1, 14, 18, 18, 21, 76,  1,  1,  78, 18, 22, 1,  0,  1,  0,  0,  125, 1,  1,  1,   62, 37, 82, 29, 21, 21, 9, 5,  5,  14, 6,  26, 6,  42,
    9,  29,  70, 14,  9,   42,  9,   29, 70, 14, 9,  42, 9, 29, 70, 14, 9,  9,   2,  22, 22, 30, 50, 30, 50, 14, 50, 2,  21,  50, 2,  21,  5,  5,  6,  24, 13, 14, 9, 41, 13, 10, 21, 61, 41, 21,
    18, 102, 24, 38,  1,   1,   1,   1,  1,  1,  1,  1,  1, 1,  1,  1,  5,  5,   6,  24, 13, 14, 9,  41, 13, 10, 21, 61, 41,  21, 18, 112, 22, 38, 1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  1,  1,
    13, 66,  34, 18,  13,  30,  18,  2,  14, 98, 78, 46, 2, 42, 38, 30, 13, 124, 42, 22, 13, 10, 38, 18, 14, 14, 50, 26, 30,  30, 1,  1,   1,  1,  1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  2,  2,
};

uint_8 cabac_states_golden3[184] = {14, 29, 3,  125, 125, 3,  124, 2,  1,  1,  1,  1,  1,   14, 18, 18, 21, 39, 1,  1,   46, 18, 22, 1,  1,  1, 0, 0,  125, 1,  1,  1,  62, 37, 82, 29, 1,
                                    21, 9,  5,  5,   14,  6,  26,  6,  70, 4,  29, 70, 14,  9,  56, 8,  29, 70, 14, 9,   42, 9,  29, 70, 14, 9, 9, 2,  22,  22, 30, 50, 30, 50, 14, 50, 2,
                                    21, 50, 2,  21,  5,   5,  6,   3,  0,  14, 9,  41, 13,  10, 21, 61, 41, 21, 18, 124, 24, 38, 1,  1,  1,  1, 1, 1,  1,   1,  1,  1,  1,  1,  5,  5,  6,
                                    5,  6,  14, 9,   41,  13, 10,  21, 61, 41, 21, 18, 124, 22, 38, 1,  1,  1,  1,  1,   1,  1,  1,  1,  1,  1, 1, 17, 76,  30, 14, 13, 30, 18, 2,  14, 98,
                                    78, 46, 2,  42,  38,  30, 13,  96, 42, 22, 13, 10, 38,  18, 20, 14, 50, 26, 32, 30,  1,  1,  1,  1,  1,  1, 1, 1,  1,   1,  1,  1,  1,  1,  2,  2};

uint_8 cabac_states_golden4[184] = {
    14, 29,  3,  125, 125, 5,  124, 2, 1,  1,   1,  1,  1, 14, 18, 18, 21, 87,  1,  1,  26, 18, 22, 1,  1,  1,  0,  0,  125, 1,  1,  1,   62, 37, 82, 29, 5,  21, 9, 5,  5,  14, 6,  26, 6,  22,
    1,  29,  70, 14,  9,   38, 1,   1, 70, 14,  9,  42, 9, 29, 70, 14, 9,  9,   2,  22, 22, 30, 50, 30, 50, 14, 50, 2,  21,  50, 2,  21,  5,  5,  6,  3,  12, 14, 9, 41, 13, 10, 21, 61, 41, 21,
    18, 124, 24, 38,  1,   1,  1,   1, 1,  1,   1,  1,  1, 1,  1,  1,  5,  5,   6,  19, 6,  20, 9,  41, 13, 10, 21, 61, 41,  21, 18, 124, 22, 38, 1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  1,  1,
    5,  68,  50, 26,  13,  30, 18,  2, 14, 104, 78, 46, 2, 42, 38, 30, 13, 124, 42, 22, 13, 10, 38, 18, 34, 14, 50, 26, 32,  30, 1,  1,   1,  1,  1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  2,  2,
};

uint_8 cabac_states_golden5[184] = {
    14, 29,  3,  125, 125, 3,  124, 2, 1,  1,   1,  1,  1, 14, 18, 18, 21, 63,  1,  1,  22, 18, 22, 1,  1,  1,  0,  0,  125, 1,  1,  1,   62, 37, 82, 29, 3,  21, 9, 5,  5,  14, 6,  26, 6,  36,
    8,  19,  70, 14,  9,   44, 1,   9, 70, 14,  9,  42, 9, 29, 70, 14, 9,  5,   2,  24, 22, 30, 50, 30, 50, 14, 50, 2,  21,  50, 2,  21,  5,  5,  6,  45, 10, 16, 9, 41, 13, 10, 21, 61, 41, 21,
    18, 124, 24, 38,  1,   1,  1,   1, 1,  1,   1,  1,  1, 1,  1,  1,  5,  5,   6,  4,  2,  18, 9,  41, 13, 10, 21, 61, 41,  21, 18, 120, 24, 38, 1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  1,  1,
    4,  116, 74, 54,  13,  30, 18,  2, 14, 104, 78, 46, 2, 42, 38, 30, 13, 124, 42, 22, 13, 10, 38, 18, 26, 14, 50, 26, 32,  30, 1,  1,   1,  1,  1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  2,  2,
};

uint_8 cabac_states_golden6[184] = {14, 29, 3,  125, 125, 125, 124, 2,   1,  1,  1,  1,  1,   14, 18, 18, 21, 76, 1,  1,   78, 18, 22, 1,  0,  1, 0, 0,  125, 1,  1,  1,  62, 37, 82, 29, 21,
                                    21, 9,  5,  5,   14,  6,   26,  6,   42, 9,  29, 70, 14,  9,  42, 9,  29, 70, 14, 9,   42, 9,  29, 70, 14, 9, 9, 2,  22,  22, 30, 50, 30, 50, 14, 50, 2,
                                    21, 50, 2,  21,  5,   5,   6,   24,  13, 14, 9,  41, 13,  10, 21, 61, 41, 21, 18, 102, 24, 38, 1,  1,  1,  1, 1, 1,  1,   1,  1,  1,  1,  1,  5,  5,  6,
                                    24, 13, 14, 9,   41,  13,  10,  21,  61, 41, 21, 18, 112, 22, 38, 1,  1,  1,  1,  1,   1,  1,  1,  1,  1,  1, 1, 13, 66,  34, 18, 13, 30, 18, 2,  14, 98,
                                    78, 46, 2,  42,  38,  30,  13,  124, 42, 22, 13, 10, 38,  18, 14, 14, 50, 26, 30, 30,  1,  1,  1,  1,  1,  1, 1, 1,  1,   1,  1,  1,  1,  1,  2,  2};

uint_8 cabac_states_golden7[184] = {
    14, 29,  3,  125, 125, 9,  124, 2, 1,  1,   1,  1,  1, 14, 18, 18, 21, 23,  1,  1,  116, 18, 22, 1,  1,  1,  0,  0,  125, 1,  1,  1,   62, 37, 82, 29, 6,  21, 9, 5,  5,  14, 6,  26, 6,  32,
    24, 13,  76, 40,  1,   50, 2,   8, 74, 22,  3,  42, 9, 29, 70, 14, 9,  9,   2,  24, 22,  30, 50, 30, 50, 14, 50, 2,  21,  50, 2,  21,  5,  5,  6,  19, 14, 24, 9, 41, 13, 10, 21, 61, 41, 21,
    18, 88,  26, 38,  1,   1,  1,   1, 1,  1,   1,  1,  1, 1,  1,  1,  5,  5,   6,  1,  10,  26, 9,  41, 13, 10, 21, 61, 41,  21, 18, 112, 26, 38, 1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  1,  1,
    4,  124, 68, 42,  13,  30, 18,  2, 14, 120, 78, 46, 2, 42, 38, 30, 13, 124, 46, 22, 13,  10, 38, 18, 28, 14, 50, 26, 32,  30, 1,  1,   1,  1,  1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  2,  2,
};

uint_8 cabac_states_golden8[184] = {
    14, 29, 3,  81, 125, 125, 124, 2,  1,  1,  1,  1,  1, 14, 18, 18, 21, 54,  1,  1,  124, 18, 22, 1,  1,  1,  0,  0,  125, 1,  1,  1,  62, 37, 82, 29, 21, 21, 9, 5,  5,  14, 6,  26, 6,  42,
    9,  29, 70, 14, 9,   42,  9,   29, 70, 14, 9,  42, 9, 29, 70, 14, 9,  9,   2,  22, 22,  30, 50, 30, 50, 14, 50, 2,  21,  50, 2,  21, 5,  5,  6,  4,  13, 14, 9, 41, 13, 10, 21, 61, 41, 21,
    18, 46, 24, 38, 1,   1,   1,   1,  1,  1,  1,  1,  1, 1,  1,  1,  5,  5,   6,  4,  13,  14, 9,  41, 13, 10, 21, 61, 41,  21, 18, 56, 22, 38, 1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  1,  1,
    13, 46, 34, 18, 13,  30,  18,  2,  14, 98, 78, 46, 2, 42, 38, 30, 13, 100, 42, 22, 13,  10, 38, 18, 14, 14, 50, 26, 30,  30, 1,  1,  1,  1,  1,  1,  1,  1,  1, 1,  1,  1,  1,  1,  2,  2,
};

uint_8 cabac_states_golden9[184] = {14, 29, 38, 21, 49, 41, 94, 2,  1,  1,  1,  1,  1,  14, 18, 18, 21, 0,  1,  1,  66, 18, 22, 1,  1,  1, 0, 0,  17, 1,  1,  1,  62, 37, 82, 29, 21,
                                    21, 9,  5,  5,  14, 6,  26, 6,  42, 9,  29, 70, 14, 9,  42, 9,  29, 70, 14, 9,  42, 9,  29, 70, 14, 9, 9, 2,  22, 22, 30, 50, 30, 50, 14, 50, 2,
                                    21, 50, 6,  21, 5,  5,  6,  5,  13, 14, 9,  41, 13, 10, 4,  61, 41, 21, 18, 26, 22, 38, 1,  1,  1,  1, 1, 1,  1,  1,  1,  1,  1,  1,  5,  5,  6,
                                    5,  13, 14, 9,  41, 13, 10, 4,  61, 41, 21, 18, 38, 22, 38, 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1, 13, 54, 34, 18, 13, 30, 18, 2,  14, 98,
                                    78, 46, 2,  42, 38, 30, 13, 60, 42, 22, 13, 10, 38, 18, 14, 14, 50, 26, 32, 30, 1,  1,  1,  1,  1,  1, 1, 1,  1,  1,  1,  1,  1,  1,  2,  2};

uint_8 cabac_states_golden10[184] = {
    14, 29,  84, 13, 45, 43, 124, 2,  1,  1,   1,   1,   1,   14, 18, 18, 21, 125, 1,  1,  55, 18, 22, 1,  1,  1,  0,   0,   125, 1,  1,  1,  76, 16, 42, 10, 9, 21, 9,  5, 5,  14, 6,  26,  6,  44,
    2,  15,  70, 20, 5,  74, 10,  11, 60, 24,  0,   66,  102, 66, 76, 66, 84, 4,   10, 18, 26, 38, 54, 34, 60, 14, 124, 38,  4,   78, 72, 60, 5,  5,  6,  23, 0, 20, 35, 9, 18, 4,  79, 103, 27, 27,
    19, 23,  32, 48, 1,  1,  1,   1,  1,  1,   1,   1,   1,   1,  1,  1,  5,  5,   6,  5,  4,  14, 51, 59, 2,  4,  125, 125, 105, 51, 3,  27, 48, 46, 1,  1,  1, 1,  1,  1, 1,  1,  1,  1,   1,  1,
    0,  124, 72, 18, 4,  1,  12,  2,  38, 124, 124, 106, 28,  70, 52, 32, 9,  122, 76, 72, 13, 10, 38, 18, 64, 16, 60,  30,  46,  30, 1,  1,  1,  1,  1,  1,  1, 1,  1,  1, 1,  1,  1,  1,   2,  2,
};

void fill_cabac_state(ac_int< 5, false > ctu_id, uint_8 golden_states[184], uint_8 cabac_states[192 * LCU_COUNT], ac_channel< ac_int< 8 * 184, false > > &cabac_states_out) {
  for (int a = 0; a < 184; a++) {
    cabac_states[a + ctu_id * 192] = golden_states[a];
  }
}

#define DUT_CALL()                                                                                                                                                                                     \
  CCS_DESIGN(cabac_write_main)                                                                                                                                                                         \
  (cabac_states,                                                                                                                                                                                       \
   low,                                                                                                                                                                                                \
   range,                                                                                                                                                                                              \
   buffered_byte,                                                                                                                                                                                      \
   num_buffered_bytes,                                                                                                                                                                                 \
   bits_left,                                                                                                                                                                                          \
   stream_data,                                                                                                                                                                                        \
   stream_cur_bit,                                                                                                                                                                                     \
   stream_zercount,                                                                                                                                                                                    \
   cabac_write,                                                                                                                                                                                        \
   cabac_write_encode,                                                                                                                                                                                 \
   cabac_write_coding_tree,                                                                                                                                                                            \
   ctu_done,                                                                                                                                                                                           \
   bitstream_last_data,                                                                                                                                                                                \
   cabac_metas,                                                                                                                                                                                        \
   0);

CCS_MAIN(int argc, char *argv[]) {
  static ac_int< 32, false > low;
  static ac_int< 16, false > range;
  static ac_int< 8, false > buffered_byte;
  static ac_int< 8, false > num_buffered_bytes;
  static ac_int< 8, false > bits_left;

  static uint_8 stream_data;
  static uint_8 stream_cur_bit;
  static uint_8 stream_zercount;
  static cabac_chan cabac_write;
  static cabac_chan cabac_write_encode;
  static cabac_chan cabac_write_coding_tree;
  static ac_channel< ac_int< 32 + 16 + 8 + 8 + 8, false > > data;
  static ac_channel< ac_int< 32 + 32 + 8 + 8 + 8, false > > bitstream;
  static uint_8 bitstream_last_data[KVZ_DATA_CHUNK_SIZE * LCU_COUNT];
  static uint_8 cabac_states[192 * LCU_COUNT];
  static ac_channel< ac_int< 5, false > > ctu_done;
  static ac_int< 128, false > cabac_metas[LCU_COUNT];

  static ac_channel< ac_int< 8 * 184, false > > cabac_states_in;
  static ac_channel< ac_int< 8 * 184, false > > cabac_states_out;

  static uint_32 meta[16];

  int RUN_TEST_CASE = 0; // 1-10 0=all

  int tests_passed = 1;
  for (ac_int< 5, false > ctu_id = 0; ctu_id < 16; ctu_id++) {
    printf("##CTU_ID: %d\n", ctu_id);
    if (RUN_TEST_CASE == 1 || RUN_TEST_CASE == 0) {
      printf("##Testcase #1:\n");

      uint_8 cmd[9] = {CMD_START, 3, 3, 3, 3, 2, 2, 2, CMD_STOP};
      uint_8 ctx[9] = {0, 16, 14, 18, 20, 20, 20, 20, 0};
      uint_8 ctx_i[9] = {0, 3, 3, 1, 0, 0, 0, 0, 0};
      uint_8 num_bins[9] = {0, 0, 0, 0, 0, 1, 8, 4, 0};
      uint_8 bin_values[9] = {0, 0, 0, 1, 1, 1, 254, 10, 0};

      ctx_i[0] = ctu_id;
      low = 49920;
      range = 344;
      buffered_byte = 255;
      num_buffered_bytes = 0;
      bits_left = 16;
      stream_data = 0;
      stream_cur_bit = 0;
      stream_zercount = 0;

      memset(cabac_metas, 0, sizeof(cabac_metas));
      memset(cabac_states, 0, sizeof(cabac_states));

      fill_cabac_state(ctu_id, cabac_states_golden1, cabac_states, cabac_states_out);

      for (int a = 0; a < 9; a++) {

        cabac_val temp = cabac_param_to_val(bin_values[a], num_bins[a], ctx[a], ctx_i[a], cmd[a]);
        cabac_write_coding_tree.write(temp);

        DUT_CALL();
      }

      {
        ac_int< 128, false > cabac_metas_temp = cabac_metas[ctu_id];

        // # #Testcase #1:
        // # low 1040656, range 360, buf_byte 87, num_buf_byte 1, bits_left 12
        // # cur_bit 0, data 0, zerocount 0, last_len 1

        {
          ac_int< 32, false > low_r = cabac_metas_temp.slc< 32 >(0);                   // low
          ac_int< 16, false > range_r = cabac_metas_temp.slc< 16 >(32);                // range
          ac_int< 8, false > buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16);          // buffered_byte
          ac_int< 8, false > num_buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8);  // num_buffered_bytes
          ac_int< 8, false > bits_left_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8); // bits_left
          ac_int< 8, false > zerocount_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8 + 8);
          ac_int< 8, false > len_r = cabac_metas_temp.slc< 10 >(32 + 16 + 8 + 8 + 8 + 8);

          if (!(low_r.to_uint() == 1040656 && range_r.to_uint() == 360 && buf_byte_r.to_uint() == 87 && num_buf_byte_r.to_uint() == 1 && bits_left_r.to_uint() == 12)) {
            tests_passed = 0;
            printf("Error\n");
          }
          if (!(zerocount_r.to_uint() == 0 && len_r.to_uint() == 1)) {
            tests_passed = 0;
            printf("Error\n");
          }
        }
      }
    }

    if (RUN_TEST_CASE == 2 || RUN_TEST_CASE == 0) {
      printf("##Testcase #2:\n");

      uint_8 cmd[18] = {CMD_START, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, CMD_STOP};
      uint_8 ctx[18] = {0, 16, 16, 14, 14, 12, 12, 12, 12, 18, 18, 18, 18, 20, 20, 20, 20, 0};
      uint_8 ctx_i[18] = {0, 3, 3, 3, 3, 10, 10, 10, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0};
      uint_8 num_bins[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 1, 0, 0};
      uint_8 bin_values[18] = {0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 12, 0, 0, 0};

      ctx_i[0] = ctu_id;
      low = 75584;
      range = 448;
      buffered_byte = 4;
      num_buffered_bytes = 1;
      bits_left = 12;
      stream_data = 0;
      stream_cur_bit = 0;
      stream_zercount = 1;

      memset(cabac_metas, 0, sizeof(cabac_metas));
      memset(cabac_states, 0, sizeof(cabac_states));

      fill_cabac_state(ctu_id, cabac_states_golden2, cabac_states, cabac_states_out);

      for (int a = 0; a < 18; a++) {
        cabac_val temp = cabac_param_to_val(bin_values[a], num_bins[a], ctx[a], ctx_i[a], cmd[a]);
        cabac_write.write(temp);

        DUT_CALL();
      }
      {
        ac_int< 128, false > cabac_metas_temp = cabac_metas[ctu_id];

        // # #Testcase #2:
        // # low 4736, range 336, buf_byte 100, num_buf_byte 1, bits_left 18
        // # cur_bit 0, data 0, zerocount 0, last_len 3

        {
          ac_int< 32, false > low_r = cabac_metas_temp.slc< 32 >(0);                   // low
          ac_int< 16, false > range_r = cabac_metas_temp.slc< 16 >(32);                // range
          ac_int< 8, false > buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16);          // buffered_byte
          ac_int< 8, false > num_buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8);  // num_buffered_bytes
          ac_int< 8, false > bits_left_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8); // bits_left
          ac_int< 8, false > zerocount_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8 + 8);
          ac_int< 8, false > len_r = cabac_metas_temp.slc< 10 >(32 + 16 + 8 + 8 + 8 + 8);

          if (!(low_r.to_uint() == 4736 && range_r.to_uint() == 336 && buf_byte_r.to_uint() == 100 && num_buf_byte_r.to_uint() == 1 && bits_left_r.to_uint() == 18)) {
            tests_passed = 0;
            printf("Error\n");
          }
          if (!(zerocount_r.to_uint() == 0 && len_r.to_uint() == 3)) {
            tests_passed = 0;
            printf("Error\n");
          }
        }
      }
    }

    if (RUN_TEST_CASE == 3 || RUN_TEST_CASE == 0) {
      printf("##Testcase #3:\n");

      uint_8 cmd[33] = {
          CMD_START, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, CMD_STOP,
      };
      uint_8 ctx[33] = {
          0, 16, 16, 16, 16, 16, 14, 14, 18, 18, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 18, 18, 18, 18, 18, 18, 0,
      };
      uint_8 ctx_i[33] = {
          0, 3, 3, 4, 4, 5, 3, 3, 9, 9, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 0, 1, 2, 3, 3, 3, 3, 0,
      };
      uint_8 num_bins[33] = {
          0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0,
      };
      uint_8 bin_values[33] = {
          0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 6, 0,
      };

      ctx_i[0] = ctu_id;
      low = 2816;
      range = 285;
      buffered_byte = 237;
      num_buffered_bytes = 1;
      bits_left = 16;
      stream_data = 0;
      stream_cur_bit = 0;
      stream_zercount = 0;

      memset(cabac_metas, 0, sizeof(cabac_metas));
      memset(cabac_states, 0, sizeof(cabac_states));

      fill_cabac_state(ctu_id, cabac_states_golden3, cabac_states, cabac_states_out);

      for (int a = 0; a < 33; a++) {
        cabac_val temp = cabac_param_to_val(bin_values[a], num_bins[a], ctx[a], ctx_i[a], cmd[a]);
        cabac_write.write(temp);

        DUT_CALL();
      }
      {
        ac_int< 128, false > cabac_metas_temp = cabac_metas[ctu_id];

        // # #Testcase #3:
        // # low 127472, range 424, buf_byte 101, num_buf_byte 1, bits_left 15
        // # cur_bit 0, data 0, zerocount 0, last_len 4

        {
          ac_int< 32, false > low_r = cabac_metas_temp.slc< 32 >(0);                   // low
          ac_int< 16, false > range_r = cabac_metas_temp.slc< 16 >(32);                // range
          ac_int< 8, false > buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16);          // buffered_byte
          ac_int< 8, false > num_buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8);  // num_buffered_bytes
          ac_int< 8, false > bits_left_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8); // bits_left
          ac_int< 8, false > zerocount_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8 + 8);
          ac_int< 8, false > len_r = cabac_metas_temp.slc< 10 >(32 + 16 + 8 + 8 + 8 + 8);

          if (!(low_r.to_uint() == 127472 && range_r.to_uint() == 424 && buf_byte_r.to_uint() == 101 && num_buf_byte_r.to_uint() == 1 && bits_left_r.to_uint() == 15)) {
            tests_passed = 0;
            printf("Error\n");
          }
          if (!(zerocount_r.to_uint() == 0 && len_r.to_uint() == 4)) {
            tests_passed = 0;
            printf("Error\n");
          }
        }
      }
    }

    if (RUN_TEST_CASE == 4 || RUN_TEST_CASE == 0) {
      printf("##Testcase #4:\n");

      uint_8 cmd[32] = {
          CMD_START, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, CMD_STOP,
      };
      uint_8 ctx[32] = {
          0, 16, 16, 16, 16, 14, 14, 14, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 0,
      };
      uint_8 ctx_i[32] = {
          0, 3, 3, 4, 4, 3, 3, 4, 15, 15, 16, 15, 15, 16, 16, 15, 16, 16, 0, 1, 2, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0,
      };
      uint_8 num_bins[32] = {
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 3, 0, 0,
      };
      uint_8 bin_values[32] = {
          0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 14, 6, 0, 0,
      };

      ctx_i[0] = ctu_id;
      low = 270168;
      range = 340;
      buffered_byte = 24;
      num_buffered_bytes = 1;
      bits_left = 14;
      stream_data = 0;
      stream_cur_bit = 0;
      stream_zercount = 0;

      memset(cabac_metas, 0, sizeof(cabac_metas));
      memset(cabac_states, 0, sizeof(cabac_states));

      fill_cabac_state(ctu_id, cabac_states_golden4, cabac_states, cabac_states_out);

      for (int a = 0; a < 32; a++) {
        cabac_val temp = cabac_param_to_val(bin_values[a], num_bins[a], ctx[a], ctx_i[a], cmd[a]);
        cabac_write.write(temp);

        DUT_CALL();
      }
      {
        ac_int< 128, false > cabac_metas_temp = cabac_metas[ctu_id];

        // # #Testcase #4:
        // # low 40828, range 346, buf_byte 237, num_buf_byte 1, bits_left 16
        // # cur_bit 0, data 0, zerocount 0, last_len 5

        {
          ac_int< 32, false > low_r = cabac_metas_temp.slc< 32 >(0);                   // low
          ac_int< 16, false > range_r = cabac_metas_temp.slc< 16 >(32);                // range
          ac_int< 8, false > buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16);          // buffered_byte
          ac_int< 8, false > num_buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8);  // num_buffered_bytes
          ac_int< 8, false > bits_left_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8); // bits_left
          ac_int< 8, false > zerocount_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8 + 8);
          ac_int< 8, false > len_r = cabac_metas_temp.slc< 10 >(32 + 16 + 8 + 8 + 8 + 8);

          if (!(low_r.to_uint() == 40828 && range_r.to_uint() == 346 && buf_byte_r.to_uint() == 237 && num_buf_byte_r.to_uint() == 1 && bits_left_r.to_uint() == 16)) {
            tests_passed = 0;
            printf("Error\n");
          }
          if (!(zerocount_r.to_uint() == 0 && len_r.to_uint() == 5)) {
            tests_passed = 0;
            printf("Error\n");
          }
        }
      }
    }

    if (RUN_TEST_CASE == 5 || RUN_TEST_CASE == 0) {
      printf("##Testcase #5:\n");

      uint_8 cmd[37] = {
          CMD_START, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, CMD_STOP,
      };
      uint_8 ctx[37] = {
          0, 16, 16, 16, 16, 16, 14, 14, 14, 12, 12, 12, 12, 18, 18, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 18, 18, 18, 20, 20, 0,
      };
      uint_8 ctx_i[37] = {
          0, 3, 3, 4, 4, 5, 3, 3, 3, 18, 19, 19, 20, 9, 9, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 0, 1, 2, 0, 0, 0, 0,
      };
      uint_8 num_bins[37] = {
          0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0,
      };
      uint_8 bin_values[37] = {
          0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 6, 0,
      };

      ctx_i[0] = ctu_id;
      low = 198292;
      range = 303;
      buffered_byte = 160;
      num_buffered_bytes = 1;
      bits_left = 14;
      stream_data = 0;
      stream_cur_bit = 0;
      stream_zercount = 0;

      memset(cabac_metas, 0, sizeof(cabac_metas));
      memset(cabac_states, 0, sizeof(cabac_states));

      fill_cabac_state(ctu_id, cabac_states_golden5, cabac_states, cabac_states_out);

      for (int a = 0; a < 37; a++) {
        cabac_val temp = cabac_param_to_val(bin_values[a], num_bins[a], ctx[a], ctx_i[a], cmd[a]);
        cabac_write.write(temp);

        DUT_CALL();
      }

      {
        ac_int< 128, false > cabac_metas_temp = cabac_metas[ctu_id];

        // # #Testcase #5:
        // # low 66004, range 462, buf_byte 234, num_buf_byte 1, bits_left 15
        // # zerocount 0, cur_bit 0, data 0, last_len 4

        {
          ac_int< 32, false > low_r = cabac_metas_temp.slc< 32 >(0);                   // low
          ac_int< 16, false > range_r = cabac_metas_temp.slc< 16 >(32);                // range
          ac_int< 8, false > buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16);          // buffered_byte
          ac_int< 8, false > num_buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8);  // num_buffered_bytes
          ac_int< 8, false > bits_left_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8); // bits_left
          ac_int< 8, false > zerocount_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8 + 8);
          ac_int< 8, false > len_r = cabac_metas_temp.slc< 10 >(32 + 16 + 8 + 8 + 8 + 8);

          if (!(low_r.to_uint() == 66004 && range_r.to_uint() == 462 && buf_byte_r.to_uint() == 234 && num_buf_byte_r.to_uint() == 1 && bits_left_r.to_uint() == 15)) {
            tests_passed = 0;
            printf("Error\n");
          }
          if (!(zerocount_r.to_uint() == 0 && len_r.to_uint() == 4)) {
            tests_passed = 0;
            printf("Error\n");
          }
        }
      }
    }

    if (RUN_TEST_CASE == 6 || RUN_TEST_CASE == 0) {
      printf("##Testcase #6:\n");

      uint_8 cmd[18] = {
          CMD_START,
          3,
          3,
          3,
          3,
          3,
          3,
          3,
          3,
          3,
          3,
          3,
          3,
          3,
          2,
          2,
          2,
          CMD_STOP,
      };
      uint_8 ctx[18] = {
          0,
          16,
          16,
          14,
          14,
          12,
          12,
          12,
          12,
          18,
          18,
          18,
          18,
          20,
          20,
          20,
          20,
          0,
      };
      uint_8 ctx_i[18] = {
          0,
          3,
          3,
          3,
          3,
          10,
          10,
          10,
          0,
          1,
          2,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
      };
      uint_8 num_bins[18] = {
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          4,
          1,
          0,
          0,
      };
      uint_8 bin_values[18] = {
          0,
          1,
          0,
          1,
          0,
          0,
          1,
          1,
          1,
          0,
          1,
          0,
          1,
          0,
          12,
          0,
          0,
          0,
      };

      ctx_i[0] = ctu_id;
      low = 75584;
      range = 448;
      buffered_byte = 8;
      num_buffered_bytes = 1;
      bits_left = 13;
      stream_data = 0;
      stream_cur_bit = 0;
      stream_zercount = 0;

      memset(cabac_metas, 0, sizeof(cabac_metas));
      memset(cabac_states, 0, sizeof(cabac_states));

      fill_cabac_state(ctu_id, cabac_states_golden6, cabac_states, cabac_states_out);

      for (int a = 0; a < 18; a++) {
        cabac_val temp = cabac_param_to_val(bin_values[a], num_bins[a], ctx[a], ctx_i[a], cmd[a]);
        cabac_write.write(temp);

        DUT_CALL();
      }

      {
        ac_int< 128, false > cabac_metas_temp = cabac_metas[ctu_id];

        // # #Testcase #6:
        // # low 4736, range 336, buf_byte 200, num_buf_byte 1, bits_left 19
        // # zerocount 0, cur_bit 0, data 0, last_len 3

        {
          ac_int< 32, false > low_r = cabac_metas_temp.slc< 32 >(0);                   // low
          ac_int< 16, false > range_r = cabac_metas_temp.slc< 16 >(32);                // range
          ac_int< 8, false > buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16);          // buffered_byte
          ac_int< 8, false > num_buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8);  // num_buffered_bytes
          ac_int< 8, false > bits_left_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8); // bits_left
          ac_int< 8, false > zerocount_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8 + 8);
          ac_int< 8, false > len_r = cabac_metas_temp.slc< 10 >(32 + 16 + 8 + 8 + 8 + 8);

          if (!(low_r.to_uint() == 4736 && range_r.to_uint() == 336 && buf_byte_r.to_uint() == 200 && num_buf_byte_r.to_uint() == 1 && bits_left_r.to_uint() == 19)) {
            tests_passed = 0;
            printf("Error\n");
          }
          if (!(zerocount_r.to_uint() == 0 && len_r.to_uint() == 3)) {
            tests_passed = 0;
            printf("Error\n");
          }
        }
      }
    }

    if (RUN_TEST_CASE == 7 || RUN_TEST_CASE == 0) {
      printf("##Testcase #7:\n");

      uint_8 cmd[54] = {
          CMD_START, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, CMD_STOP,
      };
      uint_8 ctx[54] = {
          0,  16, 16, 16, 14, 14, 14, 14, 14, 14, 12, 12, 12, 12, 12, 18, 18, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
          12, 12, 12, 12, 12, 12, 18, 18, 18, 18, 18, 18, 18, 18, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 0,
      };
      uint_8 ctx_i[54] = {
          0, 3, 3, 4, 3, 3, 4, 4, 5, 5, 13, 13, 13, 13, 14, 9, 9, 9, 9, 9, 9, 9, 10, 9, 9, 10, 11, 9, 10, 11, 10, 11, 0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      };
      uint_8 num_bins[54] = {
          0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 1, 0, 2, 0, 1, 0, 1, 0, 2, 0, 0,
      };
      uint_16 bin_values[54] = {
          0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1438, 0, 0, 2, 0, 0, 0, 0, 0, 2, 0, 0,
      };

      ctx_i[0] = ctu_id;
      low = 41344;
      range = 450;
      buffered_byte = 44;
      num_buffered_bytes = 1;
      bits_left = 14;
      stream_data = 0;
      stream_cur_bit = 0;
      stream_zercount = 0;

      memset(cabac_metas, 0, sizeof(cabac_metas));
      memset(cabac_states, 0, sizeof(cabac_states));

      fill_cabac_state(ctu_id, cabac_states_golden7, cabac_states, cabac_states_out);

      for (int a = 0; a < 54; a++) {
        cabac_val temp = cabac_param_to_val(bin_values[a], num_bins[a], ctx[a], ctx_i[a], cmd[a]);
        cabac_write.write(temp);

        DUT_CALL();
      }

      {
        ac_int< 128, false > cabac_metas_temp = cabac_metas[ctu_id];

        // # #Testcase #7:
        // # low 158980, range 354, buf_byte 14, num_buf_byte 1, bits_left 12
        // # zerocount 0, cur_bit 0, data 0, last_len 7

        {
          ac_int< 32, false > low_r = cabac_metas_temp.slc< 32 >(0);                   // low
          ac_int< 16, false > range_r = cabac_metas_temp.slc< 16 >(32);                // range
          ac_int< 8, false > buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16);          // buffered_byte
          ac_int< 8, false > num_buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8);  // num_buffered_bytes
          ac_int< 8, false > bits_left_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8); // bits_left
          ac_int< 8, false > zerocount_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8 + 8);
          ac_int< 8, false > len_r = cabac_metas_temp.slc< 10 >(32 + 16 + 8 + 8 + 8 + 8);

          if (!(low_r.to_uint() == 158980 && range_r.to_uint() == 354 && buf_byte_r.to_uint() == 14 && num_buf_byte_r.to_uint() == 1 && bits_left_r.to_uint() == 12)) {
            tests_passed = 0;
            printf("Error\n");
          }
          if (!(zerocount_r.to_uint() == 0 && len_r.to_uint() == 7)) {
            tests_passed = 0;
            printf("Error\n");
          }
        }
      }
    }

    if (RUN_TEST_CASE == 8 || RUN_TEST_CASE == 0) {
      printf("##Testcase #8:\n");

      uint_8 cmd[6] = {
          CMD_START,
          3,
          3,
          3,
          2,
          CMD_STOP,
      };
      uint_8 ctx[6] = {
          0,
          16,
          14,
          18,
          18,
          0,
      };
      uint_8 ctx_i[6] = {
          0,
          3,
          3,
          1,
          1,
          0,
      };
      uint_8 num_bins[6] = {
          0,
          0,
          0,
          0,
          1,
          0,
      };
      uint_8 bin_values[6] = {
          0,
          0,
          0,
          0,
          1,
          0,
      };

      ctx_i[0] = ctu_id;
      low = 97248;
      range = 280;
      buffered_byte = 219;
      num_buffered_bytes = 1;
      bits_left = 15;
      stream_data = 0;
      stream_cur_bit = 0;
      stream_zercount = 1;

      memset(cabac_metas, 0, sizeof(cabac_metas));
      memset(cabac_states, 0, sizeof(cabac_states));

      fill_cabac_state(ctu_id, cabac_states_golden8, cabac_states, cabac_states_out);

      for (int a = 0; a < 6; a++) {
        cabac_val temp = cabac_param_to_val(bin_values[a], num_bins[a], ctx[a], ctx_i[a], cmd[a]);
        cabac_write.write(temp);

        DUT_CALL();
      }

      {
        ac_int< 128, false > cabac_metas_temp = cabac_metas[ctu_id];

        // # #Testcase #8:
        // # low 778283, range 299, buf_byte 219, num_buf_byte 1, bits_left 12
        // # zerocount 1, cur_bit 0, data 0, last_len 0

        {
          ac_int< 32, false > low_r = cabac_metas_temp.slc< 32 >(0);                   // low
          ac_int< 16, false > range_r = cabac_metas_temp.slc< 16 >(32);                // range
          ac_int< 8, false > buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16);          // buffered_byte
          ac_int< 8, false > num_buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8);  // num_buffered_bytes
          ac_int< 8, false > bits_left_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8); // bits_left
          ac_int< 8, false > zerocount_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8 + 8);
          ac_int< 8, false > len_r = cabac_metas_temp.slc< 10 >(32 + 16 + 8 + 8 + 8 + 8);

          if (!(low_r.to_uint() == 778283 && range_r.to_uint() == 299 && buf_byte_r.to_uint() == 219 && num_buf_byte_r.to_uint() == 1 && bits_left_r.to_uint() == 12)) {
            tests_passed = 0;
            printf("Error\n");
          }
          if (!(zerocount_r.to_uint() == 1 && len_r.to_uint() == 0)) {
            tests_passed = 0;
            printf("Error\n");
          }
        }
      }
    }

    if (RUN_TEST_CASE == 9 || RUN_TEST_CASE == 0) {
      printf("##Testcase #9:\n");

      uint_8 cmd[9] = {
          CMD_START,
          3,
          4,
          4,
          3,
          3,
          3,
          3,
          CMD_STOP,
      };
      uint_8 ctx[9] = {
          0,
          3,
          3,
          3,
          4,
          8,
          8,
          7,
          0,
      };
      uint_8 ctx_i[9] = {
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          1,
          0,
      };
      uint_8 num_bins[9] = {
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
      };
      uint_8 bin_values[9] = {
          0,
          1,
          1,
          1,
          0,
          0,
          0,
          0,
          0,
      };

      ctx_i[0] = ctu_id;
      low = 1756;
      range = 386;
      buffered_byte = 43;
      num_buffered_bytes = 1;
      bits_left = 18;
      stream_data = 0;
      stream_cur_bit = 0;
      stream_zercount = 0;

      memset(cabac_metas, 0, sizeof(cabac_metas));
      memset(cabac_states, 0, sizeof(cabac_states));

      fill_cabac_state(ctu_id, cabac_states_golden9, cabac_states, cabac_states_out);

      for (int a = 0; a < 9; a++) {
        cabac_val temp = cabac_param_to_val(bin_values[a], num_bins[a], ctx[a], ctx_i[a], cmd[a]);
        cabac_write_encode.write(temp);

        DUT_CALL();
      }

      {
        ac_int< 128, false > cabac_metas_temp = cabac_metas[ctu_id];

        {
          ac_int< 32, false > low_r = cabac_metas_temp.slc< 32 >(0);                   // low
          ac_int< 16, false > range_r = cabac_metas_temp.slc< 16 >(32);                // range
          ac_int< 8, false > buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16);          // buffered_byte
          ac_int< 8, false > num_buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8);  // num_buffered_bytes
          ac_int< 8, false > bits_left_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8); // bits_left
          ac_int< 8, false > zerocount_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8 + 8);
          ac_int< 8, false > len_r = cabac_metas_temp.slc< 10 >(32 + 16 + 8 + 8 + 8 + 8);

          if (!(low_r.to_uint() == 15926 && range_r.to_uint() == 262 && buf_byte_r.to_uint() == 43 && num_buf_byte_r.to_uint() == 1 && bits_left_r.to_uint() == 15)) {
            tests_passed = 0;
            printf("Error\n");
          }
          if (!(zerocount_r.to_uint() == 0 && len_r.to_uint() == 0)) {
            tests_passed = 0;
            printf("Error\n");
          }
        }
      }
    }

    if (RUN_TEST_CASE == 10 || RUN_TEST_CASE == 0) {
      printf("##Testcase #10:\n");

      uint_8 cmd[214 + 2] = {
          CMD_START, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
          3,         3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
          3,         3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3,
          3,         3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, CMD_STOP,
      };
      uint_8 ctx[214 + 2] = {
          0,  16, 16, 16, 16, 16, 16, 16, 16, 16, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 18, 18, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
          11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 18, 18, 11,
          11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 18, 18, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
          12, 12, 18, 18, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 18, 18, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 18, 18, 11,
          11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 18, 18, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 18, 18, 18, 0,
      };
      uint_8 ctx_i[214 + 2] = {
          0,  10, 10, 11, 11, 12, 12, 13, 13, 14, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 14, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 26, 9,  9,  0,  0,  0,  1,  1,  0,  0,  0,  0,  0,
          0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 9,  9,  0,
          0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 26, 9,  9,  0,  24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25,
          25, 26, 9,  9,  0,  1,  24, 24, 24, 24, 24, 25, 24, 24, 25, 26, 24, 25, 26, 25, 26, 26, 9,  9,  1,  26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 9,  9,  1,
          1,  1,  24, 24, 24, 25, 24, 24, 26, 25, 24, 24, 26, 25, 24, 26, 25, 26, 9,  9,  21, 21, 21, 21, 21, 22, 21, 21, 22, 23, 21, 22, 23, 22, 23, 0,  1,  2,  2,  0,
      };
      uint_8 num_bins[214 + 2] = {
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0,
      };
      uint_16 bin_values[214 + 2] = {
          0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 2, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
      };

      ctx_i[0] = ctu_id;
      low = 979686;
      range = 273;
      buffered_byte = 124;
      num_buffered_bytes = 1;
      bits_left = 12;
      stream_data = 0;
      stream_cur_bit = 0;
      stream_zercount = 0;

      memset(cabac_metas, 0, sizeof(cabac_metas));
      memset(cabac_states, 0, sizeof(cabac_states));

      fill_cabac_state(ctu_id, cabac_states_golden10, cabac_states, cabac_states_out);

      for (int a = 0; a < 216; a++) {
        cabac_val temp = cabac_param_to_val(bin_values[a], num_bins[a], ctx[a], ctx_i[a], cmd[a]);
        cabac_write_encode.write(temp);

        DUT_CALL();
      }

      {
        ac_int< 128, false > cabac_metas_temp = cabac_metas[ctu_id];
        {

          ac_int< 32, false > low_r = cabac_metas_temp.slc< 32 >(0);                   // low
          ac_int< 16, false > range_r = cabac_metas_temp.slc< 16 >(32);                // range
          ac_int< 8, false > buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16);          // buffered_byte
          ac_int< 8, false > num_buf_byte_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8);  // num_buffered_bytes
          ac_int< 8, false > bits_left_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8); // bits_left
          ac_int< 8, false > zerocount_r = cabac_metas_temp.slc< 8 >(32 + 16 + 8 + 8 + 8);
          ac_int< 8, false > len_r = cabac_metas_temp.slc< 10 >(32 + 16 + 8 + 8 + 8 + 8);

          if (!(low_r.to_uint() == 3682 && range_r.to_uint() == 418 && buf_byte_r.to_uint() == 158 && num_buf_byte_r.to_uint() == 1 && bits_left_r.to_uint() == 19)) {
            tests_passed = 0;
            printf("Error\n");
          }
          if (!(zerocount_r.to_uint() == 0 && len_r.to_uint() == 12)) {
            tests_passed = 0;
            printf("Error\n");
          }
        }
      }
    }
  }

  if (tests_passed) {
    cout << "## SIMULATION PASSED ##" << endl;
  } else {
    cout << "## SIMULATION FAILED ##" << endl;
  }

  CCS_RETURN(0);
}
