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
#include <mc_scverify.h>
#include <string.h>

using namespace std;

uint_6 width[340] = {32, 16, 8,  4, 4,  4, 4, 8, 4, 4,  4,  4,  8, 4, 4, 4,  4, 8,  4, 4, 4, 4, 16, 8,  4,  4,  4, 4, 8, 4, 4,  4, 4,  8, 4, 4, 4,  4,  8,  4, 4, 4, 4, 16, 8, 4,  4, 4, 4,
                     8,  4,  4,  4, 4,  8, 4, 4, 4, 4,  8,  4,  4, 4, 4, 16, 8, 4,  4, 4, 4, 8, 4,  4,  4,  4,  8, 4, 4, 4, 4,  8, 4,  4, 4, 4, 32, 16, 8,  4, 4, 4, 4, 8,  4, 4,  4, 4, 8,
                     4,  4,  4,  4, 8,  4, 4, 4, 4, 16, 8,  4,  4, 4, 4, 8,  4, 4,  4, 4, 8, 4, 4,  4,  4,  8,  4, 4, 4, 4, 16, 8, 4,  4, 4, 4, 8,  4,  4,  4, 4, 8, 4, 4,  4, 4,  8, 4, 4,
                     4,  4,  16, 8, 4,  4, 4, 4, 8, 4,  4,  4,  4, 8, 4, 4,  4, 4,  8, 4, 4, 4, 4,  32, 16, 8,  4, 4, 4, 4, 8,  4, 4,  4, 4, 8, 4,  4,  4,  4, 8, 4, 4, 4,  4, 16, 8, 4, 4,
                     4,  4,  8,  4, 4,  4, 4, 8, 4, 4,  4,  4,  8, 4, 4, 4,  4, 16, 8, 4, 4, 4, 4,  8,  4,  4,  4, 4, 8, 4, 4,  4, 4,  8, 4, 4, 4,  4,  16, 8, 4, 4, 4, 4,  8, 4,  4, 4, 4,
                     8,  4,  4,  4, 4,  8, 4, 4, 4, 4,  32, 16, 8, 4, 4, 4,  4, 8,  4, 4, 4, 4, 8,  4,  4,  4,  4, 8, 4, 4, 4,  4, 16, 8, 4, 4, 4,  4,  8,  4, 4, 4, 4, 8,  4, 4,  4, 4, 8,
                     4,  4,  4,  4, 16, 8, 4, 4, 4, 4,  8,  4,  4, 4, 4, 8,  4, 4,  4, 4, 8, 4, 4,  4,  4,  16, 8, 4, 4, 4, 4,  8, 4,  4, 4, 4, 8,  4,  4,  4, 4, 8, 4, 4,  4, 4};
uint_16 x_luma[340] = {0,  0,  0,  0,  4,  0,  4,  8,  8,  12, 8,  12, 0,  0,  4,  0,  4,  8,  8,  12, 8,  12, 16, 16, 16, 20, 16, 20, 24, 24, 28, 24, 28, 16, 16, 20, 16, 20, 24, 24, 28, 24, 28,
                       0,  0,  0,  4,  0,  4,  8,  8,  12, 8,  12, 0,  0,  4,  0,  4,  8,  8,  12, 8,  12, 16, 16, 16, 20, 16, 20, 24, 24, 28, 24, 28, 16, 16, 20, 16, 20, 24, 24, 28, 24, 28, 32,
                       32, 32, 32, 36, 32, 36, 40, 40, 44, 40, 44, 32, 32, 36, 32, 36, 40, 40, 44, 40, 44, 48, 48, 48, 52, 48, 52, 56, 56, 60, 56, 60, 48, 48, 52, 48, 52, 56, 56, 60, 56, 60, 32,
                       32, 32, 36, 32, 36, 40, 40, 44, 40, 44, 32, 32, 36, 32, 36, 40, 40, 44, 40, 44, 48, 48, 48, 52, 48, 52, 56, 56, 60, 56, 60, 48, 48, 52, 48, 52, 56, 56, 60, 56, 60, 0,  0,
                       0,  0,  4,  0,  4,  8,  8,  12, 8,  12, 0,  0,  4,  0,  4,  8,  8,  12, 8,  12, 16, 16, 16, 20, 16, 20, 24, 24, 28, 24, 28, 16, 16, 20, 16, 20, 24, 24, 28, 24, 28, 0,  0,
                       0,  4,  0,  4,  8,  8,  12, 8,  12, 0,  0,  4,  0,  4,  8,  8,  12, 8,  12, 16, 16, 16, 20, 16, 20, 24, 24, 28, 24, 28, 16, 16, 20, 16, 20, 24, 24, 28, 24, 28, 32, 32, 32,
                       32, 36, 32, 36, 40, 40, 44, 40, 44, 32, 32, 36, 32, 36, 40, 40, 44, 40, 44, 48, 48, 48, 52, 48, 52, 56, 56, 60, 56, 60, 48, 48, 52, 48, 52, 56, 56, 60, 56, 60, 32, 32, 32,
                       36, 32, 36, 40, 40, 44, 40, 44, 32, 32, 36, 32, 36, 40, 40, 44, 40, 44, 48, 48, 48, 52, 48, 52, 56, 56, 60, 56, 60, 48, 48, 52, 48, 52, 56, 56, 60, 56, 60};
