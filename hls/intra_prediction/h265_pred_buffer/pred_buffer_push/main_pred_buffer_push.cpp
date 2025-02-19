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
void pred_buffer_push(input_port_t &pred_in, ac_channel< ac_int< 8, false > > &intra_mode_in, pixel_port_t &pred_out, pixel_port_t &ref_out) {
#ifndef __SYNTHESIS__
  while (pred_in.available(1)) {
#endif
    // Read & Write config

    static uint_8 intra_mode = 0;
    static one_bit read_intra_mode = 1;
    static uint_3 x_cir = 0;
    static pixel_slc_t pred_slice = 0;
    static pixel_slc_t ref_slice = 0;

    if (read_intra_mode) {
      intra_mode = intra_mode_in.read();
      read_intra_mode = 0;
    } else {
      input_t input = pred_in.read();
      one_bit last = input.slc< 1 >(36 * 8 * INTRA_WIDTH);

      pred_slice.set_slc(x_cir * 8 * INTRA_WIDTH, input.slc< 8 * INTRA_WIDTH >(intra_mode * 8 * INTRA_WIDTH));
      ref_slice.set_slc(x_cir * 8 * INTRA_WIDTH, input.slc< 8 * INTRA_WIDTH >(35 * 8 * INTRA_WIDTH));

      if (x_cir == ((MAX_WIDTH / INTRA_WIDTH) - 1) || last) {
        x_cir = 0;
        pred_out.write(pred_slice);
        ref_out.write(ref_slice);
        pred_slice = 0;
        ref_slice = 0;
      } else {
        x_cir++;
      }

      if (last) {
        read_intra_mode = 1;
      }
    }
#ifndef __SYNTHESIS__
  }
#endif
}
