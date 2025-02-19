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

#include "ang_neg_mode_setup.h"
#include "global.h"

static const int left_offset = -1; // Negative to compensate separate zero pixel
static const int above_offset = 31;

#pragma hls_design
void main_ip_get_ang_neg_hier(input_port_t &conf_in, ac_channel< ac_int< 6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 2 + 8 > > &ref1_hier,
                              ac_channel< ac_int< 6 + 6 + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 2 + 8 > > &ref2_hier) {
  // Read config
  const ip_conf_t ip_conf = to_struct(conf_in.read());
  const uint_6 *side_pos = indexes;
  uint_6 block_size_minus_one = ((LCU_WIDTH / 2) >> ip_conf.size) - 1;
  uint_6 block_size = ((LCU_WIDTH / 2) >> ip_conf.size);

#pragma hls_pipeline_init_interval 1
  for (uint_6 y = 0; y < MAX_WIDTH; y++) {
    output_t output_h = 0, output_v = 0;
    // delta_pos_h_base = 0;

    ac_int< 11, true > delta_pos_v = delta_int_table[y];
    ac_int< 6, true > delta_fract_v = delta_fract_table[y];
    ac_int< 6, true > minus_delta_fract_v = (32 - delta_fract_v);

    ac_int< 6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 2 + 8 > ref_temp1;
    ac_int< 6 + 6 + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 2 + 8 > ref_temp2;
    // Do linear filtering
    for (uint_6 x = 0; x < MAX_WIDTH; x += PARALLEL_PIX) {
      pixel_t first, second, pred;

    hor_pred:
#pragma hls_unroll yes
      for (uint_4 j = 0; j < PARALLEL_PIX; ++j) {
        ac_int< 11, true > delta_pos_h = delta_int_table[x + j];
        ac_int< 6, true > delta_fract_h = delta_fract_table[x + j];
        ref_temp1.set_slc(6 * j, delta_fract_h);
        ac_int< 6, true > minus_delta_fract_h = (32 - delta_fract_h);
        ref_temp1.set_slc(6 * PARALLEL_PIX + 6 * j, minus_delta_fract_h);

        pixel_t first, second;

        int_7 ref_main = (delta_pos_h /* >> 5*/) + y + 1;
        ref_temp1.set_slc(6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 7 * j, ref_main);

        uint_7 ref_pos1;
        uint_7 ref_pos2;
        if (ref_main < 0) {
          ref_pos1 = side_pos[-ref_main - 1] + above_offset;
        } else {
          ref_pos1 = ref_main + left_offset;
        }
        ref_temp1.set_slc(6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * j, ref_pos1);

        if (ref_main + 1 < 0) {
          ref_pos2 = side_pos[-ref_main - 2] + above_offset;
        } else {
          ref_pos2 = ref_main + 1 + left_offset;
        }
        ref_temp1.set_slc(6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * j, ref_pos2);

        ref_temp1.set_slc(6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX, ip_conf.ip_id);
        ref_temp1.set_slc(6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 2, ip_conf.zero);
      }

    ver_pred:
#pragma hls_unroll yes
      for (uint_4 j = 0; j < PARALLEL_PIX; ++j) {
        ref_temp2.set_slc(0, delta_fract_v);
        ref_temp2.set_slc(6, minus_delta_fract_v);

        pixel_t first, second;

        int_7 ref_main = (delta_pos_v) + x + j + 1;
        ref_temp2.set_slc(6 + 6 + 7 * j, ref_main);

        uint_7 ref_pos1;
        uint_7 ref_pos2;
        if (ref_main < 0) {
          ref_pos1 = side_pos[-ref_main - 1] + left_offset;
        } else {
          ref_pos1 = ref_main + above_offset;
        }
        ref_temp2.set_slc(6 + 6 + 7 * PARALLEL_PIX + 7 * j, ref_pos1);

        if (ref_main + 1 < 0) {
          ref_pos2 = side_pos[-ref_main - 2] + left_offset;
        } else {
          ref_pos2 = ref_main + 1 + above_offset;
        }
        ref_temp2.set_slc(6 + 6 + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * j, ref_pos2);

        ref_temp2.set_slc(6 + 6 + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX, ip_conf.ip_id);
        ref_temp2.set_slc(6 + 6 + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 2, ip_conf.zero);
      }

      ref1_hier.write(ref_temp1);
      ref2_hier.write(ref_temp2);

      if ((x + PARALLEL_PIX == block_size))
        break;
    }
    if (y == block_size_minus_one)
      break;
  }
}