uint_16 y_luma[340] = {0,  0,  0,  0,  0,  4,  4,  0,  0,  0,  4,  4,  8,  8,  8,  12, 12, 8,  8,  8,  12, 12, 0,  0,  0,  0,  4,  4,  0,  0,  0,  4,  4,  8,  8,  8,  12, 12, 8,  8,  8,  12, 12,
                       16, 16, 16, 16, 20, 20, 16, 16, 16, 20, 20, 24, 24, 24, 28, 28, 24, 24, 24, 28, 28, 16, 16, 16, 16, 20, 20, 16, 16, 16, 20, 20, 24, 24, 24, 28, 28, 24, 24, 24, 28, 28, 0,
                       0,  0,  0,  0,  4,  4,  0,  0,  0,  4,  4,  8,  8,  8,  12, 12, 8,  8,  8,  12, 12, 0,  0,  0,  0,  4,  4,  0,  0,  0,  4,  4,  8,  8,  8,  12, 12, 8,  8,  8,  12, 12, 16,
                       16, 16, 16, 20, 20, 16, 16, 16, 20, 20, 24, 24, 24, 28, 28, 24, 24, 24, 28, 28, 16, 16, 16, 16, 20, 20, 16, 16, 16, 20, 20, 24, 24, 24, 28, 28, 24, 24, 24, 28, 28, 32, 32,
                       32, 32, 32, 36, 36, 32, 32, 32, 36, 36, 40, 40, 40, 44, 44, 40, 40, 40, 44, 44, 32, 32, 32, 32, 36, 36, 32, 32, 32, 36, 36, 40, 40, 40, 44, 44, 40, 40, 40, 44, 44, 48, 48,
                       48, 48, 52, 52, 48, 48, 48, 52, 52, 56, 56, 56, 60, 60, 56, 56, 56, 60, 60, 48, 48, 48, 48, 52, 52, 48, 48, 48, 52, 52, 56, 56, 56, 60, 60, 56, 56, 56, 60, 60, 32, 32, 32,
                       32, 32, 36, 36, 32, 32, 32, 36, 36, 40, 40, 40, 44, 44, 40, 40, 40, 44, 44, 32, 32, 32, 32, 36, 36, 32, 32, 32, 36, 36, 40, 40, 40, 44, 44, 40, 40, 40, 44, 44, 48, 48, 48,
                       48, 52, 52, 48, 48, 48, 52, 52, 56, 56, 56, 60, 60, 56, 56, 56, 60, 60, 48, 48, 48, 48, 52, 52, 48, 48, 48, 52, 52, 56, 56, 56, 60, 60, 56, 56, 56, 60, 60};

typedef struct {
  pixel_t y[LCU_REF_PX_WIDTH + 1];
  pixel_t u[LCU_REF_PX_WIDTH / 2 + 1];
  pixel_t v[LCU_REF_PX_WIDTH / 2 + 1];
} lcu_ref_px_t;

typedef struct {
  pixel_t y[LCU_LUMA_SIZE];
  pixel_t u[LCU_CHROMA_SIZE];
  pixel_t v[LCU_CHROMA_SIZE];
} lcu_yuv;

