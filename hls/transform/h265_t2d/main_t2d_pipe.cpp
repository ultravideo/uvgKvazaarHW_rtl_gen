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

static const uint_4 bitdepth = 8;

typedef ac_int< 8, false > t2d_2nd_conf_t;
typedef ac_channel< t2d_2nd_conf_t > t2d_2nd_port_t;

#pragma hls_design
void t2d_control(resid_port_t &resid_in, trans_conf_t::port_t &trans_conf, t2d_2nd_port_t &t2d_2nd_conf, t2d_out_port_t &t2d_out_conf, dst_resid_port_t &to_dst, dct_port_t &to_dct) {
  static const uint_5 addr_offset[4][32] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                            {0, 16, 0, 16, 0, 16, 0, 16, 0, 16, 0, 16, 0, 16, 0, 16, 1, 17, 1, 17, 1, 17, 1, 17, 1, 17, 1, 17, 1, 17, 1, 17},
                                            {0, 8, 16, 24, 0, 8, 16, 24, 1, 9, 17, 25, 1, 9, 17, 25, 2, 10, 18, 26, 2, 10, 18, 26, 3, 11, 19, 27, 3, 11, 19, 27},
                                            {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 16, 20, 24, 28, 17, 21, 25, 29, 18, 22, 26, 30, 19, 23, 27, 31}};

  // Read configuration
  const t2d_out_conf_t conf_out = resid_in.read();

  // Get configuration data
  const two_bit depth = conf_out.slc< 2 >(4);
  const two_bit color = conf_out.slc< 2 >(6);

  // Select block size
  const two_bit size = (color && depth != 3) ? two_bit(depth + 1) : depth;
  const one_bit dual_chroma = one_bit(color == 3);

  const uint_4 shift_1st = 4 - size + (bitdepth - 8);
  const uint_4 shift_2nd = 11 - size;

  trans_conf_t conf_trans;
  conf_trans.depth = depth;
  conf_trans.color = color;

  t2d_2nd_conf_t conf_2nd;
  conf_2nd.set_slc(0, size);
  conf_2nd.set_slc(2, dual_chroma);
  conf_2nd.set_slc(3, shift_2nd);
  conf_2nd.set_slc(7, one_bit(1));

  // Use DST for 4x4 Luma
  if (depth == 3 && color == 0) {
    t2d_out_conf.write(conf_out);
    to_dst.write(resid_in.read());
  } else {
  first_pass:
#pragma hls_pipeline_init_interval 1
    for (uint_6 y = 0; y < MAX_WIDTH; ++y) {
      resid_slice_t input = resid_in.read();
      slice_t resids;

    slice:
#pragma hls_unroll yes
      // Re-slice signed 9-bit residual to signed 16-bit
      for (uint_6 x = 0; x < MAX_WIDTH; x++) {
        int_9 resid_slc = input.slc< 9 >(9 * x);
        resids.set_slc(16 * x, int_16(resid_slc));
      }

      // Add configuration to slice
      resids.set_slc(16 * MAX_WIDTH, size);
      resids.set_slc(16 * MAX_WIDTH + 2, dual_chroma);
      resids.set_slc(16 * MAX_WIDTH + 3, shift_1st);
      resids.set_slc(16 * MAX_WIDTH + 7, one_bit(1));

      // Write data to transform
      to_dct.write(resids);

      // Write configs to other blocks
      if (!y) {
        trans_conf.write(conf_trans);
        t2d_2nd_conf.write(conf_2nd);
        t2d_out_conf.write(conf_out);
      }

      if (y == loops[dual_chroma][size])
        break;
    }
  }
}

#pragma hls_design
void t2d_2nd_pass(t2d_2nd_port_t &t2d_2nd_conf, coeff_port_t &from_dct, dct_port_t &to_dct) {
#ifndef __SYNTHESIS__
  if (!t2d_2nd_conf.available(1))
    return;
#endif

  // Read configuration
  const t2d_2nd_conf_t conf_2nd = t2d_2nd_conf.read();

  const two_bit size = conf_2nd.slc< 2 >(0);
  const one_bit dual_chroma = conf_2nd.slc< 1 >(2);

#pragma hls_pipeline_init_interval 1
  for (uint_6 y = 0; y < MAX_WIDTH; ++y) {
    slice_t coeffs = from_dct.read();

    // Add configuration to slice
    coeffs.set_slc(16 * MAX_WIDTH, conf_2nd);

    // Write data to transform
    to_dct.write(coeffs);

    if (y == loops[dual_chroma][size])
      break;
  }
}

#pragma hls_design
void t2d_out(t2d_out_port_t &t2d_out_conf, coeff_port_t &coeff_out, dst_coeff_port_t &dst_in, coeff_port_t &dct_in) {
  // Read configuration
  const t2d_out_conf_t conf_out = t2d_out_conf.read();
  coeff_out.write(conf_out);

  // Get configuration data
  const two_bit depth = conf_out.slc< 2 >(4);
  const two_bit color = conf_out.slc< 2 >(6);

  // Select block size
  const two_bit size = (color && depth != 3) ? two_bit(depth + 1) : depth;
  const one_bit dual_chroma = one_bit(color == 3);

  // DST & TR Skip for 4x4 Luma
  if (depth == 3 && color == 0) {
    const uint_4 shift = MAX_TR_DYNAMIC_RANGE - bitdepth - 2;

    dst_coeff_slice_t coeffs = dst_in.read();
    coeff_slice_t output;

  tr_skip:
#pragma hls_unroll yes
    for (uint_6 x = 0; x < 16; ++x) {
      int_16 coeff = coeffs.slc< 16 >(16 * x);
      output.set_slc(16 * (16 + x), coeff << shift);
    }

    output.set_slc(0, dst_in.read());
    coeff_out.write(output);
  } else {
  write_coeffs:
#pragma hls_pipeline_init_interval 1
    for (uint_6 y = 0; y < MAX_WIDTH; ++y) {
      coeff_out.write(dct_in.read());

      if (y == loops[dual_chroma][size])
        break;
    }
  }
}

#pragma hls_design top
void t2d(resid_port_t &resid_in, coeff_port_t &coeff_out, int_16 trans_push[SIZE_MULT * MAX_WIDTH][MAX_WIDTH], int_16 trans_pull[SIZE_MULT * MAX_WIDTH][MAX_WIDTH]) {
  static dct_port_t to_dct_1st;
  static dct_port_t to_dct_2nd;

  static coeff_port_t from_dct_1st;
  static coeff_port_t from_dct_2nd;
  static coeff_port_t to_dct_trans;

  static dst_resid_port_t to_dst;
  static dst_coeff_port_t from_dst;

  static t2d_out_port_t conf_out;
  static t2d_2nd_port_t conf_2nd;

  static trans_conf_t::port_t trans_conf;

  // Unused ports
  static coeff_port_t u_from_dct_1st;
  static coeff_port_t u_from_dct_2nd;

  t2d_control(resid_in, trans_conf, conf_2nd, conf_out, to_dst, to_dct_1st);

  t2d_dst< 0 >(to_dst, from_dst);
  t2d_dct< 0 >(to_dct_1st, to_dct_trans, u_from_dct_1st);
  t2d_transpose_pipe< 0 >(trans_conf, to_dct_trans, from_dct_1st, trans_push, trans_pull);

  t2d_2nd_pass(conf_2nd, from_dct_1st, to_dct_2nd);
  t2d_dct< 1 >(to_dct_2nd, from_dct_2nd, u_from_dct_2nd);

  t2d_out(conf_out, coeff_out, from_dst, from_dct_2nd);
}
