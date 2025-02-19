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

#ifndef GLOBAL_H_
#define GLOBAL_H_

#include "ac_channel.h"
#include "ac_fixed.h"
#include "ac_int.h"

#include "ctl.h"
#pragma hls_noglobals

#ifndef __SYNTHESIS__
#include <iostream>
#endif

// #define DSP
#ifdef DSP
#include "altera/accel.h"
#endif

#define MAX_WIDTH 32
#define LCU_COUNT 16
#define REF_WIDTH 4
#define PARALLEL_PIX 4

enum COLOR { COLOR_Y = 0, COLOR_U = 1, COLOR_V = 2, COLOR_ALL = 3 };

#define LCU_WIDTH 64
#define LCU_LUMA_SIZE (LCU_WIDTH * LCU_WIDTH)
#define LCU_CHROMA_SIZE (LCU_WIDTH * LCU_WIDTH >> 2)

/* Platform-specific type definition */
typedef ac_int< 8, false > pixel_t;
typedef ac_int< 8 * REF_WIDTH, false > pixel_slc_t;
typedef ac_int< 4, false > uint_4;
typedef ac_int< 5, false > uint_5;
typedef ac_int< 6, false > uint_6;
typedef ac_int< 8, false > uint_8;
typedef ac_int< 9, false > uint_9;
typedef ac_int< 12, false > uint_12;
typedef ac_int< 16, false > uint_16;
typedef ac_int< 18, false > uint_18;
typedef ac_int< 32, false > uint_32;
typedef ac_int< 64, false > uint_64;
typedef ac_int< 8, true > int_8;
typedef ac_int< 9, true > int_9;
typedef ac_int< 16, true > int_16;
typedef ac_int< 32, true > int_32;
typedef ac_int< 64, true > int_64;

typedef ac_int< 1, false > one_bit;
typedef ac_int< 2, false > two_bit;

typedef ac_int< PARALLEL_PIX * 8, false > input_t;
typedef ac_int< PARALLEL_PIX * 8, false > output_t;

typedef ac_channel< input_t > ip_port_t;

typedef ac_int< 36 * 8 * PARALLEL_PIX + 1, false > preds_t;
typedef ac_channel< preds_t > pred_port_t;

class conf_t {
public:
  typedef ac_int< 64, false > ac_t;
  typedef ac_channel< ac_t > port_t;

  uint_4 lcu_id;
  two_bit depth;
  two_bit color;

  uint_6 x_pos;
  uint_6 y_pos;

  uint_8 intra_mode; // Lamba in input
  uint_8 qp_scaled;

  // Used only in Sads block
  uint_6 intra_pred0;
  uint_6 intra_pred1;
  uint_6 intra_pred2;
  uint_6 intra_mode_chroma;
  uint_8 uc_state;

  two_bit size() const { return (color != COLOR_Y && depth != 3) ? two_bit(depth + 1) : depth; };

  uint_6 width() const { return (LCU_WIDTH / 2) >> this->size(); };

  conf_t() { conf_t(0); }
  conf_t(const ac_t &ac) {
    lcu_id = ac.slc< 4 >(0);
    depth = ac.slc< 2 >(4);
    color = ac.slc< 2 >(6);

    x_pos = uint_6(ac.slc< 4 >(8)) << 2;
    y_pos = uint_6(ac.slc< 4 >(12)) << 2;

    intra_mode = ac.slc< 8 >(16);
    qp_scaled = ac.slc< 8 >(24);

    intra_pred0 = ac.slc< 6 >(32);
    intra_pred1 = ac.slc< 6 >(38);
    intra_pred2 = ac.slc< 6 >(44);
    intra_mode_chroma = ac.slc< 6 >(50);
    uc_state = ac.slc< 8 >(56);
  }

  ac_t toInt() const {
    ac_t output;
    output.set_slc(0, lcu_id);
    output.set_slc(4, depth);
    output.set_slc(6, color);

    output.set_slc(8, uint_4(x_pos >> 2));
    output.set_slc(12, uint_4(y_pos >> 2));

    output.set_slc(16, intra_mode);
    output.set_slc(24, qp_scaled);

#ifndef __SYNTHESIS__
    output.set_slc(32, intra_pred0);
    output.set_slc(38, intra_pred1);
    output.set_slc(44, intra_pred2);
    output.set_slc(50, intra_mode_chroma);

    output.set_slc(56, uc_state);
#endif
    return output;
  }
};

struct conf_tt {
  uint_4 lcu_id;
  two_bit depth;
  two_bit color;

  uint_6 x_pos;
  uint_6 y_pos;

  uint_8 intra_mode;
  uint_8 qp_scaled;

  two_bit size; // Local variable
};

static conf_tt conf_to_struct(const int_32 &ac_int) {
  conf_tt output;
  output.lcu_id = ac_int.slc< 4 >(0);
  output.depth = ac_int.slc< 2 >(4);
  output.color = ac_int.slc< 2 >(6);

  output.x_pos = uint_6(ac_int.slc< 4 >(8)) << 2;
  output.y_pos = uint_6(ac_int.slc< 4 >(12)) << 2;

  output.intra_mode = ac_int.slc< 8 >(16);
  output.qp_scaled = ac_int.slc< 8 >(24);

  output.size = (output.color && output.depth != 3) ? two_bit(output.depth + 1) : output.depth;
  return output;
}

