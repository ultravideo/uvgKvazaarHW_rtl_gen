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

#define SHIFT ac::log2_ceil< INTRA_WIDTH >::val

static const uint_7 num_ref_pixels_top[16][16] = {{64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},
                                                  {8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4},
                                                  {16, 12, 8, 4, 16, 12, 8, 4, 16, 12, 8, 4, 16, 12, 8, 4},
                                                  {8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4},
                                                  {32, 28, 24, 20, 16, 12, 8, 4, 32, 28, 24, 20, 16, 12, 8, 4},
                                                  {8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4},
                                                  {16, 12, 8, 4, 16, 12, 8, 4, 16, 12, 8, 4, 16, 12, 8, 4},
                                                  {8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4},
                                                  {64, 60, 56, 52, 48, 44, 40, 36, 32, 28, 24, 20, 16, 12, 8, 4},
                                                  {8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4},
                                                  {16, 12, 8, 4, 16, 12, 8, 4, 16, 12, 8, 4, 16, 12, 8, 4},
                                                  {8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4},
                                                  {32, 28, 24, 20, 16, 12, 8, 4, 32, 28, 24, 20, 16, 12, 8, 4},
                                                  {8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4},
                                                  {16, 12, 8, 4, 16, 12, 8, 4, 16, 12, 8, 4, 16, 12, 8, 4},
                                                  {8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4}};
