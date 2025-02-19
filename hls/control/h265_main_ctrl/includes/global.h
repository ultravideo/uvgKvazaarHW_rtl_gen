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
using namespace std;
#endif

#define LCU_COUNT 16
#define MAX_WIDTH 32

#define Q 15
#define CTU_INST_STACK_SIZE 512

enum OP_CODE {
  IP,  // Intra Prediction
  CMP, // Compare
  STR, // Start
  END  // End
};

enum COLOR { COLOR_Y = 0, COLOR_U = 1, COLOR_V = 2, COLOR_ALL = 3 };

/* CONFIG VARIABLES */
#define LCU_WIDTH 64 /*!< Largest Coding Unit (IT'S 64x64, DO NOT TOUCH!) */

/* Platform-specific type definition */
typedef ac_int< 8, false > pixel_t;
typedef ac_int< 16, true > coeff_t;

typedef ac_int< 3, false > uint_3;
typedef ac_int< 4, false > uint_4;
typedef ac_int< 5, false > uint_5;
typedef ac_int< 6, false > uint_6;
typedef ac_int< 7, false > uint_7;
typedef ac_int< 8, false > uint_8;
typedef ac_int< 9, false > uint_9;
typedef ac_int< 10, false > uint_10;
typedef ac_int< 12, false > uint_12;
typedef ac_int< 14, false > uint_14;
typedef ac_int< 16, false > uint_16;
typedef ac_int< 18, false > uint_18;
typedef ac_int< 19, false > uint_19;
typedef ac_int< 20, false > uint_20;
typedef ac_int< 24, false > uint_24;
typedef ac_int< 26, false > uint_26;
typedef ac_int< 32, false > uint_32;
typedef ac_int< 40, false > uint_40;
typedef ac_int< 64, false > uint_64;
typedef ac_int< 3, true > int_3;
typedef ac_int< 4, true > int_4;
typedef ac_int< 8, true > int_8;
typedef ac_int< 9, true > int_9;
typedef ac_int< 16, true > int_16;
typedef ac_int< 17, true > int_17;
typedef ac_int< 18, true > int_18;
typedef ac_int< 24, true > int_24;
typedef ac_int< 32, true > int_32;
typedef ac_int< 48, true > int_48;
typedef ac_int< 56, true > int_56;
typedef ac_int< 64, true > int_64;
typedef ac_int< 96, true > int_96;
typedef ac_int< 192, true > int_192;

typedef ac_int< 1, false > one_bit;
typedef ac_int< 2, false > two_bit;

typedef uint_8 cabac_ctx_t;
typedef ac_int< LCU_COUNT, false > irq_t;

#define INT_BITS 32
#define DECIMAL_BITS 8
#define FIXED_T_BITS (INT_BITS + DECIMAL_BITS)

typedef ac_fixed< FIXED_T_BITS, INT_BITS, true > fixed_t;
typedef ac_fixed< FIXED_T_BITS, INT_BITS, false > ufixed_t;

template < typename T, int N > struct chanStruct {
  T Data[N];
};
template < typename T, int N, int M > struct chanStruct2D {
  T Data[N][M];
};

template < unsigned N > struct lcu_yuv_t {
  typedef ac_int< N > slice_t;

  slice_t y[64 * 64 * 8 / N];
  slice_t u[32 * 32 * 8 / N];
  slice_t v[32 * 32 * 8 / N];
};

static const uint16 cbf_masks[4] = {0x0f, 0x07, 0x03, 0x1};

class cu_info_t {
public:
  void set_cu_info(uint6 mode, uint6 mode_chroma, two_bit depth, one_bit trskip, uint16 cbf) {
    uint32 info;
    info.set_slc(0, mode);
    info.set_slc(8, mode_chroma);

    info.set_slc(6, depth);
    info.set_slc(14, trskip);

    info.set_slc(16, cbf);

    cu_info = info;
  }