struct lcu_yuv_t {
  pixel_slc_t ref[(LCU_LUMA_SIZE + 2 * LCU_CHROMA_SIZE) / REF_WIDTH];

  // Round coordinate to address
  static ac_int< ac::log2_ceil< (LCU_LUMA_SIZE + 2 * LCU_CHROMA_SIZE) / REF_WIDTH >::val, false > address(unsigned coordinate) { return coordinate >> ac::log2_ceil< REF_WIDTH >::val; }

  static unsigned colorOffset(two_bit color) { return color == COLOR_Y ? y_offset : (color == COLOR_U ? u_offset : v_offset); }

private:
  static const unsigned y_offset = 0;
  static const unsigned u_offset = 4096;
  static const unsigned v_offset = 5120;
};

#define BIT_DEPTH 8
#define PIXEL_MIN 0
#define PIXEL_MAX (1 << BIT_DEPTH)

/* CONFIG VARIABLES */
#define LCU_WIDTH 64 /*!< Largest Coding Unit (IT'S 64x64, DO NOT TOUCH!) */

#define MAX_INTER_SEARCH_DEPTH 3
#define MIN_INTER_SEARCH_DEPTH 0

#define MAX_INTRA_SEARCH_DEPTH 4 /*!< Max search depth -> min block size (3 == 8x8) */
#define MIN_INTRA_SEARCH_DEPTH 1 /*!< Min search depth -> max block size (0 == 64x64) */

#define MAX_DEPTH 3 /*!< smallest CU is LCU_WIDTH>>MAX_DEPTH */
#define MAX_PU_DEPTH 4
#define MIN_SIZE 3           /*!< log2_min_coding_block_size */
#define CU_MIN_SIZE_PIXELS 8 /*!< pow(2, MIN_SIZE) */

#define TR_DEPTH_INTRA 2
#define TR_DEPTH_INTER 2

#define ENABLE_PCM 0 /*!< Setting to 1 will enable using PCM blocks (current intra-search does not consider PCM) */
#define ENABLE_SIGN_HIDING 1

#define ENABLE_TEMPORAL_MVP 0 /*!< Enable usage of temporal Motion Vector Prediction */

#define OPTIMIZATION_SKIP_RESIDUAL_ON_THRESHOLD 0 /*!< skip residual coding when it's under _some_ threshold */

/* END OF CONFIG VARIABLES */

#define MAX_REF_PIC_COUNT 16
#define DEFAULT_REF_PIC_COUNT 3

#define AMVP_MAX_NUM_CANDS 2
#define AMVP_MAX_NUM_CANDS_MEM 3
#define MRG_MAX_NUM_CANDS 5

/* Some tools */
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define CLIP(low, high, value) MAX((low), MIN((high), (value)))
#define SWAP(a, b, swaptype)                                                                                                                                                                           \
  {                                                                                                                                                                                                    \
    swaptype tempval;                                                                                                                                                                                  \
    tempval = a;                                                                                                                                                                                       \
    a = b;                                                                                                                                                                                             \
    b = tempval;                                                                                                                                                                                       \
  }
#define CU_WIDTH_FROM_DEPTH(depth) (LCU_WIDTH >> depth)
#define NO_SCU_IN_LCU(no_lcu) ((no_lcu) << MAX_DEPTH)
#define WITHIN(val, min_val, max_val) ((min_val) <= (val) && (val) <= (max_val))
#define UNREFERENCED_PARAMETER(p) (p)
#define PU_INDEX(x_pu, y_pu) (((x_pu) % 2) + 2 * ((y_pu) % 2))

#define LOG2_LCU_WIDTH 6
// CU_TO_PIXEL = y * lcu_width * pic_width + x * lcu_width
#define CU_TO_PIXEL(x, y, depth, width) (((y) << (LOG2_LCU_WIDTH - (depth))) * (width) + ((x) << (LOG2_LCU_WIDTH - (depth))))
// #define SIGN3(x) ((x) > 0) ? +1 : ((x) == 0 ? 0 : -1)
#define SIGN3(x) (((x) > 0) - ((x) < 0))

#define VERSION_STRING "0.3.0"

// #define VERBOSE 1

#define SAO_ABS_OFFSET_MAX ((1 << (MIN(BIT_DEPTH, 10) - 5)) - 1)

#define SIZE_2Nx2N 0
#define SIZE_2NxN 1
#define SIZE_Nx2N 2
#define SIZE_NxN 3
#define SIZE_NONE 15

// These are for marking incomplete implementations that break if slices or
// tiles are used with asserts. They should be set to 1 if they are ever
// implemented.
#define USE_SLICES 0
#define USE_TILES 0

#define FREE_POINTER(pointer)                                                                                                                                                                          \
  {                                                                                                                                                                                                    \
    free(pointer);                                                                                                                                                                                     \
    pointer = NULL;                                                                                                                                                                                    \
  }
#define MOVE_POINTER(dst_pointer, src_pointer)                                                                                                                                                         \
  {                                                                                                                                                                                                    \
    dst_pointer = src_pointer;                                                                                                                                                                         \
    src_pointer = NULL;                                                                                                                                                                                \
  }

#ifndef MAX_INT
#define MAX_INT 0x7FFFFFFF
#endif
#ifndef MAX_INT64
#define MAX_INT64 0x7FFFFFFFFFFFFFFFLL
#endif
#ifndef MAX_DOUBLE
#define MAX_DOUBLE 1.7e+308
#endif

#endif