typedef struct {
  lcu_ref_px_t top_ref;  //!< Reference pixels from adjacent LCUs.
  lcu_ref_px_t left_ref; //!< Reference pixels from adjacent LCUs.
  lcu_yuv rec;           //!< LCU reconstructed pixels
} lcu_t;

typedef struct {
  int x;
  int y;
} vector2d_t;

typedef struct {
  pixel_t left[2 * 32 + 1];
  pixel_t top[2 * 32 + 1];
} kvz_intra_ref;

typedef struct {
  kvz_intra_ref ref;
} kvz_intra_references;

lcu_t init_lcu() {
  lcu_t lcu;

  for (int i = 0; i < LCU_LUMA_SIZE; ++i) {
    lcu.rec.y[i] = rand() % 256;
  }
  for (int i = 0; i < LCU_CHROMA_SIZE; ++i) {
    lcu.rec.u[i] = rand() % 256;
    lcu.rec.v[i] = rand() % 256;
  }

  for (int i = 0; i < LCU_REF_PX_WIDTH + 1; ++i) {
    lcu.top_ref.y[i] = rand() % 256;
    lcu.left_ref.y[i] = rand() % 256;
  }

  for (int i = 0; i < LCU_REF_PX_WIDTH / 2 + 1; ++i) {
    lcu.top_ref.u[i] = rand() % 256;
    lcu.left_ref.u[i] = rand() % 256;

    lcu.top_ref.v[i] = rand() % 256;
    lcu.left_ref.v[i] = rand() % 256;
  }

  lcu.top_ref.y[0] = lcu.left_ref.y[0] = rand() % 256;
  lcu.top_ref.u[0] = lcu.left_ref.u[0] = rand() % 256;
  lcu.top_ref.v[0] = lcu.left_ref.v[0] = rand() % 256;

  return lcu;
}

static int_48 to_ac_int_tb(const conf_t &ip_conf) {
  int_48 output = 0;
  output.set_slc(0, ip_conf.lcu_id);
  output.set_slc(4, ip_conf.depth);
  output.set_slc(6, ip_conf.color);

  output.set_slc(8, ip_conf.x_pos.slc< 4 >(2));
  output.set_slc(12, ip_conf.y_pos.slc< 4 >(2));

  output.set_slc(16, ip_conf.intra_mode);
  output.set_slc(24, ip_conf.qp_scaled);

  output.set_slc(32, ip_conf.x_limit);
  output.set_slc(39, ip_conf.x_zero);
  output.set_slc(40, ip_conf.y_limit);
  output.set_slc(47, ip_conf.y_zero);

  return output;
}

