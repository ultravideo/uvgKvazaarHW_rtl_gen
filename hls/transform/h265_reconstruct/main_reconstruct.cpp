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

template < int w_i, int w_o > ac_int< w_o, false > clip_to_unsigned(ac_int< w_i, true > a) {
  static const ac_int< w_o, false > int_max = -1;

  ac_int< w_o, false > r = a;

  one_bit MSB = a[w_i - 1];
  ac_int< w_i - w_o - 1, true > top_bits = a.template slc< w_i - w_o - 1 >(w_o);
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

#pragma hls_design top
void main_reconstruct(resid_port_t &resid_in, pixel_port_t &pred_in, pixel_port_t &ref_in, pixel_port_t &rec_out) {
  static const uint_5 loops[2][4] = {{31, 7, 1, 0}, {15, 3, 0, 0}};

  // Read and write config
  const conf_t conf(resid_in.read());
  rec_out.write(conf.toInt());

  pixel_slice_t output = 0;
  uint_25 ssd[2] = {0, 0};

#pragma hls_pipeline_init_interval 1
  for (uint_6 y = 0; y < MAX_WIDTH; ++y) {

    resid_slice_t resid_slice = resid_in.read();
    pixel_slice_t pred_slice = pred_in.read();
    pixel_slice_t ref_slice = ref_in.read();

    if (!conf.color && conf.depth == 3) {
      // Move pixels for transform skip
      pred_slice.set_slc(128, pred_slice.slc< 128 >(0));
      ref_slice.set_slc(128, ref_slice.slc< 128 >(0));
    }

    int_9 diffs[2][16];

  dual_op:
#pragma hls_unroll yes
    for (two_bit r = 0; r < 2; ++r) {

    slice_half:
#pragma hls_unroll yes
      for (uint_6 x = 0; x < 16; x += 4) {

      slice_four:
#pragma hls_unroll yes
        for (uint_4 i = 0; i < 4; ++i) {
          pixel_t pred = pred_slice.slc< 8 >(8 * (16 * r + x + i));
          pixel_t ref = ref_slice.slc< 8 >(8 * (16 * r + x + i));

          int_16 rec16 = int_16(resid_slice.slc< 16 >(16 * (16 * r + x + i))) + pred;
          pixel_t rec = clip_to_unsigned< 16, 8 >(rec16);

          diffs[r][x + i] = ref - rec;
          output.set_slc(8 * (16 * r + x + i), rec);
        }
      }
    }

    if (y == 0) {
      ssd[0] = 0;
      ssd[1] = 0;
    }
    one_bit accum_clear = (y == 0);

    ssd[0] += diffs[0][0] * diffs[0][0] + diffs[0][1] * diffs[0][1] + diffs[0][2] * diffs[0][2] + diffs[0][3] * diffs[0][3] + diffs[0][4] * diffs[0][4] + diffs[0][5] * diffs[0][5] +
              diffs[0][6] * diffs[0][6] + diffs[0][7] * diffs[0][7] + diffs[0][8] * diffs[0][8] + diffs[0][9] * diffs[0][9] + diffs[0][10] * diffs[0][10] + diffs[0][11] * diffs[0][11] +
              diffs[0][12] * diffs[0][12] + diffs[0][13] * diffs[0][13] + diffs[0][14] * diffs[0][14] + diffs[0][15] * diffs[0][15];

    ssd[1] += diffs[1][0] * diffs[1][0] + diffs[1][1] * diffs[1][1] + diffs[1][2] * diffs[1][2] + diffs[1][3] * diffs[1][3] + diffs[1][4] * diffs[1][4] + diffs[1][5] * diffs[1][5] +
              diffs[1][6] * diffs[1][6] + diffs[1][7] * diffs[1][7] + diffs[1][8] * diffs[1][8] + diffs[1][9] * diffs[1][9] + diffs[1][10] * diffs[1][10] + diffs[1][11] * diffs[1][11] +
              diffs[1][12] * diffs[1][12] + diffs[1][13] * diffs[1][13] + diffs[1][14] * diffs[1][14] + diffs[1][15] * diffs[1][15];

    rec_out.write(output);
    if (y == loops[conf.color == 3][conf.size()])
      break;
  }

  output = ssd[0] + ssd[1];
  output.set_slc(32, ssd[0]);
  output.set_slc(64, ssd[1]);

  rec_out.write(output);
}
