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

#include "iostream"
#include "stdlib.h"
#include <mc_scverify.h>

using namespace std;

#define SWAP(a, b, swaptype)                                                                                                                                                                           \
  {                                                                                                                                                                                                    \
    swaptype tempval;                                                                                                                                                                                  \
    tempval = a;                                                                                                                                                                                       \
    a = b;                                                                                                                                                                                             \
    b = tempval;                                                                                                                                                                                       \
  }
#define ABS(a) (((a) > 0) ? (a) : -(a))

static uint_8 golden_src1[65] = {245, 204, 118, 71,  49,  188, 241, 245, 214, 124, 87, 125, 220, 11,  53, 74,  55,  81,  68,  35,  162, 255, 90,  150, 160, 81,  200, 94,  226, 236, 203, 184, 202,
                                 114, 47,  125, 214, 183, 83,  182, 145, 186, 125, 48, 13,  174, 192, 99, 254, 103, 221, 102, 102, 94,  22,  126, 4,   121, 120, 226, 228, 99,  230, 14,  18};

static uint_8 golden_src2[65] = {245, 19, 211, 238, 45,  202, 217, 123, 8,  231, 249, 215, 57, 15, 40,  88, 236, 121, 37,  184, 68, 2,   73, 122, 161, 135, 33, 247, 85,  132, 111, 107, 32,
                                 56,  79, 23,  208, 190, 245, 183, 225, 38, 139, 146, 214, 45, 18, 235, 81, 254, 30,  237, 157, 93, 135, 8,  129, 190, 198, 67, 156, 250, 226, 79,  150};

const int_8 kvz_g_convert_to_bit[LCU_WIDTH + 1] = {-1, -1, -1, -1, 0,  -1, -1, -1, 1,  -1, -1, -1, -1, -1, -1, -1, 2,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3,
                                                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4};

typedef uint_8 kvz_pixel;

static void kvz_angular_pred_generic(const int_8 log2_width, const int_8 intra_mode, const kvz_pixel *const in_ref_above, const kvz_pixel *const in_ref_left, kvz_pixel *const dst);

static void intra_post_process_angular(unsigned width, unsigned stride, const kvz_pixel *ref, kvz_pixel *block);

static void intra_pred_filtered_dc(const int_8 log2_width, const kvz_pixel *const ref_top, const kvz_pixel *const ref_left, kvz_pixel *const out_block);

static void intra_pred_dc(const int_8 log2_width, const kvz_pixel *const ref_top, const kvz_pixel *const ref_left, kvz_pixel *const out_block);

static void kvz_intra_pred_planar_generic(const int_8 log2_width, const kvz_pixel *const ref_top, const kvz_pixel *const ref_left, kvz_pixel *const dst);

void kvz_intra_predict(int_8 log2_width, int_8 mode, int_8 color, const kvz_pixel *const in_ref_above, const kvz_pixel *const in_ref_left, kvz_pixel *dst) {
  const int_8 width = 1 << log2_width;

  if (mode == 0) {
    kvz_intra_pred_planar_generic(log2_width, in_ref_above, in_ref_left, dst);
  } else if (mode == 1) {
    // Do extra post filtering for edge pixels of luma DC mode.
    if (color == 0 && width < 32) {
      intra_pred_filtered_dc(log2_width, in_ref_above, in_ref_left, dst);
    } else {
      intra_pred_dc(log2_width, in_ref_above, in_ref_left, dst);
    }
  } else {
    kvz_angular_pred_generic(log2_width, mode, in_ref_above, in_ref_left, dst);
    if (color == 0 && width < 32) {
      if (mode == 10) {
        intra_post_process_angular(width, 1, in_ref_above, dst);
      } else if (mode == 26) {
        intra_post_process_angular(width, width, in_ref_left, dst);
      }
    }
  }
}

