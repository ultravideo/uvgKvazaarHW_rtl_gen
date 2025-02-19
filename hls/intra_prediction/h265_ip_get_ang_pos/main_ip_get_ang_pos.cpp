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

static const int left_offset = 0;
static const int above_offset = 64;

// LUTs as an alternative way of calculating deltas
/*
//mode 2 & 34
uint_6 delta_int_table[32] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,};
uint_5 delta_fract_table[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

//mode 3 & 33
uint_6 delta_int_table[32] = {0, 1, 2, 3, 4, 4, 5, 6, 7, 8, 8, 9, 10, 11, 12, 13, 13, 14, 15, 16, 17, 17, 18, 19, 20, 21, 21, 22, 23, 24, 25, 26,};
uint_5 delta_fract_table[32] = {26, 20, 14, 8, 2, 28, 22, 16, 10, 4, 30, 24, 18, 12, 6, 0, 26, 20, 14, 8, 2, 28, 22, 16, 10, 4, 30, 24, 18, 12, 6, 0,};

//mode 4 & 32
uint_6 delta_int_table[32] = {0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 7, 7, 8, 9, 9, 10, 11, 11, 12, 13, 13, 14, 15, 15, 16, 17, 17, 18, 19, 19, 20, 21,};
uint_5 delta_fract_table[32] = {21, 10, 31, 20, 9, 30, 19, 8, 29, 18, 7, 28, 17, 6, 27, 16, 5, 26, 15, 4, 25, 14, 3, 24, 13, 2, 23, 12, 1, 22, 11, 0,};

//mode 5 & 31
uint_6 delta_int_table[32] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 17,};
uint_5 delta_fract_table[32] = {17, 2, 19, 4, 21, 6, 23, 8, 25, 10, 27, 12, 29, 14, 31, 16, 1, 18, 3, 20, 5, 22, 7, 24, 9, 26, 11, 28, 13, 30, 15, 0,};

//mode 6 & 30
uint_6 delta_int_table[32] = {0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5, 5, 6, 6, 6, 7, 7, 8, 8, 8, 9, 9, 10, 10, 10, 11, 11, 12, 12, 13,};
uint_5 delta_fract_table[32] = {13, 26, 7, 20, 1, 14, 27, 8, 21, 2, 15, 28, 9, 22, 3, 16, 29, 10, 23, 4, 17, 30, 11, 24, 5, 18, 31, 12, 25, 6, 19, 0,};

//mode 7 & 29
uint_6 delta_int_table[32] = {0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9};
uint_5 delta_fract_table[32] = {9, 18, 27, 4, 13, 22, 31, 8, 17, 26, 3, 12, 21, 30, 7, 16, 25, 2, 11, 20, 29, 6, 15, 24, 1, 10, 19, 28, 5, 14, 23, 0,};

//mode 8 & 28
uint_6 delta_int_table[32] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5,};
uint_5 delta_fract_table[32] = {5, 10, 15, 20, 25, 30, 3, 8, 13, 18, 23, 28, 1, 6, 11, 16, 21, 26, 31, 4, 9, 14, 19, 24, 29, 2, 7, 12, 17, 22, 27, 0,};

//mode 9 & 27
uint_6 delta_int_table[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, };
uint_5 delta_fract_table[32] = {2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 0,};
*/

#pragma hls_design
void main_ip_get_ang_pos_hier(input_port_t &conf_in, ac_channel< ac_int< 6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 8 * PARALLEL_PIX + 2 > > &ref1_hier,
                              ac_channel< ac_int< 6 + 6 + 8 * PARALLEL_PIX + 2 > > &ref2_hier) {
  const ip_conf_t ip_conf = to_struct(conf_in.read());

  uint_6 block_size_minus_one = ((LCU_WIDTH / 2) >> ip_conf.size) - 1;
  uint_6 block_size = ((LCU_WIDTH / 2) >> ip_conf.size);

  // Comment out if LUTs are used
  ac_int< 11, false > delta_pos_v = ip_conf.abs_ang;

#pragma hls_pipeline_init_interval 1
  for (uint_6 y = 0; y < MAX_WIDTH; y++) {
    // Uncomment if LUTs are used
    // ac_int<11, false> delta_pos_v = delta_int_table[y];
    ac_int< 6, true > delta_fract_v = delta_pos_v & (32 - 1); // delta_fract_table[y];
    ac_int< 6, true > minus_delta_fract_v = (32 - delta_fract_v);
    ac_int< 11, false > delta_pos_h_base = 0;

    ac_int< 6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 8 * PARALLEL_PIX + 2 > ref_temp1;
    ac_int< 6 + 6 + 8 * PARALLEL_PIX + 2 > ref_temp2;

    // Do linear filtering
    for (uint_6 x = 0; x < MAX_WIDTH; x += PARALLEL_PIX) {
      output_t output_h = 0;
    hor_pred:
#pragma hls_unroll yes
      for (uint_4 j = 0; j < PARALLEL_PIX; ++j) {
        // In comments, if LUTs are used
        ac_int< 11, false > delta_pos_h = delta_pos_h_base + (ip_conf.abs_ang * (j + 1)); // delta_int_table[j+x];
        ac_int< 6, true > delta_fract_h = delta_pos_h & (32 - 1);                         // delta_fract_table[j+x];
        ref_temp1.set_slc(6 * j, delta_fract_h);
        ac_int< 6, true > minus_delta_fract_h = (32 - delta_fract_h);
        ref_temp1.set_slc(6 * PARALLEL_PIX + 6 * j, minus_delta_fract_h);

        // ">> 5" needs to be removed if LUTs are used
        uint_8 ref_main = (delta_pos_h >> 5) + y + left_offset;
        ref_temp1.set_slc(6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 8 * j, ref_main);

        ref_temp1.set_slc(6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 8 * PARALLEL_PIX, ip_conf.ip_id);
      }

    ver_pred:
#pragma hls_unroll yes
      for (uint_4 j = 0; j < PARALLEL_PIX; ++j) {
        ref_temp2.set_slc(0, delta_fract_v);
        ref_temp2.set_slc(6, minus_delta_fract_v);

        // ">> 5" needs to be removed if LUTs are used
        uint_8 ref_main = (delta_pos_v >> 5) + x + j + above_offset;
        ref_temp2.set_slc(6 + 6 + 8 * j, ref_main);

        ref_temp2.set_slc(6 + 6 + 8 * PARALLEL_PIX, ip_conf.ip_id);
      }

      ref1_hier.write(ref_temp1);
      ref2_hier.write(ref_temp2);

      // Not needed if LUTs are used
      delta_pos_h_base += ip_conf.abs_ang * PARALLEL_PIX;

      if ((x + PARALLEL_PIX == block_size))
        break;
    }
    if (y == block_size_minus_one)
      break;
    // Not needed if LUTs are used
    delta_pos_v += ip_conf.abs_ang;
  }
}