static const uint_7 num_ref_pixels_left[16][16] = {{64, 4, 8, 4, 16, 4, 8, 4, 32, 4, 8, 4, 16, 4, 8, 4},
                                                   {60, 4, 4, 4, 12, 4, 4, 4, 28, 4, 4, 4, 12, 4, 4, 4},
                                                   {56, 4, 8, 4, 8, 4, 8, 4, 24, 4, 8, 4, 8, 4, 8, 4},
                                                   {52, 4, 4, 4, 4, 4, 4, 4, 20, 4, 4, 4, 4, 4, 4, 4},
                                                   {48, 4, 8, 4, 16, 4, 8, 4, 16, 4, 8, 4, 16, 4, 8, 4},
                                                   {44, 4, 4, 4, 12, 4, 4, 4, 12, 4, 4, 4, 12, 4, 4, 4},
                                                   {40, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4},
                                                   {36, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
                                                   {32, 4, 8, 4, 16, 4, 8, 4, 32, 4, 8, 4, 16, 4, 8, 4},
                                                   {28, 4, 4, 4, 12, 4, 4, 4, 28, 4, 4, 4, 12, 4, 4, 4},
                                                   {24, 4, 8, 4, 8, 4, 8, 4, 24, 4, 8, 4, 8, 4, 8, 4},
                                                   {20, 4, 4, 4, 4, 4, 4, 4, 20, 4, 4, 4, 4, 4, 4, 4},
                                                   {16, 4, 8, 4, 16, 4, 8, 4, 16, 4, 8, 4, 16, 4, 8, 4},
                                                   {12, 4, 4, 4, 12, 4, 4, 4, 12, 4, 4, 4, 12, 4, 4, 4},
                                                   {8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4},
                                                   {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}};

template < unsigned N, unsigned B > class COutput {
public:
  COutput() : output(0) {}

  void operator<<(const ac_int< B > &rhs) {
    output >>= B;
    output.set_slc((N - 1) * B, rhs);
  }

  ac_int< N * B > get() { return output; }

private:
  ac_int< N * B > output;
};

#pragma hls_design top
void main_build_reference_border(ac_channel< int_48 > &conf_in, output_port &conf_out, lcu_rec_t lcu_rec[LCU_COUNT], lcu_border_t lcu_border[LCU_COUNT]) {
  static const pixel_t dc_val = 128;

  const conf_t conf(conf_in.read());

  const one_bit is_chroma = conf.color != 0;
  const uint_6 x_px = conf.x_pos >> is_chroma;
  const uint_6 y_px = conf.y_pos >> is_chroma;

  // Init pointers to LCUs reconstruction buffers
  const slice_t *lcu_rec_top = &lcu_rec[conf.lcu_id].top[lcu_rec_t::colorOffset(conf.color)];
  const slice_t *lcu_rec_left = &lcu_rec[conf.lcu_id].left[lcu_rec_t::colorOffset(conf.color)];

  const slice_t *lcu_border_top = &lcu_border[conf.lcu_id].top[lcu_border_t::colorOffset(conf.color)];
  const slice_t *lcu_border_left = &lcu_border[conf.lcu_id].left[lcu_border_t::colorOffset(conf.color)];

  // Init top borders pointer to point to the correct place in the correct reference array.
  const pixel_t *tlPtr = &lcu_rec[conf.lcu_id].top_left[lcu_rec_t::colorTopLeftOffset(conf.color)];
  const slice_t *top_border = y_px ? &lcu_rec_top[lcu_rec_t::address(x_px)] : &lcu_border_top[lcu_border_t::address(x_px) + 1];
  const slice_t *left_border = x_px ? &lcu_rec_left[lcu_rec_t::address(y_px)] : &lcu_border_left[lcu_border_t::address(y_px) + 1];

  uint_6 first_pixel_top_pos = x_px % INTRA_WIDTH;
  uint_6 first_pixel_left_pos = y_px % INTRA_WIDTH;

  COutput< INTRA_WIDTH, 8 > output;
  slice_t top_slice = top_border[0];
  slice_t left_slice = left_border[0];

  pixel_t first_pixel, top_zero, left_zero;
  pixel_t top_first = top_slice.slc< 8 >(8 * first_pixel_top_pos);
  pixel_t left_first = left_slice.slc< 8 >(8 * first_pixel_left_pos);

  if (first_pixel_left_pos == 0) {
    left_zero = left_border[-1].slc< 8 >(8 * (INTRA_WIDTH - 1));
  } else {
    left_zero = left_slice.slc< 8 >(8 * (first_pixel_left_pos - 1));
  }

  if (first_pixel_top_pos == 0) {
    top_zero = top_border[-1].slc< 8 >(8 * (INTRA_WIDTH - 1));
  } else {
    top_zero = top_slice.slc< 8 >(8 * (first_pixel_top_pos - 1));
  }

  // Generate top-left reference.
  if (!conf.x_zero && !conf.y_zero) {
    if (conf.x_pos != 0 && conf.y_pos != 0) {
      uint_4 yo = (y_px >> 2) - 1;
      uint_4 xo = (x_px >> 2) - 1;

      first_pixel = tlPtr[((yo * 16) >> is_chroma) + xo];
    } else if (conf.x_pos != 0) {
      first_pixel = top_zero;
    } else {
      first_pixel = left_zero;
    }
  } else if (!conf.x_zero) {
    first_pixel = left_first;
  } else if (!conf.y_zero) {
    first_pixel = top_first;
  } else {
    first_pixel = dc_val;
  }

  // Get the number of reference pixels based on the PU coordinate within the LCU.
  uint_7 px_available_left = num_ref_pixels_left[conf.y_pos / 4][conf.x_pos / 4] >> is_chroma;
  uint_7 px_available_top = num_ref_pixels_top[conf.y_pos / 4][conf.x_pos / 4] >> is_chroma;

  // Limit the number of available pixels based on block size and dimensions of the picture.
  px_available_left = MIN(px_available_left, (uint_7)(2 * conf.width()));
  px_available_left = MIN(px_available_left, conf.y_limit >> is_chroma);
  px_available_left = !conf.x_zero ? px_available_left : (uint_7)0;

  px_available_top = MIN(px_available_top, (uint_7)(2 * conf.width()));
  px_available_top = MIN(px_available_top, conf.x_limit >> is_chroma);
  px_available_top = !conf.y_zero ? px_available_top : (uint_7)0;

  pixel_t next_pixel_left = !conf.y_zero ? top_first : dc_val;
  pixel_t next_pixel_top = !conf.x_zero ? left_first : dc_val;

  ac_int< 88, false > confout;
  confout.set_slc(0, conf.toInt());
  confout.set_slc(32, (int_32)conf_in.read());
  confout.set_slc(64, first_pixel);
  confout.set_slc(72, 0 < px_available_left ? left_first : next_pixel_left);
  confout.set_slc(80, 0 < px_available_top ? top_first : next_pixel_top);
  conf_out.write(confout);

gen_reference:
#pragma hls_pipeline_init_interval 1
  for (two_bit r = 0; r < 2; ++r) {
    slice_t cur_slice = !r ? left_slice : top_slice;
    pixel_t next_pixel = !r ? next_pixel_left : next_pixel_top;

    uint_7 px_available = !r ? px_available_left : px_available_top;
    uint_6 first_pixel_pos = !r ? first_pixel_left_pos : first_pixel_top_pos;

    for (uint_7 y = 1; y < 65; y += INTRA_WIDTH) {
      slice_t next_slice = !r ? left_border[(y >> SHIFT) + 1] : top_border[(y >> SHIFT) + 1];

      ac_int< 2 * INTRA_WIDTH * 8, false > slice = cur_slice;
      slice.set_slc(INTRA_WIDTH * 8, next_slice);
      slice >>= 8 * (first_pixel_pos + 1);

#pragma hls_unroll yes
      for (uint_6 x = 0; x < INTRA_WIDTH; x++) {
        next_pixel = (y + x) < px_available ? slice.slc< 8 >(8 * x) : next_pixel;

        output << next_pixel;
      }

      cur_slice = next_slice;
      conf_out.write(output.get());
      if (y + INTRA_WIDTH >= 2 * conf.width() + 1)
        break;
    }
  }
}