static void kvz_angular_pred_generic(const int_8 log2_width, const int_8 intra_mode, const kvz_pixel *const in_ref_above, const kvz_pixel *const in_ref_left, kvz_pixel *const dst) {
  static const int_8 modedisp2sampledisp[9] = {0, 2, 5, 9, 13, 17, 21, 26, 32};
  static const int_16 modedisp2invsampledisp[9] = {0, 4096, 1638, 910, 630, 482, 390, 315, 256}; // (256 * 32) / sampledisp

  // Temporary buffer for modes 11-25.
  // It only needs to be big enough to hold indices from -width to width-1.
  kvz_pixel tmp_ref[2 * 32];
  const int_8 width = 1 << log2_width;

  // Whether to swap references to always project on the left reference row.
  const bool vertical_mode = intra_mode >= 18;
  // Modes distance to horizontal or vertical mode.
  const int_8 mode_disp = vertical_mode ? intra_mode - 26 : 10 - intra_mode;
  int_8 mode_disp_abs = mode_disp;
  if (mode_disp_abs < 0) {
    mode_disp_abs = -mode_disp_abs;
  }
  // Sample displacement per column in fractions of 32.
  const int_8 sample_disp = (mode_disp < 0 ? -1 : 1) * modedisp2sampledisp[mode_disp_abs];

  // Pointer for the reference we are interpolating from.
  const kvz_pixel *ref_main;
  // Pointer for the other reference.
  const kvz_pixel *ref_side;

  // Set ref_main and ref_side such that, when indexed with 0, they point to
  // index 0 in block coordinates.
  if (sample_disp < 0) {
    // Negative sample_disp means, we need to use both references.

    ref_side = (vertical_mode ? in_ref_left : in_ref_above) + 1;
    ref_main = (vertical_mode ? in_ref_above : in_ref_left) + 1;

    // Move the reference pixels to start from the middle to the later half of
    // the tmp_ref, so there is room for negative indices.
    for (int_8 x = -1; x < width; ++x) {
      tmp_ref[x + width] = ref_main[x];
    }
    // Get a pointer to block index 0 in tmp_ref.
    ref_main = &tmp_ref[width];

    // Extend the side reference to the negative indices of main reference.
    int_32 col_sample_disp = 128; // rounding for the ">> 8"
    int_16 inv_abs_sample_disp = modedisp2invsampledisp[mode_disp_abs];
    int_8 most_negative_index = (width * sample_disp) >> 5;
    for (int_8 x = -2; x >= most_negative_index; --x) {
      col_sample_disp += inv_abs_sample_disp;
      int_8 side_index = col_sample_disp >> 8;
      tmp_ref[x + width] = ref_side[side_index - 1];
    }
  } else {
    // sample_disp >= 0 means we don't need to refer to negative indices,
    // which means we can just use the references as is.
    ref_main = (vertical_mode ? in_ref_above : in_ref_left) + 1;
    ref_side = (vertical_mode ? in_ref_left : in_ref_above) + 1;
  }

  if (sample_disp != 0) {
    // The mode is not horizontal or vertical, we have to do interpolation.

    int_16 delta_pos = 0;
    for (int_8 y = 0; y < width; ++y) {
      delta_pos += sample_disp;
      int_8 delta_int = delta_pos >> 5;
      int_8 delta_fract = delta_pos & (32 - 1);

      if (delta_fract) {
        // Do linear filtering
        for (int_8 x = 0; x < width; ++x) {
          kvz_pixel ref1 = ref_main[x + delta_int];
          kvz_pixel ref2 = ref_main[x + delta_int + 1];
          dst[y * width + x] = ((32 - delta_fract) * ref1 + delta_fract * ref2 + 16) >> 5;
        }
      } else {
        // Just copy the integer samples
        for (int_8 x = 0; x < width; x++) {
          dst[y * width + x] = ref_main[x + delta_int];
        }
      }
    }
  } else {
    // Mode is horizontal or vertical, just copy the pixels.

    for (int_8 y = 0; y < width; ++y) {
      for (int_8 x = 0; x < width; ++x) {
        dst[y * width + x] = ref_main[x];
      }
    }
  }

  // Flip the block if this is was a horizontal mode.
  if (!vertical_mode) {
    for (int_8 y = 0; y < width - 1; ++y) {
      for (int_8 x = y + 1; x < width; ++x) {
        SWAP(dst[y * width + x], dst[x * width + y], kvz_pixel);
      }
    }
  }
}

