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

#if !defined(SIZE_MULT)
#define SIZE_MULT 2
#endif

namespace {
typedef ac_int< ac::log2_ceil< SIZE_MULT * MAX_WIDTH >::val, false > cir_t;

const uint_6 Mod[2][4] = {{0x1F, 0x07, 0x01, 0x00}, {0x0F, 0x03, 0x00, 0x00}};
const uint_5 out_slc[4][32] = {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31},
                               {0, 16, 2, 18, 4, 20, 6, 22, 8, 24, 10, 26, 12, 28, 14, 30, 1, 17, 3, 19, 5, 21, 7, 23, 9, 25, 11, 27, 13, 29, 15, 31},
                               {0, 8, 16, 24, 4, 12, 20, 28, 1, 9, 17, 25, 5, 13, 21, 29, 2, 10, 18, 26, 6, 14, 22, 30, 3, 11, 19, 27, 7, 15, 23, 31},
                               {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 16, 20, 24, 28, 17, 21, 25, 29, 18, 22, 26, 30, 19, 23, 27, 31}};
} // namespace

/////////////////////////////////////////////////////////////////////////////////////////////
//                                       Unpipelined                                       //
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma hls_design
void it2d_transpose(trans_conf_t::port_t &trans_conf, coeff_port_t &input_port, coeff_port_t &output_port) {
  static int_16 trans[MAX_WIDTH][MAX_WIDTH]; // SinglePort, Interleave MAX_WIDTH
  static bool E = ac::init_array< AC_VAL_DC, 16, true >(&trans[0][0], MAX_WIDTH * MAX_WIDTH);

  // Read config
  const trans_conf_t conf = trans_conf.read();

  if (conf.size() == 3) {
    const coeff_slice_t input = input_port.read();
    coeff_slice_t output;

  fast_transpose:
#pragma hls_unroll yes
    for (uint_6 x = 0; x < MAX_WIDTH; ++x) {
      output.set_slc(16 * x, input.slc< 16 >(16 * out_slc[3][x]));
    }

    output_port.write(output);
  } else {
  read:
#pragma hls_pipeline_init_interval 1
    for (uint_6 y = 0; y < MAX_WIDTH; y++) {
      const coeff_slice_t input = input_port.read();
      coeff_slice_t shift = ctl::RotateL< 512, 16 >(input, uint_5(y << conf.size()));

    write_mem:
#pragma hls_unroll yes
      for (uint_6 x = 0; x < MAX_WIDTH; ++x) {
        trans[((x >> conf.size()) - y) & Mod[0][conf.size()]][x] = shift.slc< 16 >(16 * x);
      }

      if (y == loops[0][conf.size()])
        break;
    }

  write:
#pragma hls_pipeline_init_interval 1
    for (uint_6 y = 0; y < MAX_WIDTH; y++) {
      coeff_slice_t input, output[3];
    read_mem:
#pragma hls_unroll yes
      for (uint_6 x = 0; x < MAX_WIDTH; ++x) {
        input.set_slc(16 * x, trans[y][x]);
      }

      coeff_slice_t shift = ctl::RotateR< 512, 16 >(input, uint_5(y << conf.size()));

    sort:
#pragma hls_unroll yes
      for (uint_6 x = 0; x < MAX_WIDTH; ++x) {
        output[0].set_slc(16 * x, shift.slc< 16 >(16 * out_slc[0][x]));
        output[1].set_slc(16 * x, shift.slc< 16 >(16 * out_slc[1][x]));
        output[2].set_slc(16 * x, shift.slc< 16 >(16 * out_slc[2][x]));
      }

      output_port.write(output[conf.size()]);
      if (y == loops[0][conf.size()])
        break;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                        Pipelined                                        //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma hls_design
void it2d_transpose_push(trans_conf_t::port_t &trans_conf, trans_conf_t::port_t &trans_push_conf, coeff_port_t &input_port, int_16 trans[SIZE_MULT * MAX_WIDTH][MAX_WIDTH]) {
  static cir_t cir_offset = 0;

  // Read config
  const trans_conf_t conf = trans_conf.read();
  cir_t y_offset = cir_offset;

write_mem:
#pragma hls_pipeline_init_interval 1
  for (uint_6 y = 0; y < MAX_WIDTH; y++) {
    const coeff_slice_t input = input_port.read();
    coeff_slice_t shift = ctl::RotateL< 512, 16 >(input, uint_5(y << conf.size()));

#pragma hls_unroll yes
    for (uint_6 x = 0; x < MAX_WIDTH; ++x) {
      trans[cir_t((((x >> conf.size()) - y) & Mod[0][conf.size()]) + y_offset)][x] = shift.slc< 16 >(16 * x);
    }

    cir_offset += 1;
    if (y == loops[0][conf.size()])
      break;
  }

  trans_push_conf.write(conf);
}

#pragma hls_design
void it2d_transpose_pull(trans_conf_t::port_t &trans_conf, coeff_port_t &output_port, int_16 trans[SIZE_MULT * MAX_WIDTH][MAX_WIDTH]) {
  static cir_t cir_offset = 0;

  // Read config
  const trans_conf_t conf = trans_conf.read();
  cir_t y_offset = cir_offset;

read_mem:
#pragma hls_pipeline_init_interval 1
  for (uint_6 y = 0; y < MAX_WIDTH; y++) {
    coeff_slice_t input, output[4];

#pragma hls_unroll yes
    for (uint_6 x = 0; x < MAX_WIDTH; ++x) {
      input.set_slc(16 * x, trans[cir_t(y + y_offset)][x]);
    }

    coeff_slice_t shift = ctl::RotateR< 512, 16 >(input, uint_5(y << conf.size()));

  sort:
#pragma hls_unroll yes
    for (uint_6 x = 0; x < MAX_WIDTH; ++x) {
      output[0].set_slc(16 * x, shift.slc< 16 >(16 * out_slc[0][x]));
      output[1].set_slc(16 * x, shift.slc< 16 >(16 * out_slc[1][x]));
      output[2].set_slc(16 * x, shift.slc< 16 >(16 * out_slc[2][x]));
      output[3].set_slc(16 * x, shift.slc< 16 >(16 * out_slc[3][x]));
    }

    output_port.write(output[conf.size()]);

    cir_offset += 1;
    if (y == loops[0][conf.size()])
      break;
  }
}

#pragma hls_design top
void it2d_transpose_pipe(trans_conf_t::port_t &trans_conf, coeff_port_t &input_port, coeff_port_t &output_port, int_16 trans_push[SIZE_MULT * MAX_WIDTH][MAX_WIDTH],
                         int_16 trans_pull[SIZE_MULT * MAX_WIDTH][MAX_WIDTH]) {
  static trans_conf_t::port_t trans_push_conf;

  it2d_transpose_push(trans_conf, trans_push_conf, input_port, trans_push);
  it2d_transpose_pull(trans_push_conf, output_port, trans_pull);
}