  void set_cu_info(uint32 info) { cu_info = info; }

#define NUM_CBF_DEPTHS 5
  /**
   * Check if CBF in a given level >= depth is true.
   */
  bool cbf_is_set(two_bit depth, two_bit color) const {
    // SW Ref: return (cbf & (cbf_masks[depth] << (NUM_CBF_DEPTHS * plane))) != 0;
    return (this->get_cbf() & (cbf_masks[depth] << (NUM_CBF_DEPTHS * color))) != 0;
  }

  /**
   * Check if CBF in a given level >= depth is true.F
   */
  bool cbf_is_set_any(two_bit depth) const { return cbf_is_set(depth, COLOR_Y) || cbf_is_set(depth, COLOR_U) || cbf_is_set(depth, COLOR_V); }

  /**
   * Set CBF in a level to true.
   */
  void cbf_set(two_bit depth, two_bit color) {
    // SW Ref: *cbf |= (0x10 >> depth) << (NUM_CBF_DEPTHS * plane);
    this->set_cbf(this->get_cbf() | (uint16(0x08 >> depth) << (NUM_CBF_DEPTHS * color)));
  }

  /**
   * Set CBF in a levels <= depth to false.
   */
  void cbf_clear(two_bit depth, two_bit color) {
    // SW Ref: *cbf &= ~(cbf_masks[depth] << (NUM_CBF_DEPTHS * plane));
    ac_int< 16, false > cbf_temp = this->get_cbf();
#pragma hls_unroll yes
    for (ac_int< 5, false > i = 0; i < 16; i++) {
      if ((i >= (NUM_CBF_DEPTHS * color)) && (i < (NUM_CBF_DEPTHS * color + (4 - depth)))) {
        cbf_temp[i] = 0;
      }
    }
    this->set_cbf(cbf_temp);
    // this->set_cbf(this->get_cbf() & ~(cbf_masks[depth] << (NUM_CBF_DEPTHS * color)));
  }

  void cbf_clear_color(two_bit color) {
    // SW Ref: *cbf &= ~(cbf_masks[depth] << (NUM_CBF_DEPTHS * plane));
    ac_int< 16, false > cbf_temp = this->get_cbf();
#pragma hls_unroll yes
    for (ac_int< 5, false > i = 0; i < 16; i++) {
      if (i >= color * NUM_CBF_DEPTHS && i < color * NUM_CBF_DEPTHS + 5) {
        cbf_temp[i] = 0;
      }
    }
    this->set_cbf(cbf_temp);
    // this->set_cbf(this->get_cbf() & ~(cbf_masks[depth] << (NUM_CBF_DEPTHS * color)));
  }

  /*
   * Variables.
   */
  two_bit depth() const { return cu_info.slc< 2 >(6); }
  uint6 intra_mode() const { return cu_info.slc< 6 >(0); }
  uint6 intra_mode_chroma() const { return cu_info.slc< 6 >(8); }

  uint32 get_cu() const { return cu_info; }
  uint16 get_cbf() const { return cu_info.slc< 16 >(16); }
  void set_cbf(uint16 cbf) { cu_info.set_slc(16, cbf); }

private:
  uint32 cu_info;
};

class cu_simple_info_t {
public:
  cu_simple_info_t() {}
  cu_simple_info_t(const cu_info_t &cu) : cu_info(cu.get_cu()) {}

  uint2 depth() const { return cu_info.slc< 2 >(6); }
  uint6 intra_mode() const { return cu_info.slc< 6 >(0); }

private:
  uint8 cu_info;
};

struct cu_info_mem_t {
  cu_info_t cu[256];
  cu_simple_info_t cu_hor[16];
  cu_simple_info_t cu_ver[16];
};

struct cu_info_str_mem_t {
  cu_simple_info_t cu[256];
  cu_simple_info_t cu_hor[16];
};

struct cabac_t {
  cabac_ctx_t cabac[12];