static void intra_post_process_angular(unsigned width, unsigned stride, const kvz_pixel *ref, kvz_pixel *block) {
  kvz_pixel ref2 = ref[0];
  for (unsigned i = 0; i < width; i++) {
    kvz_pixel val = block[i * stride];
    kvz_pixel ref1 = ref[i + 1];

    if (val + ((ref1 - ref2) >> 1) > 255) {
      block[i * stride] = 255;
    } else if (val + ((ref1 - ref2) >> 1) < 0) {
      block[i * stride] = 0;
    } else {
      block[i * stride] = val + ((ref1 - ref2) >> 1);
    }
  }
}

static void intra_pred_filtered_dc(const int_8 log2_width, const kvz_pixel *const ref_top, const kvz_pixel *const ref_left, kvz_pixel *const out_block) {
  assert(log2_width >= 2 && log2_width <= 5);

  const int_8 width = 1 << log2_width;

  int_16 sum = 0;
  for (int_8 i = 0; i < width; ++i) {
    sum += ref_top[i + 1];
    sum += ref_left[i + 1];
  }

  const kvz_pixel dc_val = (sum + width) >> (log2_width + 1);

  // Filter top-left with ([1 2 1] / 4)
  out_block[0] = (ref_left[1] + 2 * dc_val + ref_top[1] + 2) / 4;

  // Filter rest of the boundary with ([1 3] / 4)
  for (int_8 x = 1; x < width; ++x) {
    out_block[x] = (ref_top[x + 1] + 3 * dc_val + 2) / 4;
  }
  for (int_8 y = 1; y < width; ++y) {
    out_block[y * width] = (ref_left[y + 1] + 3 * dc_val + 2) / 4;
    for (int_8 x = 1; x < width; ++x) {
      out_block[y * width + x] = dc_val;
    }
  }
}

static void intra_pred_dc(const int_8 log2_width, const kvz_pixel *const ref_top, const kvz_pixel *const ref_left, kvz_pixel *const out_block) {
  int_8 width = 1 << log2_width;

  int_16 sum = 0;
  for (int_8 i = 0; i < width; ++i) {
    sum += ref_top[i + 1];
    sum += ref_left[i + 1];
  }

  const kvz_pixel dc_val = (sum + width) >> (log2_width + 1);
  const int_16 block_size = 1 << (log2_width * 2);

  for (int_16 i = 0; i < block_size; ++i) {
    out_block[i] = dc_val;
  }
}

static void kvz_intra_pred_planar_generic(const int_8 log2_width, const kvz_pixel *const ref_top, const kvz_pixel *const ref_left, kvz_pixel *const dst) {
  const int_8 width = 1 << log2_width;
  const kvz_pixel top_right = ref_top[width + 1];
  const kvz_pixel bottom_left = ref_left[width + 1];

#if 1
  // Unoptimized version for reference.
  for (int y = 0; y < width; ++y) {
    for (int x = 0; x < width; ++x) {
      int_16 hor = (width - 1 - x) * ref_left[y + 1] + (x + 1) * top_right;
      int_16 ver = (width - 1 - y) * ref_top[x + 1] + (y + 1) * bottom_left;
      dst[y * width + x] = (ver + hor + width) >> (log2_width + 1);
    }
  }
#else
  int_16 top[32];
  for (int i = 0; i < width; ++i) {
    top[i] = ref_top[i + 1] << log2_width;
  }

  for (int y = 0; y < width; ++y) {
    int_16 hor = (ref_left[y + 1] << log2_width) + width;
    for (int x = 0; x < width; ++x) {
      hor += top_right - ref_left[y + 1];
      top[x] += bottom_left - ref_top[x + 1];
      dst[y * width + x] = (hor + top[x]) >> (log2_width + 1);
    }
  }
#endif
}

