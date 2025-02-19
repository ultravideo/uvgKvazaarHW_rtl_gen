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
static const int above_offset = 32;

#pragma hls_design
void main_ip_get_planar_hier(input_port_t &conf_in, pixel_slc_t ref[4][64 / 4][PARALLEL_PIX / 2], ac_channel< ac_int< 8 + 8 + 6 + 8 + 4 + 4 + 6 + 6 * PARALLEL_PIX + 8 * PARALLEL_PIX > > &ref_hier) {
  // Read config
  const ip_conf_t ip_conf = to_struct(conf_in.read());
  uint_6 block_size = (LCU_WIDTH / 2) >> ip_conf.size;
  uint_6 block_size_loop = (LCU_WIDTH / 2) >> ip_conf.size;
  uint_6 block_size_m_one = ((LCU_WIDTH / 2) >> ip_conf.size) - 1;

  const uint_4 shift_1d = 5 - ip_conf.size;
  const uint_4 shift_2d = shift_1d + 1;

#pragma hls_pipeline_init_interval 1
  for (uint_6 y = 0; y < MAX_WIDTH; ++y) {
    // Generate prediction signal
    pixel_slc_t ref_slc_v = ref[ip_conf.ip_id][(y + left_offset) >> 2][0];

    ac_int< 8 + 8 + 6 + 8 + 4 + 4 + 6 + 6 * 8 + 8 * 8 > ref_temp;
    ref_temp.set_slc(0, ip_conf.last_ver);
    ref_temp.set_slc(8, ip_conf.last_hor);
    ref_temp.set_slc(8 + 8, y);

    ref_temp.set_slc(8 + 8 + 6 + 8, shift_1d);
    ref_temp.set_slc(8 + 8 + 6 + 8 + 4, shift_2d);
    ref_temp.set_slc(8 + 8 + 6 + 8 + 4 + 4, block_size);

    for (uint_6 x = 0; x < MAX_WIDTH; x += PARALLEL_PIX) {
      pixel_slc_t ref_slc_h = ref[ip_conf.ip_id][(x + above_offset) >> 2][1];
      pixel_t ref_v = ref_slc_v.slc< 8 >(y.slc< 2 >(0) * 8);
      ref_temp.set_slc(8 + 8 + 6, ref_v);

#pragma hls_unroll yes
      for (uint_4 j = 0; j < 4; ++j) {
        uint_6 h_index = x + j;
        ref_temp.set_slc(8 + 8 + 6 + 8 + 4 + 4 + 6 + 6 * j, h_index);
        pixel_t ref_h = ref_slc_h.slc< 8 >(h_index.slc< 2 >(0) * 8);
        ref_temp.set_slc(8 + 8 + 6 + 8 + 4 + 4 + 6 + 6 * PARALLEL_PIX + 8 * j, ref_h);
      }

      ref_hier.write(ref_temp);

      if ((x + PARALLEL_PIX == block_size_loop))
        break;
    }
    if (y == block_size_m_one)
      break;
  }
}

#pragma hls_design
void main_ip_get_planar_calc(ac_channel< ac_int< 8 + 8 + 6 + 8 + 4 + 4 + 6 + 6 * PARALLEL_PIX + 8 * PARALLEL_PIX > > &conf_in, output_port_t &pred_out) {
#ifndef __SYNTHESIS__
  while (conf_in.available(1)) {
#endif
    ac_int< 8 + 8 + 6 + 8 + 4 + 4 + 6 + 6 * PARALLEL_PIX + 8 * PARALLEL_PIX > conf = conf_in.read();
    pixel_t last_ver = conf.slc< 8 >(0);
    pixel_t last_hor = conf.slc< 8 >(8);
    uint_6 y = conf.slc< 6 >(8 + 8);
    pixel_t ref_v = conf.slc< 8 >(8 + 8 + 6);
    uint_4 shift_1d = conf.slc< 4 >(8 + 8 + 6 + 8);
    uint_4 shift_2d = conf.slc< 4 >(8 + 8 + 6 + 8 + 4);
    uint_6 block_size = conf.slc< 6 >(8 + 8 + 6 + 8 + 4 + 4);

    output_t output_planar;

#pragma hls_unroll yes
    for (uint_4 j = 0; j < PARALLEL_PIX; ++j) {
      uint_6 h_index = conf.slc< 6 >(8 + 8 + 6 + 8 + 4 + 4 + 6 + 6 * j);
      pixel_t ref_h = conf.slc< 8 >(8 + 8 + 6 + 8 + 4 + 4 + 6 + 6 * PARALLEL_PIX + 8 * j);
#ifdef DSP
      pixel_t planar = (Alt_2mult_add< 0, 0, 0, 9, 1, 6, 0 >((last_hor - ref_v), (h_index + 1), (last_ver - ref_h), (y + 1)) + ((uint_16)(ref_v + ref_h) << shift_1d) + block_size) >> shift_2d;
#else
    pixel_t planar = ((last_hor - ref_v) * (h_index + 1) + (last_ver - ref_h) * (y + 1) + ((uint_16)(ref_v + ref_h) << shift_1d) + block_size) >> shift_2d;
#endif
      output_planar.set_slc(8 * j, planar);
    }
    pred_out.write(output_planar);
#ifndef __SYNTHESIS__
  }
#endif
}

#pragma hls_design top
void main_ip_get_planar(input_port_t &conf_in, pixel_slc_t ref[4][64 / 4][PARALLEL_PIX / 2], output_port_t &pred_out) {
  static ac_channel< ac_int< 8 + 8 + 6 + 8 + 4 + 4 + 6 + 6 * PARALLEL_PIX + 8 * PARALLEL_PIX > > ref_hier;
  main_ip_get_planar_hier(conf_in, ref, ref_hier);
  main_ip_get_planar_calc(ref_hier, pred_out);
}
