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

#ifdef __SYNTHESIS__
template < int w_i, int w_o > ac_int< w_o, false > clip_to_unsigned(ac_int< w_i, true > a) {
  static const ac_int< w_o, false > int_max = -1;

  ac_int< w_o, false > r = a;

  one_bit MSB = a[w_i - 1];
  ac_int< w_i - w_o - 1, false > top_bits = a.slc< w_i - w_o - 1 >(w_o);

  if (MSB) {
    // Signed
    r = 0;
  } else {
    // Unsigned
    if (top_bits)
      r = int_max;
  }

  return r;
}
#else
template < int w_i, int w_o > ac_int< 8, false > clip_to_unsigned(ac_int< 10, true > a) {
  static const ac_int< 8, false > int_max = -1;

  ac_int< 8, false > r = a;

  one_bit MSB = a[10 - 1];
  ac_int< 10 - 8 - 1, false > top_bits = a.slc< 10 - 8 - 1 >(8);

  if (MSB) {
    // Signed
    r = 0;
  } else {
    // Unsigned
    if (top_bits)
      r = int_max;
  }

  return r;
}
#endif

#pragma hls_design top
void main_ip_get_ang_zero(input_port_t &conf_in, pixel_slc_t ref[4][64 / 4][PARALLEL_PIX / 2], output_port_t &hor_out, output_port_t &ver_out) {
  // Read config
  const ip_conf_t ip_conf = to_struct(conf_in.read());
  uint_6 block_size = (LCU_WIDTH / 2) >> ip_conf.size;
  uint_6 block_size_m_one = ((LCU_WIDTH / 2) >> ip_conf.size) - 1;

#pragma hls_pipeline_init_interval 1
  for (uint_6 y = 0; y < MAX_WIDTH; ++y) {
    for (uint_6 x = 0; x < MAX_WIDTH; x += PARALLEL_PIX) {
      output_t output_hor, output_ver;
      uint_6 x_plus_four = x + 4;

      pixel_slc_t ref_slc_v = ref[ip_conf.ip_id][(y + left_offset) >> 2][0];
      pixel_slc_t ref_slc_h = ref[ip_conf.ip_id][(x + above_offset) >> 2][1];

#pragma hls_unroll yes
      for (uint_4 j = 0; j < 4; ++j) {
        pixel_t ref_v = ref_slc_v.slc< 8 >(y.slc< 2 >(0) * 8);
        pixel_t ref_h = ref_slc_h.slc< 8 >((x + j).slc< 2 >(0) * 8);

        pixel_t pred_h = clip_to_unsigned< 10, 8 >(ref_v + ((ref_h - ip_conf.zero) >> 1));
        pixel_t pred_v = clip_to_unsigned< 10, 8 >(ref_h + ((ref_v - ip_conf.zero) >> 1));

        output_hor.set_slc(j * 8, (ip_conf.filter && y == 0) ? pred_h : ref_v);
        output_ver.set_slc(j * 8, (ip_conf.filter && (x + j) == 0) ? pred_v : ref_h);
      }

      // Write four pixels
      hor_out.write(output_hor);
      ver_out.write(output_ver);

      if ((x + PARALLEL_PIX == block_size))
        break;
    }
    if (y == block_size_m_one)
      break;
  }
}
