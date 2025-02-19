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

#pragma hls_design top
void pred_buffer_pull(conf_port_t &conf_in, ac_channel< ac_int< 8, false > > &intra_mode_in, pixel_port_t &pred_in, pixel_port_t &ref_in, resid_port_t &resid_out, pixel_port_t &pred_out,
                      pixel_port_t &ref_out) {
  static const uint_5 loops[2][4] = {{31, 7, 1, 0}, {15, 3, 0, 0}};
  conf_t conf = conf_to_struct(conf_in.read());
  conf.intra_mode = intra_mode_in.read();
  resid_out.write(conf_to_int(conf));

#pragma hls_pipeline_init_interval 1
  for (uint_6 y = 0; y < MAX_WIDTH; ++y) {
    {
      pixel_slc_t pred_slice = pred_in.read();
      pixel_slc_t ref_slice = ref_in.read();
      resid_slc_t resid_slice;

    calc_resid_slice:
#pragma hls_unroll yes
      for (uint_6 x = 0; x < MAX_WIDTH; ++x) {
        pixel_t ref = ref_slice.slc< 8 >(8 * x);
        pixel_t pred = pred_slice.slc< 8 >(8 * x);
        resid_slice.set_slc(9 * x, int_9(ref - pred));
      }

      ref_out.write(ref_slice);
      pred_out.write(pred_slice);
      resid_out.write(resid_slice);
    }

    if (y == loops[conf.color == 3][conf.size])
      break;
  }
}