#pragma hls_design
void main_ip_get_ang_neg_hier_hor(ac_channel< ac_int< 6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 2 + 8 > > &conf_in,
                                  pixel_t ref[4][64][PARALLEL_PIX * 2], output_port_t &hor_out) {
#ifndef __SYNTHESIS__
  while (conf_in.available(1)) {
#endif
    // Read config
    ac_int< 6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 2 + 8 > conf = conf_in.read();
    output_t output_h = 0;
#pragma hls_unroll yes
    for (uint_4 j = 0; j < PARALLEL_PIX; ++j) {
      ac_int< 6, true > delta_fract_h = conf.slc< 6 >(6 * j);
      ac_int< 6, true > minus_delta_fract_h = conf.slc< 6 >(6 * PARALLEL_PIX + 6 * j);
      int_7 ref_main = conf.slc< 7 >(6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 7 * j);
      uint_7 ref_pos1 = conf.slc< 7 >(6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * j);
      uint_7 ref_pos2 = conf.slc< 7 >(6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * j);

      two_bit ip_id = conf.slc< 2 >(6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX);
      pixel_t zero = conf.slc< 8 >(6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 2);

      pixel_t first = ref[ip_id][ref_pos1][j];
      pixel_t second = ref[ip_id][ref_pos2][j + PARALLEL_PIX];

      if (ref_main == 0) {
        first = zero;
      }
      if (ref_main + 1 == 0) {
        second = zero;
      }

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
void main_ip_get_ang_neg_hier_ver(ac_channel< ac_int< 6 + 6 + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 2 + 8 > > &conf_in, pixel_t ref[4][64][PARALLEL_PIX * 2],
                                  output_port_t &ver_out) {
#ifndef __SYNTHESIS__
  while (conf_in.available(1)) {
#endif
    // Read config
    ac_int< 6 + 6 + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 2 + 8 > conf = conf_in.read();
    output_t output_v = 0;
    pixel_t ver_prev;
#pragma hls_unroll yes
    for (uint_4 j = 0; j < PARALLEL_PIX; ++j) {
      ac_int< 6, true > delta_fract_v = conf.slc< 6 >(0);
      ac_int< 6, true > minus_delta_fract_v = conf.slc< 6 >(6);
      int_7 ref_main = conf.slc< 7 >(6 + 6 + 7 * j);
      uint_7 ref_pos1 = conf.slc< 7 >(6 + 6 + 7 * PARALLEL_PIX + 7 * j);
      uint_7 ref_pos2 = conf.slc< 7 >(6 + 6 + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * j);

      two_bit ip_id = conf.slc< 2 >(6 + 6 + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX);
      pixel_t zero = conf.slc< 8 >(6 + 6 + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 2);

      pixel_t first, second;

      if (j == 0) {
        // Only read first pixel on first go
        first = ref[ip_id][ref_pos1][j];
        if (ref_main == 0) {
          first = zero;
        }
      } else {
        // Use previous second pixel as first
        first = ver_prev;
      }

      second = ver_prev = ref[ip_id][ref_pos2][j + PARALLEL_PIX];

      if (ref_main + 1 == 0) {
        second = ver_prev = zero;
      }

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
MAIN_IP_GET_ANG_NEG(MODES) {
  static ac_channel< ac_int< 6 * PARALLEL_PIX + 6 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 2 + 8 > > ref1_hier;
  static ac_channel< ac_int< 6 + 6 + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 7 * PARALLEL_PIX + 2 + 8 > > ref2_hier;

  main_ip_get_ang_neg_hier(conf_in, ref1_hier, ref2_hier);
  main_ip_get_ang_neg_hier_hor(ref1_hier, ref1, hor_out);
  main_ip_get_ang_neg_hier_ver(ref2_hier, ref2, ver_out);
}