static const uint_6 ang_table[9] = {0, 2, 5, 9, 13, 17, 21, 26, 32};
static const uint_4 dir_modes[35] = {0, 0, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8};

void main_ip_get_dc(input_port_t &conf_in, pixel_slc_t ref[4][64 / 4][PARALLEL_PIX / 2], output_port_t &pred_out);

CCS_MAIN(int argc, char *argv[]) {
  static input_port_t data_in;
  static output_port_t data_out;

  static kvz_pixel ref_dst[32 * 32];

  static kvz_pixel rnd_top[65];
  static kvz_pixel rnd_left[65];

  const kvz_pixel *top_ref = golden_src2;
  const kvz_pixel *left_ref = golden_src1;

  const int mode = 1;

  one_bit error = 0;
  one_bit simulation_successful = 1;

  static pixel_slc_t ref[4][64 / 4][PARALLEL_PIX / 2];

  for (int rnd = 0; rnd < 32; rnd++, error = 0) {
    if (rnd) {
      srand(rnd);
      for (int i = 1; i < 65; i++) {
        rnd_top[i] = rand() % 256;
        rnd_left[i] = rand() % 256;
      }

      rnd_top[0] = rnd_left[0] = rand() % 256;

      top_ref = rnd_top;
      left_ref = rnd_left;
    }

    uint_4 depth = 0;
    for (depth = 0; depth < 4; depth++, error = 0) {
      uint_4 color = 0;
      for (color = 0; color < 3; ++color, error = 0) {
        const two_bit size = (color && depth != 3) ? (two_bit)(depth + 1) : (two_bit)depth;
        const uint_6 width = (LCU_WIDTH / 2) >> size;

        ip_conf_t conf;
        conf.ip_id = 0;
        conf.size = size;
        conf.filter = !color && depth != 0;

        kvz_intra_predict(kvz_g_convert_to_bit[width] + 2, mode, color, top_ref, left_ref, ref_dst);

        uint_16 ref_sum = width;

        for (uint_32 i = 0; i < width; ++i) {
          ref_sum += left_ref[i + 1];
          for (int j = 0; j < PARALLEL_PIX / 2; ++j)
            ref[conf.ip_id][i >> 2][j].set_slc(8 * i.slc< 2 >(0), left_ref[i + 1]);
        }
        for (uint_32 i = 0; i < width; ++i) {
          ref_sum += top_ref[i + 1];
          for (int j = 0; j < PARALLEL_PIX / 2; ++j)
            ref[conf.ip_id][(i + 32) >> 2][j].set_slc(8 * i.slc< 2 >(0), top_ref[i + 1]);
        }

        conf.dc_val = ref_sum >> (6 - size);

        data_in.write(to_ac_int(conf));

        CCS_DESIGN(main_ip_get_dc)
        (data_in, ref, data_out);

        for (uint_6 y = 0; y < width; ++y) {
          for (uint_6 x = 0; x < width; x += PARALLEL_PIX) {
            output_t dc = data_out.read();

            for (int j = 0; j < PARALLEL_PIX; ++j) {
              pixel_t pred = dc.slc< 8 >(j * 8);
              if (pred != ref_dst[y * width + x + j])
                error = 1;
            }
          }
        }

        if (error) {
          simulation_successful = 0;
          cout << "ERROR: Width " << width << ", Color " << color << ", Mode " << mode << endl;
        }
        if (!simulation_successful)
          break;
      }
      if (!simulation_successful)
        break;
    }
    if (!simulation_successful)
      break;
  }

  if (simulation_successful) {
    cout << "## SIMULATION PASSED ##" << endl;
  } else {
    cout << "## SIMULATION FAILED ##" << endl;
  }

  CCS_RETURN(0);
}