#pragma hls_design
void main_ip_get_ang_pos_hier_hor(ac_channel< ac_int< 6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 8 * PARALLEL_PIX + 2 > > &conf_in, pixel_t ref[4][128][PARALLEL_PIX * 2], output_port_t &hor_out) {
#ifndef __SYNTHESIS__
  while (conf_in.available(1)) {
#endif
    // Read config
    ac_int< 6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 8 * PARALLEL_PIX + 2 > conf = conf_in.read();
    output_t output_h = 0;
#pragma hls_unroll yes
    for (uint_4 j = 0; j < PARALLEL_PIX; ++j) {
      ac_int< 6, true > delta_fract_h = conf.slc< 6 >(6 * j);
      ac_int< 6, true > minus_delta_fract_h = conf.slc< 6 >(6 * PARALLEL_PIX + 6 * j);
      uint_8 ref_main = conf.slc< 8 >(6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 8 * j);

      two_bit ip_id = conf.slc< 2 >(6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 8 * PARALLEL_PIX);

      pixel_t first = ref[ip_id][ref_main][j];
      pixel_t second = ref[ip_id][ref_main + 1][j + PARALLEL_PIX];
#ifdef DSP
      pixel_t pred = (Alt_2mult_add< 0, 0, 1, 6, 1, 8, 0 >(minus_delta_fract_h, first, delta_fract_h, second) + 16) >> 5;
#else
    pixel_t pred = (((minus_delta_fract_h * first) + (delta_fract_h * second)) + 16) >> 5;
    if (delta_fract_h == 0) {
      pred = first;
    }
#endif
      output_h.set_slc(8 * j, pred);
    }
    hor_out.write(output_h);
#ifndef __SYNTHESIS__
  }
#endif
}

#pragma hls_design
void main_ip_get_ang_pos_hier_ver(ac_channel< ac_int< 6 + 6 + 8 * PARALLEL_PIX + 2 > > &conf_in, pixel_t ref[4][128][PARALLEL_PIX * 2], output_port_t &ver_out) {
#ifndef __SYNTHESIS__
  while (conf_in.available(1)) {
#endif
    // Read config
    ac_int< 6 + 6 + 8 * 8 + 2 > conf = conf_in.read();
    output_t output_v = 0;
    pixel_t ver_prev;
#pragma hls_unroll yes
    for (uint_4 j = 0; j < PARALLEL_PIX; ++j) {
      ac_int< 6, true > delta_fract_v = conf.slc< 6 >(0);
      ac_int< 6, true > minus_delta_fract_v = conf.slc< 6 >(6);
      uint_8 ref_main = conf.slc< 8 >(6 + 6 + 8 * j);

      two_bit ip_id = conf.slc< 2 >(6 + 6 + 8 * PARALLEL_PIX);

      pixel_t first = ver_prev;

      if (j == 0) { // Only read first pixel on first go
        first = ref[ip_id][ref_main][PARALLEL_PIX];
      } else { // Use previous second pixel as first
        first = ver_prev;
      }

      pixel_t second = ver_prev = ref[ip_id][ref_main + 1][j];
#ifdef DSP
      pixel_t pred = (Alt_2mult_add< 0, 0, 1, 6, 1, 8, 0 >(minus_delta_fract_v, first, delta_fract_v, second) + 16) >> 5;
#else
    pixel_t pred = (((minus_delta_fract_v * first) + (delta_fract_v * second)) + 16) >> 5;
    if (delta_fract_v == 0) {
      pred = first;
    }
#endif
      output_v.set_slc(8 * j, pred);
    }

    // Write four pixels
    ver_out.write(output_v);
#ifndef __SYNTHESIS__
  }
#endif
}

#pragma hls_design top
void main_ip_get_ang_pos(input_port_t &conf_in, pixel_t ref1[4][128][PARALLEL_PIX * 2], pixel_t ref2[4][128][PARALLEL_PIX * 2], output_port_t &hor_out, output_port_t &ver_out) {
  static ac_channel< ac_int< 6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 8 * PARALLEL_PIX + 2 > > ref1_hier;
  static ac_channel< ac_int< 6 + 6 + 8 * PARALLEL_PIX + 2 > > ref2_hier;

  main_ip_get_ang_pos_hier(conf_in, ref1_hier, ref2_hier);
  main_ip_get_ang_pos_hier_hor(ref1_hier, ref1, hor_out);
  main_ip_get_ang_pos_hier_ver(ref2_hier, ref2, ver_out);
}
