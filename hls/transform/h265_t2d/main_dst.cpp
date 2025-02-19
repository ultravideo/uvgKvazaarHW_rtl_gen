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

// Fast DST Algorithm. Full matrix multiplication for
// DST and Fast DST algorithm gives identical results

int_17 get_c(int_17 c) {
  // return ac::dont_touch<int_17>(c, ac::dt_until_dsp);
  return c;
}

void main_dst(dst_resid_port_t &input_port, dst_coeff_port_t &output_port) {
  static const uint_4 shift[2] = {1, 8};
  static const uint_6 trans_slc[16] = {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15};
  std::cout << "plaa" << std::endl;
  // Read all data
  dst_resid_slice_t resid_input = input_port.read();
  dst_coeff_slice_t output, input, trans;

slice_16bit:
#pragma hls_unroll yes
  for (uint_6 x = 0; x < 16; x++) {
    // Re-slice signed 9-bit residual to signed 16-bit
    input.set_slc(16 * x, int_16(int_9(resid_input.slc< 9 >(9 * x))));

    // Re-slice and transpose residual for transform skip
    trans.set_slc(16 * x, int_16(int_9(resid_input.slc< 9 >(9 * trans_slc[x]))));
  }

#pragma hls_pipeline_init_interval 2
  for (two_bit pass = 0; pass < 2; ++pass) {
    int_17 rnd_factor = pass ? 1 << (8 - 1) : 1;

    // Send re-sliced residuals for tr skip
    if (!pass)
      output_port.write(trans);
  dst:
#pragma hls_unroll yes
    for (uint_4 y = 0; y < 4; ++y) {
      int_17 c[3];
      int_25 d[4];

      int_17 c0 = (int_16)input.slc< 16 >(64 * y + 0);
      int_17 c1 = (int_16)input.slc< 16 >(64 * y + 16);
      int_17 c2 = (int_16)input.slc< 16 >(64 * y + 32);
      int_17 c3 = (int_16)input.slc< 16 >(64 * y + 48);

      c[0] = c0 + c3;
      c[1] = c1 + c3;
      c[2] = c0 - c1;
#ifdef DST_DSP
      d[0] = 29 * get_c(c[0]) + 55 * get_c(c[1]) + 74 * get_c(c2) + 1 * get_c(rnd_factor);
      d[1] = 74 * get_c(c0) + 74 * get_c(c1) + -74 * get_c(c3) + 1 * get_c(rnd_factor);
      d[2] = 29 * get_c(c[2]) + 55 * get_c(c[0]) + -74 * get_c(c2) + 1 * get_c(rnd_factor);
      d[3] = -29 * get_c(c[1]) + 55 * get_c(c[2]) + 74 * get_c(c2) + 1 * get_c(rnd_factor);
#else
      d[0] = (29 * c[0] + 55 * c[1] + 74 * c2 + rnd_factor);
      d[1] = (74 * (c0 + c1 - c3) + rnd_factor);
      d[2] = (29 * c[2] + 55 * c[0] - 74 * c2 + rnd_factor);
      d[3] = (55 * c[2] - 29 * c[1] + 74 * c2 + rnd_factor);
#endif
      output.set_slc(y * 64 + 0, (int_16)(pass ? d[0] >> 8 : d[0] >> 1));
      output.set_slc(y * 64 + 16, (int_16)(pass ? d[1] >> 8 : d[1] >> 1));
      output.set_slc(y * 64 + 32, (int_16)(pass ? d[2] >> 8 : d[2] >> 1));
      output.set_slc(y * 64 + 48, (int_16)(pass ? d[3] >> 8 : d[3] >> 1));
    }

  transpose:
#pragma hls_unroll yes
    for (uint_6 x = 0; x < 16; ++x) {
      input.set_slc(16 * x, output.slc< 16 >(16 * trans_slc[x]));
    }
  }

  output_port.write(output);
}