  cabac_ctx_t *trans_subdiv_model() { return &cabac[0]; }  // Size 3
  cabac_ctx_t *qt_cbf_model_luma() { return &cabac[3]; }   // Size 1
  cabac_ctx_t *qt_cbf_model_chroma() { return &cabac[4]; } // Size 1
  cabac_ctx_t *intra_mode_model() { return &cabac[5]; }    // Size 1
  cabac_ctx_t *chroma_pred_model() { return &cabac[6]; }   // Size 1
  cabac_ctx_t *split_flag_model() { return &cabac[7]; }    // Size 3
  cabac_ctx_t *part_size_model() { return &cabac[10]; }    // Size 1
};

const uint18 entropy_bits[128] = {0x08000, 0x08000, 0x076da, 0x089a0, 0x06e92, 0x09340, 0x0670a, 0x09cdf, 0x06029, 0x0a67f, 0x059dd, 0x0b01f, 0x05413, 0x0b9bf, 0x04ebf, 0x0c35f,
                                  0x049d3, 0x0ccff, 0x04546, 0x0d69e, 0x0410d, 0x0e03e, 0x03d22, 0x0e9de, 0x0397d, 0x0f37e, 0x03619, 0x0fd1e, 0x032ee, 0x106be, 0x02ffa, 0x1105d,
                                  0x02d37, 0x119fd, 0x02aa2, 0x1239d, 0x02836, 0x12d3d, 0x025f2, 0x136dd, 0x023d1, 0x1407c, 0x021d2, 0x14a1c, 0x01ff2, 0x153bc, 0x01e2f, 0x15d5c,
                                  0x01c87, 0x166fc, 0x01af7, 0x1709b, 0x0197f, 0x17a3b, 0x0181d, 0x183db, 0x016d0, 0x18d7b, 0x01595, 0x1971b, 0x0146c, 0x1a0bb, 0x01354, 0x1aa5a,
                                  0x0124c, 0x1b3fa, 0x01153, 0x1bd9a, 0x01067, 0x1c73a, 0x00f89, 0x1d0da, 0x00eb7, 0x1da79, 0x00df0, 0x1e419, 0x00d34, 0x1edb9, 0x00c82, 0x1f759,
                                  0x00bda, 0x200f9, 0x00b3c, 0x20a99, 0x00aa5, 0x21438, 0x00a17, 0x21dd8, 0x00990, 0x22778, 0x00911, 0x23118, 0x00898, 0x23ab8, 0x00826, 0x24458,
                                  0x007ba, 0x24df7, 0x00753, 0x25797, 0x006f2, 0x26137, 0x00696, 0x26ad7, 0x0063f, 0x27477, 0x005ed, 0x27e17, 0x0059f, 0x287b6, 0x00554, 0x29156,
                                  0x0050e, 0x29af6, 0x004cc, 0x2a497, 0x0048d, 0x2ae35, 0x00451, 0x2b7d6, 0x00418, 0x2c176, 0x003e2, 0x2cb15, 0x003af, 0x2d4b5, 0x0037f, 0x2de55};