void kvz_intra_build_reference(const int_8 log2_width, const int_8 color, const vector2d_t *const luma_px, const vector2d_t *const pic_px, const lcu_t *const lcu, kvz_intra_references *const refs) {
  // Tables for looking up the number of intra reference pixels based on
  // prediction units coordinate within an LCU.
  // generated by "tools/generate_ref_pixel_tables.py".
  static const uint_8 num_ref_pixels_top[16][16] = {{64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},
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
  static const uint_8 num_ref_pixels_left[16][16] = {{64, 4, 8, 4, 16, 4, 8, 4, 32, 4, 8, 4, 16, 4, 8, 4},
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

  pixel_t *out_left_ref = &refs->ref.left[0];
  pixel_t *out_top_ref = &refs->ref.top[0];

  const pixel_t dc_val = 128;
  const int is_chroma = color != 0 ? 1 : 0;
  const int_8 width = 1 << log2_width;

  std::cout << std::endl << "Width: " << width << " Color: " << color << std::endl;

  // Convert luma coordinates to chroma coordinates for chroma.
  const vector2d_t lcu_px = {luma_px->x % LCU_WIDTH, luma_px->y % LCU_WIDTH};
  const vector2d_t px = {
      lcu_px.x >> is_chroma,
      lcu_px.y >> is_chroma,
  };

  // Init pointers to LCUs reconstruction buffers, such that index 0 refers to block coordinate 0.
  const pixel_t *left_ref = !color ? &lcu->left_ref.y[1] : (color == 1) ? &lcu->left_ref.u[1] : &lcu->left_ref.v[1];
  const pixel_t *top_ref = !color ? &lcu->top_ref.y[1] : (color == 1) ? &lcu->top_ref.u[1] : &lcu->top_ref.v[1];
  const pixel_t *rec_ref = !color ? lcu->rec.y : (color == 1) ? lcu->rec.u : lcu->rec.v;

  // Init top borders pointer to point to the correct place in the correct reference array.
  const pixel_t *top_border;
  if (px.y) {
    top_border = &rec_ref[px.x + (px.y - 1) * (LCU_WIDTH >> is_chroma)];
  } else {
    top_border = &top_ref[px.x];
  }

  // Init left borders pointer to point to the correct place in the correct reference array.
  const pixel_t *left_border;
  int left_stride; // Distance between reference samples.
  if (px.x) {
    left_border = &rec_ref[px.x - 1 + px.y * (LCU_WIDTH >> is_chroma)];
    left_stride = LCU_WIDTH >> is_chroma;
  } else {
    left_border = &left_ref[px.y];
    left_stride = 1;
  }

  // Generate left reference.
  if (luma_px->x > 0) {
    // Get the number of reference pixels based on the PU coordinate within the LCU.
    int px_available_left = num_ref_pixels_left[lcu_px.y / 4][lcu_px.x / 4] >> is_chroma;

    // Limit the number of available pixels based on block size and dimensions
    // of the picture.
    px_available_left = MIN(px_available_left, (int)(width.to_int() * 2));
    px_available_left = MIN(px_available_left, (pic_px->y - luma_px->y) >> is_chroma);

    // Copy pixels from coded CUs.
    for (int i = 0; i < px_available_left; ++i) {
      out_left_ref[i + 1] = left_border[i * left_stride];
    }
    // Extend the last pixel for the rest of the reference values.
    pixel_t nearest_pixel = out_left_ref[px_available_left];
    for (int i = px_available_left; i < width * 2; ++i) {
      out_left_ref[i + 1] = nearest_pixel;
    }
  } else {
    // If we are on the left edge, extend the first pixel of the top row.
    pixel_t nearest_pixel = luma_px->y > 0 ? top_border[0] : dc_val;
    for (int i = 0; i < width * 2; i++) {
      out_left_ref[i + 1] = nearest_pixel;
    }
  }

  // Generate top-left reference.
  if (luma_px->x > 0 && luma_px->y > 0) {
    // If the block is at an LCU border, the top-left must be copied from
    // the border that points to the LCUs 1D reference buffer.
    if (px.x == 0) {
      out_left_ref[0] = left_border[-1 * left_stride];
      out_top_ref[0] = left_border[-1 * left_stride];
    } else {
      out_left_ref[0] = top_border[-1];
      out_top_ref[0] = top_border[-1];
    }
  } else {
    // Copy reference clockwise.
    out_left_ref[0] = out_left_ref[1];
    out_top_ref[0] = out_left_ref[1];
  }

  // Generate top reference.
  if (luma_px->y > 0) {
    // Get the number of reference pixels based on the PU coordinate within the LCU.
    int px_available_top = num_ref_pixels_top[lcu_px.y / 4][lcu_px.x / 4] >> is_chroma;

    // Limit the number of available pixels based on block size and dimensions
    // of the picture.
    px_available_top = MIN(px_available_top, (int)(width.to_int() * 2));
    px_available_top = MIN(px_available_top, (pic_px->x - luma_px->x) >> is_chroma);

    // Copy all the pixels we can.
    for (int i = 0; i < px_available_top; ++i) {
      out_top_ref[i + 1] = top_border[i];
    }
    // Extend the last pixel for the rest of the reference values.
    pixel_t nearest_pixel = top_border[px_available_top - 1];
    for (int i = px_available_top; i < width * 2; ++i) {
      out_top_ref[i + 1] = nearest_pixel;
    }
  } else {
    // Extend nearest pixel.
    pixel_t nearest_pixel = luma_px->x > 0 ? left_border[0] : dc_val;
    for (int i = 0; i < width * 2; i++) {
      out_top_ref[i + 1] = nearest_pixel;
    }
  }
}

void main_build_reference_border(ac_channel< int_48 > &conf_in, output_port &conf_out, lcu_rec_t lcu_rec[LCU_COUNT], lcu_border_t lcu_border[LCU_COUNT]);

CCS_MAIN(int argc, char *argv[]) {
  lcu_t lcu = init_lcu();

  ac_channel< int_48 > config_in;
  output_port conf_out;

  lcu_rec_t lcu_rec[LCU_COUNT];
  lcu_border_t lcu_border[LCU_COUNT];

  one_bit simulation_successful = 1;

  vector2d_t xy_idxs[4] = {{0, 0}, {16, 0}, {0, 32}, {8, 16}};

  for (int xy_idx = 0; xy_idx < 4; xy_idx++) {
    for (int a = 0; a < 4 * 3; a++) {
      kvz_intra_references refs;
      memset(&refs, 0, sizeof(refs));

      uint_4 depth = a % 4;
      uint_4 color = a / 4;

      if (color != 0 && depth == 3) {
        continue;
      }

      const two_bit size = color && depth != 3 ? (two_bit)(depth + 1) : (two_bit)depth;
      const uint_6 block_size = (LCU_WIDTH / 2) >> size;

      // Luma cordinates
      vector2d_t luma_px = xy_idxs[xy_idx];
      vector2d_t pic_px = {416, 240}; // Frame size

      kvz_intra_build_reference(LOG2_LCU_WIDTH - depth - (!color ? 1 : 2), color, &luma_px, &pic_px, &lcu, &refs);

      std::cout << "\nLeft: " << std::hex << (int)refs.ref.left[0] << "\n";
      for (int i = 1; i < 2 * block_size + 1; ++i) {
        std::cout << std::hex << (int)refs.ref.left[i] << " ";
      }
      std::cout << "\nTop: " << std::hex << (int)refs.ref.top[0] << "\n";
      for (int i = 1; i < 2 * block_size + 1; ++i) {
        std::cout << std::hex << (int)refs.ref.top[i] << " ";
      }

      conf_t conf;
      conf.depth = depth;
      conf.color = color;
      conf.lcu_id = 0;
      conf.x_pos = luma_px.x % 64;
      conf.y_pos = luma_px.y % 64;
      conf.x_limit = uint_7(MIN(64, pic_px.x - luma_px.x));
      conf.y_limit = uint_7(MIN(64, pic_px.y - luma_px.y));
      conf.x_zero = one_bit(luma_px.x == 0);
      conf.y_zero = one_bit(luma_px.y == 0);

      config_in.write(to_ac_int_tb(conf));
      config_in.write(0x00000000);

      // Generate arrays
      {
        depth = (conf.color != COLOR_Y && depth != 3) ? (two_bit)(depth + 1) : (two_bit)depth;
        int width = 32 >> depth;

        pixel_t *rec_ptr;
        pixel_t *top_ptr;
        pixel_t *left_ptr;

        uint_6 x_pos = conf.x_pos;
        uint_6 y_pos = conf.y_pos;

        int rec_line_offset = 64;
        if (conf.color == 0) {
          rec_ptr = lcu.rec.y;
          top_ptr = lcu.top_ref.y;
          left_ptr = lcu.left_ref.y;
        } else if (conf.color == 1) {
          rec_ptr = lcu.rec.u;
          top_ptr = lcu.top_ref.u;
          left_ptr = lcu.left_ref.u;
          rec_line_offset = 32;
          x_pos >>= 1;
          y_pos >>= 1;
        } else if (conf.color == 2) {
          rec_ptr = lcu.rec.v;
          top_ptr = lcu.top_ref.v;
          left_ptr = lcu.left_ref.v;
          rec_line_offset = 32;
          x_pos >>= 1;
          y_pos >>= 1;
        }

        // Init pointers to LCUs reconstruction buffers
        slice_t *lcu_rec_top = &lcu_rec[conf.lcu_id].top[lcu_rec_t::colorOffset(conf.color)];
        slice_t *lcu_rec_left = &lcu_rec[conf.lcu_id].left[lcu_rec_t::colorOffset(conf.color)];

        slice_t *lcu_border_top = &lcu_border[conf.lcu_id].top[lcu_border_t::colorOffset(conf.color)];
        slice_t *lcu_border_left = &lcu_border[conf.lcu_id].left[lcu_border_t::colorOffset(conf.color)];

        // Init top borders pointer to point to the correct place in the correct reference array.
        pixel_t *tlPtr = &lcu_rec[conf.lcu_id].top_left[lcu_rec_t::colorTopLeftOffset(conf.color)];
        slice_t *top_border = conf.y_pos ? &lcu_rec_top[lcu_rec_t::address(x_pos)] : &lcu_border_top[lcu_border_t::address(x_pos) + 1];
        slice_t *left_border = conf.x_pos ? &lcu_rec_left[lcu_rec_t::address(y_pos)] : &lcu_border_left[lcu_border_t::address(y_pos) + 1];

        int x, y, i;
        for (x = 0, i = 0; x < width * 2 + 1; x += INTRA_WIDTH, i++) {
          slice_t temp = 0;
          for (int z = 0; z < INTRA_WIDTH; ++z) {
            if (y_pos == 0) {
              temp.set_slc(8 * z, top_ptr[1 + x + z]);
            } else {
              uint_16 index = x_pos + (y_pos - 1) * rec_line_offset + x + z;
              temp.set_slc(8 * z, rec_ptr[index]);
            }
          }
          top_border[i] = temp;
          if (x + INTRA_WIDTH >= 2 * width + 1)
            break;
        }

        for (y = 0, i = 0; y < width * 2 + 1; y += INTRA_WIDTH, i++) {
          slice_t temp = 0;
          for (int z = 0; z < INTRA_WIDTH; ++z) {
            if (x_pos == 0) {
              temp.set_slc(8 * z, left_ptr[1 + y + z]);
            } else {
              temp.set_slc(8 * z, rec_ptr[y_pos * rec_line_offset + x_pos - 1 + y * rec_line_offset + z * rec_line_offset]);
            }
          }
          left_border[i] = temp;
          if (y + INTRA_WIDTH >= 2 * width + 1)
            break;
        }
        if (y_pos == 0) {
          tlPtr[0] = top_ptr[x_pos];
        } else if (x_pos == 0) {
          tlPtr[0] = left_ptr[y_pos];
        } else {
          uint_16 index = (y_pos - 1) * rec_line_offset - 1;
          tlPtr[0] = rec_ptr[index];
        }
      }

      pixel_t dout_top[128], dout_left[128];
      CCS_DESIGN(main_build_reference_border)
      (config_in, conf_out, lcu_rec, lcu_border);

      ac_int< 88, false > out = conf_out.read();
      pixel_t zero = out.slc< 8 >(64);

      dout_left[0] = out.slc< 8 >(72);
      dout_top[0] = out.slc< 8 >(80);

      cout << "\n\nFirst: " << std::hex << (int)zero << "  " << (int)out.slc< 8 >(72) << " " << (int)out.slc< 8 >(80) << endl;

      for (int y = 1; y < 2 * block_size + 1; y += INTRA_WIDTH) {
        slice_t output = conf_out.read();

        for (int x = 0; x < INTRA_WIDTH; ++x)
          dout_left[y + x] = (pixel_t)output.slc< 8 >(8 * x);
      }
      for (int y = 1; y < 2 * block_size + 1; y += INTRA_WIDTH) {
        slice_t output = conf_out.read();

        for (int x = 0; x < INTRA_WIDTH; ++x)
          dout_top[y + x] = (pixel_t)output.slc< 8 >(8 * x);
      }

      std::cout << "\nLeft: " << std::hex << (int)zero << endl;
      for (uint_8 y = 0; y < 2 * block_size; ++y) {
        std::cout << (int)dout_left[y] << " ";

        if (dout_left[y] != refs.ref.left[y + 1])
          simulation_successful = 0;
      }

      std::cout << "\nTop: " << std::hex << (int)zero << endl;
      for (uint_8 y = 0; y < 2 * block_size; ++y) {
        std::cout << (int)dout_top[y] << " ";

        if (dout_top[y] != refs.ref.top[y + 1])
          simulation_successful = 0;
      }
      if (simulation_successful == 0) {
        break;
      }
    }
  }
  std::cout << std::endl;
  if (simulation_successful == 1) {
    cout << "## SIMULATION PASSED ##" << endl;
  } else {
    cout << "## SIMULATION FAILED ##" << endl;
  }

  CCS_RETURN(0);
}
