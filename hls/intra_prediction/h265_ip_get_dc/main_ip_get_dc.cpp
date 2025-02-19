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

#pragma hls_design top
void main_ip_get_dc(input_port_t &conf_in, pixel_slc_t ref[4][64 / 4][PARALLEL_PIX / 2], output_port_t &pred_out) {
  // Read config
  const ip_conf_t ip_conf = to_struct(conf_in.read());
  uint_6 block_size = (LCU_WIDTH / 2) >> ip_conf.size;
  uint_6 block_size_m_one = ((LCU_WIDTH / 2) >> ip_conf.size) - 1;

#pragma hls_pipeline_init_interval 1
  for (uint_6 y = 0; y < MAX_WIDTH; y++) {
    // Generate prediction signal
    for (uint_6 x = 0; x < MAX_WIDTH; x += PARALLEL_PIX) {
      output_t output_dc;

      pixel_slc_t ref_slc_v = ref[ip_conf.ip_id][(y + left_offset) >> 2][0];
      pixel_slc_t ref_slc_h = ref[ip_conf.ip_id][(x + above_offset) >> 2][1];

#pragma hls_unroll yes
      for (uint_4 j = 0; j < 4; ++j) {
        pixel_t ref_v = ref_slc_v.slc< 8 >(y.slc< 2 >(0) * 8);
        pixel_t ref_h = ref_slc_h.slc< 8 >((x + j).slc< 2 >(0) * 8);

        pixel_t dc_filt = ((!y ? ref_h : ip_conf.dc_val) + 2 * ip_conf.dc_val + (!x && !j ? ref_v : ip_conf.dc_val) + 2) >> 2;
        output_dc.set_slc(8 * j, ip_conf.filter ? dc_filt : ip_conf.dc_val);
      }

      pred_out.write(output_dc);

      if ((x + PARALLEL_PIX == block_size))
        break;
    }
    if (y == block_size_m_one)
      break;
  }
}