const ufixed_t f_entropy_bits[128] = {
    1.0,
    1.0,
    0.92852783203125,
    1.0751953125,
    0.86383056640625,
    1.150390625,
    0.80499267578125,
    1.225555419921875,
    0.751251220703125,
    1.300750732421875,
    0.702056884765625,
    1.375946044921875,
    0.656829833984375,
    1.451141357421875,
    0.615203857421875,
    1.526336669921875,
    0.576751708984375,
    1.601531982421875,
    0.54119873046875,
    1.67669677734375,
    0.508209228515625,
    1.75189208984375,
    0.47760009765625,
    1.82708740234375,
    0.449127197265625,
    1.90228271484375,
    0.422637939453125,
    1.97747802734375,
    0.39788818359375,
    2.05267333984375,
    0.37481689453125,
    2.127838134765625,
    0.353240966796875,
    2.203033447265625,
    0.33306884765625,
    2.278228759765625,
    0.31414794921875,
    2.353424072265625,
    0.29644775390625,
    2.428619384765625,
    0.279815673828125,
    2.5037841796875,
    0.26422119140625,
    2.5789794921875,
    0.24957275390625,
    2.6541748046875,
    0.235809326171875,
    2.7293701171875,
    0.222869873046875,
    2.8045654296875,
    0.210662841796875,
    2.879730224609375,
    0.199188232421875,
    2.954925537109375,
    0.188385009765625,
    3.030120849609375,
    0.17822265625,
    3.105316162109375,
    0.168609619140625,
    3.180511474609375,
    0.1595458984375,
    3.255706787109375,
    0.1510009765625,
    3.33087158203125,
    0.1429443359375,
    3.40606689453125,
    0.135345458984375,
    3.48126220703125,
    0.128143310546875,
    3.55645751953125,
    0.121368408203125,
    3.63165283203125,
    0.114959716796875,
    3.706817626953125,
    0.10888671875,
    3.782012939453125,
    0.1031494140625,
    3.857208251953125,
    0.09771728515625,
    3.932403564453125,
    0.09259033203125,
    4.007598876953125,
    0.0877685546875,
    4.082794189453125,
    0.083160400390625,
    4.157958984375,
    0.078826904296875,
    4.233154296875,
    0.07470703125,
    4.308349609375,
    0.070831298828125,
    4.383544921875,
    0.067138671875,
    4.458740234375,
    0.06365966796875,
    4.533935546875,
    0.06036376953125,
    4.609100341796875,
    0.057220458984375,
    4.684295654296875,
    0.05426025390625,
    4.759490966796875,
    0.05145263671875,
    4.834686279296875,
    0.048797607421875,
    4.909881591796875,
    0.046295166015625,
    4.985076904296875,
    0.043914794921875,
    5.06024169921875,
    0.0416259765625,
    5.13543701171875,
    0.03948974609375,
    5.21063232421875,
    0.0374755859375,
    5.285858154296875,
    0.035552978515625,
    5.360992431640625,
    0.033721923828125,
    5.43621826171875,
    0.031982421875,
    5.51141357421875,
    0.03033447265625,
    5.586578369140625,
    0.028778076171875,
    5.661773681640625,
    0.027313232421875,
    5.736968994140625,
};

/**
 * \brief Return the 7 lowest-order bits of the pixel coordinate.
 *
 * The 7 lower-order bits correspond to the distance from the left or top edge
 * of the containing LCU.
 */
#define SUB_SCU(xy) ((xy) & (LCU_WIDTH - 1))

#define LCU_CU_WIDTH 16
#define LCU_T_CU_WIDTH (LCU_CU_WIDTH + 1)
#define LCU_CU_OFFSET (LCU_T_CU_WIDTH + 1)
#define SCU_WIDTH (LCU_WIDTH / LCU_CU_WIDTH)

// Width from top left of the LCU, so +1 for ref buffer size.
#define LCU_REF_PX_WIDTH (LCU_WIDTH + LCU_WIDTH / 2)

/**
 * \brief Return pointer to the CU containing a given pixel.
 *
 * \param x_px  x-coordinate relative to the upper left corner of the LCU
 * \param y_px  y-coordinate relative to the upper left corner of the LCU
 * \return      pointer to the CU at coordinates (x_px, y_px)
 */
#define LCU_GET_CU_AT_PX(cu_mem, x_px, y_px) (&(cu_mem)->cu[((x_px) >> 2) + ((y_px) >> 2) * 16])

#define CU_COST 3

#define BIT_DEPTH 8
#define PIXEL_MIN 0
#define PIXEL_MAX (1 << BIT_DEPTH)

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

#define LCU_LUMA_SIZE (LCU_WIDTH * LCU_WIDTH)
#define LCU_CHROMA_SIZE (LCU_WIDTH * LCU_WIDTH >> 2)

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
#define MAX_UINT 0xFFFFFFFF
#endif
#ifndef MAX_INT64
#define MAX_INT64 0x7FFFFFFFFFFFFFFFLL
#endif
#ifndef MAX_DOUBLE
#define MAX_DOUBLE 1.7e+308
#endif

#endif
