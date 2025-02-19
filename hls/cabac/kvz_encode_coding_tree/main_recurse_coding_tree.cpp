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

#include "cabac.h"

#define NO_PRINT

#define MAX_DEPTH 3
#define MIN_SIZE 3
#define LCU_WIDTH (1 << (MIN_SIZE + MAX_DEPTH))
#define TR_MIN_LOG2_SIZE 2

#define LCU_LUMA_SIZE (LCU_WIDTH * LCU_WIDTH)
#define LCU_CHROMA_SIZE (LCU_WIDTH * LCU_WIDTH >> 2)

#define LCU_WIDTH_C (LCU_WIDTH / 2)

#define SUB_SCU(xy) ((xy) & (LCU_WIDTH - 1))

#define LCU_CU_WIDTH 16
#define LCU_T_CU_WIDTH (LCU_CU_WIDTH + 1)
#define LCU_CU_OFFSET (LCU_T_CU_WIDTH + 1)
#define SCU_WIDTH (LCU_WIDTH / LCU_CU_WIDTH)

#define MAX_PU_DEPTH 4

#define SWAP(a, b, swaptype)                                                                                                                                                                           \
  {                                                                                                                                                                                                    \
    swaptype tempval;                                                                                                                                                                                  \
    tempval = a;                                                                                                                                                                                       \
    a = b;                                                                                                                                                                                             \
    b = tempval;                                                                                                                                                                                       \
  }

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define ABS(a) ((a) >= 0 ? (a) : (-a))

#define CEILDIV(x, y) (((x) + (y) - 1) / (y))

#define PU_GET_X(part_mode, cu_width, cu_x, i) ((cu_x) + kvz_part_mode_offsets[(part_mode)][(i)][0] * (cu_width) / 4)

#define PU_GET_Y(part_mode, cu_width, cu_y, i) ((cu_y) + kvz_part_mode_offsets[(part_mode)][(i)][1] * (cu_width) / 4)

#define GET_SPLITDATA(CU, curDepth) (cu_get_depth((CU)) > curDepth)

#define LCU_GET_CU_AT_PX(cu, x_px, y_px) (&(cu)[LCU_CU_OFFSET + ((x_px) >> 2) + ((y_px) >> 2) * LCU_T_CU_WIDTH])

#define LCU_GET_CU_AT_PX_16x16(cu, x_px, y_px) ((cu)[((x_px) >> 2) + ((y_px) >> 2) * LCU_CU_WIDTH])

#define LCU_GET_CU_TOP_AT_PX(cu, x_px) ((cu)[(x_px) >> 2])

#define LCU_GET_CU_LEFT_AT_PX(cu, y_px) ((cu)[(y_px) >> 2])

#define NUM_CBF_DEPTHS 5

#define C1FLAG_NUMBER 8 // maximum number of largerThan1 flag coded in one chunk
#define C2FLAG_NUMBER 1 // maximum number of largerThan2 flag coded in one chunk

typedef enum {
  CU_NOTSET = 0,
  CU_INTRA = 1,
  CU_INTER = 2,
  CU_PCM = 3,
} cu_type_t;

typedef struct {
  coeff_t y[LCU_LUMA_SIZE];
  coeff_t u[LCU_CHROMA_SIZE];
  coeff_t v[LCU_CHROMA_SIZE];
} lcu_coeff_t;

typedef struct {
  uint_8 type;      //      : 2; //!< \brief block type, one of cu_type_t values
  uint_8 depth;     //     : 3; //!< \brief depth / size of this block
  uint_8 part_size; // : 3; //!< \brief partition mode, one of part_mode_t values
  uint_8 tr_depth;  //  : 3; //!< \brief transform depth
  uint_8 skipped;   //   : 1; //!< \brief flag to indicate this block is skipped
  uint_8 merged;    //    : 1; //!< \brief flag to indicate this block is merged
  uint_8 merge_idx; // : 3; //!< \brief merge index
  uint_16 cbf;
  int_8 tr_skip;

  /**
   * \brief QP used for the CU.
   *
   * This is required for deblocking when per-LCU QPs are enabled.
   */
  uint_8 qp;

  struct {
    int_8 mode;
    int_8 mode_chroma;
  } intra;
} cu_info_t;

typedef struct cu_array_t {
  struct cu_array_t *base; //!< \brief base cu array or NULL
  cu_info_t *data;         //!< \brief cu array
  int_32 width;            //!< \brief width of the array in pixels
  int_32 height;           //!< \brief height of the array in pixels
  int_32 stride;           //!< \brief stride of the array in pixels
  int_32 refcount;         //!< \brief number of references to this cu_array
} cu_array_t;

typedef struct videoframe {
  int_32 width;         //!< \brief Luma pixel array width.
  int_32 height;        //!< \brief Luma pixel array height.
  int_32 height_in_lcu; //!< \brief Picture width in number of LCU's.
  int_32 width_in_lcu;  //!< \brief Picture height in number of LCU's.

  cu_array_t *cu_array; //!< \brief Info for each CU at each depth.
} videoframe_t;

#define SCAN_DIAG 0
#define SCAN_HOR 1
#define SCAN_VER 2

uint_8 kvz_part_mode_offsets[][4][2] = {
    {{0, 0}},                         // 2Nx2N
    {{0, 0}, {0, 2}},                 // 2NxN
    {{0, 0}, {2, 0}},                 // Nx2N
    {{0, 0}, {2, 0}, {0, 2}, {2, 2}}, // NxN
    {{0, 0}, {0, 1}},                 // 2NxnU
    {{0, 0}, {0, 3}},                 // 2NxnD
    {{0, 0}, {1, 0}},                 // nLx2N
    {{0, 0}, {3, 0}},                 // nRx2N
};

template < int width > ac_int< width, false > Abs(ac_int< width, true > a) {
  ac_int< width, false > tmp0 = a, tmp1 = 0;

#pragma hls_unroll yes
  for (int i = 0; i < width; i++)
    tmp1[i] = tmp0[i] ^ tmp0[width - 1];

  return tmp1 + tmp0[width - 1];
}

static uint_16 cbf_masks[NUM_CBF_DEPTHS] = {0x1f, 0x0f, 0x07, 0x03, 0x1};

static ac_int< 6, false > g_group_idx[32] = {0, 1, 2, 3, 4, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9};

static uint_8 g_min_in_group[10] = {0, 1, 2, 3, 4, 6, 8, 12, 16, 24};

ac_int< 3, true > kvz_g_convert_to_bit[LCU_WIDTH + 1] = {-1, -1, -1, -1, 0,  -1, -1, -1, 1,  -1, -1, -1, -1, -1, -1, -1, 2,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3,
                                                         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4};

ac_int< 11, false > coeff_scan_idx[4] = {0, 16, 80, 336};

static uint_8 cu_get_mode(uint_32 cu) { return cu & 0x3F; }

static uint_8 cu_get_depth(uint_32 cu) { return ((cu >> 6) & 0x03) + 1; }

static uint_8 cu_get_tsf(uint_32 cu) { return (cu >> 14) & 0x01; }

static uint_16 cu_get_cbf(uint_32 cu) { return cu >> 16; }

#define ACC_LCU_GET_CU_AT_PX(cu, x_px, y_px) ((cu)[((x_px) >> 2) + ((y_px) >> 2) * LCU_CU_WIDTH])

const ac_int< 13, false > coeff_offset[3] = {0, LCU_LUMA_SIZE, LCU_LUMA_SIZE + LCU_CHROMA_SIZE};

template < int depth >
void recursive_coding_tree(ac_channel< ac_int< ENCODE_CONFIG_WIDTH + 1, false > > &encode_intra_config_and_cabac_write, ac_int< 1, false > &config_qp_values, cu_info lcu[16 * 16],
                           cu_info_simple lcu_top[16], cu_info_simple lcu_left[16], cu_info corner, uint_16 width, uint_16 height, ac_int< 16, false > x_real, ac_int< 16, false > y_real,
                           ac_int< 16, false > x_ctu, ac_int< 16, false > y_ctu, uint_16 offset_x, uint_16 offset_y, bool must_code_qp_delta, uint_8 qp, uint_8 ref_qp) {
  cu_info cur_cu = LCU_GET_CU_AT_PX_16x16(lcu, x_ctu, y_ctu);

  cu_info left_cu;
  ac_int< 1, false > left_valid = 0;
  if (x_real > 0) {
    ac_int< 17, true > x_ctu_minus = x_ctu - 1;

    if (x_ctu_minus == -1) {
      if (y_ctu == -1) {
        left_cu = corner;
      } else {
        left_cu = LCU_GET_CU_LEFT_AT_PX(&lcu_left[0], y_ctu);
      }
    } else {
      left_cu = LCU_GET_CU_AT_PX_16x16(lcu, x_ctu_minus, y_ctu);
    }
    left_valid = 1;
  }

  cu_info above_cu;
  ac_int< 1, false > above_valid = 0;
  if (y_real > 0) {
    ac_int< 17, true > y_ctu_minus = y_ctu - 1;

    if (y_ctu_minus == -1) {
      if (x_ctu == -1) {
        above_cu = corner;
      } else {
        above_cu = LCU_GET_CU_TOP_AT_PX(&lcu_top[0], x_ctu);
      }
    } else {
      above_cu = LCU_GET_CU_AT_PX_16x16(lcu, x_ctu, y_ctu_minus);
    }
    above_valid = 1;
  }

  uint_8 split_flag = GET_SPLITDATA(cur_cu, depth);

  uint_8 split_model = 0;

  // Absolute coordinates
  uint_16 abs_x = x_real + offset_x;
  uint_16 abs_y = y_real + offset_y;

  // Check for slice border FIXME
  bool border_x = width < abs_x + (LCU_WIDTH >> depth);
  bool border_y = height < abs_y + (LCU_WIDTH >> depth);
  bool border_split_x = width >= abs_x + (LCU_WIDTH >> MAX_DEPTH) + (LCU_WIDTH >> (depth + 1));
  bool border_split_y = height >= abs_y + (LCU_WIDTH >> MAX_DEPTH) + (LCU_WIDTH >> (depth + 1));
  bool border = border_x || border_y; /*!< are we in any border CU */

  // When not in MAX_DEPTH, insert split flag and split the blocks if needed
  // if (depth != MAX_DEPTH)
  {
    // Implisit split flag when on border
    if (!border) {
      // Get left and top block split_flags and if they are present and true, increase model number
      if (left_valid && GET_SPLITDATA(left_cu, depth) == 1) {
        split_model++;
      }

      if (above_valid && GET_SPLITDATA(above_cu, depth) == 1) {
        split_model++;
      }

      {
#ifndef NO_PRINT
        printf("cur_ctx %d, index %d : ", 2, split_model);
        printf("%d %s\n", split_flag, "SplitFlag");
#endif

        cabac_val temp_cmd = cabac_param_to_val(split_flag, 0, 2, split_model, CMD_ENCODE_BIN);
        ac_int< ENCODE_CONFIG_WIDTH + 1, false > temp_out;
        temp_out.set_slc(1, temp_cmd);
        temp_out[0] = 1;

        encode_intra_config_and_cabac_write.write(temp_out);
      }
    }

    if (split_flag || border) {
      // Split blocks and remember to change x and y block positions
      int offset = LCU_WIDTH >> (depth + 1);

      recursive_coding_tree< depth + 1 >(
          encode_intra_config_and_cabac_write, config_qp_values, lcu, lcu_top, lcu_left, corner, width, height, x_real, y_real, x_ctu, y_ctu, offset_x, offset_y, must_code_qp_delta, qp, ref_qp);

      if (!border_x || border_split_x) {
        recursive_coding_tree< depth + 1 >(encode_intra_config_and_cabac_write,
                                           config_qp_values,
                                           lcu,
                                           lcu_top,
                                           lcu_left,
                                           corner,
                                           width,
                                           height,
                                           x_real + offset,
                                           y_real,
                                           x_ctu + offset,
                                           y_ctu,
                                           offset_x,
                                           offset_y,
                                           must_code_qp_delta,
                                           qp,
                                           ref_qp);
      }
      if (!border_y || border_split_y) {
        recursive_coding_tree< depth + 1 >(encode_intra_config_and_cabac_write,
                                           config_qp_values,
                                           lcu,
                                           lcu_top,
                                           lcu_left,
                                           corner,
                                           width,
                                           height,
                                           x_real,
                                           y_real + offset,
                                           x_ctu,
                                           y_ctu + offset,
                                           offset_x,
                                           offset_y,
                                           must_code_qp_delta,
                                           qp,
                                           ref_qp);
      }
      if (!border || (border_split_x && border_split_y)) {
        recursive_coding_tree< depth + 1 >(encode_intra_config_and_cabac_write,
                                           config_qp_values,
                                           lcu,
                                           lcu_top,
                                           lcu_left,
                                           corner,
                                           width,
                                           height,
                                           x_real + offset,
                                           y_real + offset,
                                           x_ctu + offset,
                                           y_ctu + offset,
                                           offset_x,
                                           offset_y,
                                           must_code_qp_delta,
                                           qp,
                                           ref_qp);
      }
      return;
    }
  }

  encode_config_val config_temp =
      encode_config_param_to_val(x_real, y_real, x_ctu, y_ctu, depth, must_code_qp_delta, qp, ref_qp, cu_get_mode(cur_cu), cu_get_depth(cur_cu), corner, 0, config_qp_values);

  ac_int< ENCODE_CONFIG_WIDTH + 1, false > temp_out;
  temp_out.set_slc(1, config_temp);
  temp_out[0] = 0;

  encode_intra_config_and_cabac_write.write(temp_out);
}

template <>
void recursive_coding_tree< MAX_DEPTH >(ac_channel< ac_int< ENCODE_CONFIG_WIDTH + 1, false > > &encode_intra_config_and_cabac_write, ac_int< 1, false > &config_qp_values, cu_info lcu[16 * 16],
                                        cu_info_simple lcu_top[16], cu_info_simple lcu_left[16], cu_info corner, uint_16 width, uint_16 height, ac_int< 16, false > x_real, ac_int< 16, false > y_real,
                                        ac_int< 16, false > x_ctu, ac_int< 16, false > y_ctu, uint_16 offset_x, uint_16 offset_y, bool must_code_qp_delta, uint_8 qp, uint_8 ref_qp) {
  cu_info cur_cu = LCU_GET_CU_AT_PX_16x16(lcu, x_ctu, y_ctu);

  {
#ifndef NO_PRINT
    printf("cur_ctx %d, index %d : ", 10, 0);
    printf("%d %s\n", 1, "part_mode 2Nx2N");
#endif

    cabac_val temp_cmd = cabac_param_to_val(1, 0, 10, 0, CMD_ENCODE_BIN);
    ac_int< ENCODE_CONFIG_WIDTH + 1, false > temp_out;
    temp_out.set_slc(1, temp_cmd);
    temp_out[0] = 1;

    encode_intra_config_and_cabac_write.write(temp_out);
  }

  encode_config_val config_temp =
      encode_config_param_to_val(x_real, y_real, x_ctu, y_ctu, MAX_DEPTH, must_code_qp_delta, qp, ref_qp, cu_get_mode(cur_cu), cu_get_depth(cur_cu), corner, 0, config_qp_values);

  ac_int< ENCODE_CONFIG_WIDTH + 1, false > temp_out;
  temp_out.set_slc(1, config_temp);
  temp_out[0] = 0;

  encode_intra_config_and_cabac_write.write(temp_out);
}

void recursive_kvz_encode_coding_tree_hier(ac_channel< ac_int< ENCODE_CONFIG_WIDTH + 1, false > > &encode_intra_config_and_cabac_write, ac_channel< ac_int< 32, false > > &config_in,
                                           cu_info lcu[16 * 16], cu_info_simple lcu_top[16], cu_info_simple lcu_left[16]) {
  cu_info corner;
  uint_8 depth;
  bool must_code_qp_delta;
  uint_8 qp;
  uint_8 ref_qp;
  uint_8 intra_pred_mode_chroma;

  uint_16 width;
  uint_16 height;
  uint_16 offset_x;
  uint_16 offset_y;

  ac_int< 32, false > temp = config_in.read();

  width = temp.slc< 16 >(0);
  height = temp.slc< 16 >(16);

  temp = config_in.read();

  offset_x = temp.slc< 16 >(0);
  offset_y = temp.slc< 16 >(16);

  temp = config_in.read();

  ac_int< 16, false > x_real = temp.slc< 16 >(0);
  ac_int< 16, false > y_real = temp.slc< 16 >(16);

  temp = config_in.read();

  ac_int< 16, false > x_ctu = temp.slc< 16 >(0);
  ac_int< 16, false > y_ctu = temp.slc< 16 >(16);

  temp = config_in.read();

  depth = temp.slc< 8 >(0);

  {
    must_code_qp_delta = temp.slc< 1 >(8);
    qp = temp.slc< 8 >(8 + 1);
    ref_qp = temp.slc< 8 >(8 + 1 + 8);
  }

  ac_int< 1, false > config_qp_values = 1;

  temp = config_in.read();

  corner = temp.slc< 8 >(8);

  coding_tree_command commands[32];
  ac_int< 6, false > command_num = 0;

  if (temp[31] == 1) {
    ac_int< ENCODE_CONFIG_WIDTH + 1, false > temp_out;
    cabac_val temp_cmd = cabac_param_to_val(0, 0, 0, 0, CMD_START);

    temp_out.set_slc(1, temp_cmd);
    temp_out[0] = 1;
    encode_intra_config_and_cabac_write.write(temp_out);

    recursive_coding_tree< 0 >(
        encode_intra_config_and_cabac_write, config_qp_values, lcu, lcu_top, lcu_left, corner, width, height, x_real, y_real, x_ctu, y_ctu, offset_x, offset_y, must_code_qp_delta, qp, ref_qp);

    temp_cmd = cabac_param_to_val(0, 0, 0, 0, CMD_STOP);
    temp_out.set_slc(1, temp_cmd);
    temp_out[0] = 1;

    encode_intra_config_and_cabac_write.write(temp_out);
  }
}

ac_int< 6, false > border_idx_depth[3][4] = {{1, 6, 11, 16}, {1, 2, 3, 4}, {0, 0, 0, 0}};

template < int depth >
void recurse_borders(ac_int< 5, false > border_flags[21], uint_16 width, uint_16 height, ac_int< 16, false > x_real, ac_int< 16, false > y_real, uint_16 offset_x, uint_16 offset_y,
                     ac_int< 10, false > border_idx) {
  // Absolute coordinates
  uint_16 abs_x = x_real + offset_x;
  uint_16 abs_y = y_real + offset_y;

  ac_int< 5, false > temp;

  temp[0] = width < abs_x + (LCU_WIDTH >> depth);
  temp[1] = height < abs_y + (LCU_WIDTH >> depth);
  temp[2] = width >= abs_x + (LCU_WIDTH >> MAX_DEPTH) + (LCU_WIDTH >> (depth + 1));
  temp[3] = height >= abs_y + (LCU_WIDTH >> MAX_DEPTH) + (LCU_WIDTH >> (depth + 1));
  temp[4] = temp[0] || temp[1]; /*!< are we in any border CU */

  border_flags[border_idx] = temp;

  ac_int< 6, false > offset = LCU_WIDTH >> (depth + 1);

  recurse_borders< depth + 1 >(border_flags, width, height, x_real, y_real, offset_x, offset_y, border_idx + border_idx_depth[depth][0]);
  recurse_borders< depth + 1 >(border_flags, width, height, x_real + offset, y_real, offset_x, offset_y, border_idx + border_idx_depth[depth][1]);
  recurse_borders< depth + 1 >(border_flags, width, height, x_real, y_real + offset, offset_x, offset_y, border_idx + border_idx_depth[depth][2]);
  recurse_borders< depth + 1 >(border_flags, width, height, x_real + offset, y_real + offset, offset_x, offset_y, border_idx + border_idx_depth[depth][3]);
}

template <>
void recurse_borders< MAX_DEPTH >(ac_int< 5, false > border_flags[21], uint_16 width, uint_16 height, ac_int< 16, false > x_real, ac_int< 16, false > y_real, uint_16 offset_x, uint_16 offset_y,
                                  ac_int< 10, false > border_idx) {
  return;
}

void kvz_encode_coding_tree_hier(ac_channel< ac_int< ENCODE_CONFIG_WIDTH + 1, false > > &encode_intra_config_and_cabac_write, ac_channel< ac_int< 32, false > > &config_in,
                                 cu_info lcu[16 * 16 * LCU_COUNT], cu_info_simple lcu_top[16 * LCU_COUNT], cu_info_simple lcu_left[16 * LCU_COUNT], uint_32 *low, uint_16 *range, uint_8 *buffered_byte,
                                 uint_8 *num_buffered_bytes, uint_8 *bits_left, uint_8 *stream_data, uint_8 *stream_cur_bit, uint_8 *stream_zercount, ac_int< 128, false > cabac_metas[LCU_COUNT]) {
  cu_info corner;
  uint_8 depth;
  bool must_code_qp_delta;
  uint_8 qp;
  uint_8 ref_qp;
  uint_8 intra_pred_mode_chroma;

  uint_16 width;
  uint_16 height;
  uint_16 offset_x;
  uint_16 offset_y;

  ac_int< 5, false > ctu_id = 0;

  ac_int< 32, false > temp = config_in.read();

  width = temp.slc< 16 >(0);
  height = temp.slc< 16 >(16);

  temp = config_in.read();

  offset_x = temp.slc< 16 >(0);
  offset_y = temp.slc< 16 >(16);

  temp = config_in.read();

  ac_int< 16, false > x_real = temp.slc< 16 >(0);
  ac_int< 16, false > y_real = temp.slc< 16 >(16);

  temp = config_in.read();

  ac_int< 16, false > x_ctu = temp.slc< 16 >(0);
  ac_int< 16, false > y_ctu = temp.slc< 16 >(16);

  temp = config_in.read();

  depth = 0;

  {
    must_code_qp_delta = temp.slc< 1 >(8);
    qp = temp.slc< 8 >(8 + 1);
    ref_qp = temp.slc< 8 >(8 + 1 + 8);
    ctu_id = temp.slc< 5 >(8 + 1 + 8 + 8);
  }

  ac_int< 1, false > config_qp_values = 1;

  ac_int< 128, false > cabac_metadata = cabac_metas[ctu_id];

  temp = config_in.read();

  corner = temp.slc< 8 >(8);

  coding_tree_command commands[32];
  ac_int< 6, false > command_num = 0;

  *low = cabac_metadata.slc< 32 >(0);
  *range = cabac_metadata.slc< 16 >(32);
  *buffered_byte = cabac_metadata.slc< 8 >(32 + 16);
  *num_buffered_bytes = cabac_metadata.slc< 8 >(32 + 16 + 8);
  *bits_left = cabac_metadata.slc< 8 >(32 + 16 + 8 + 8);
  *stream_data = cabac_metadata.slc< 8 >(32 + 16 + 8 + 8 + 8);
  *stream_cur_bit = cabac_metadata.slc< 8 >(32 + 16 + 8 + 8 + 8 + 8);
  *stream_zercount = cabac_metadata.slc< 8 >(32 + 16 + 8 + 8 + 8 + 8 + 8);

  if (temp[31] == 1) {
    {
      ac_int< CABAC_VAL_WIDTH + ENCODE_CONFIG_WIDTH + 2, false > temp_out = 0;
      cabac_val temp_cmd = cabac_param_to_val(0, 0, 0, ctu_id, CMD_START);
      temp_out.set_slc(1, temp_cmd);
      temp_out[0] = 1;

      encode_intra_config_and_cabac_write.write(temp_out);
    }

    ac_int< 5, false > border_flags[21];

    recurse_borders< 0 >(border_flags, width, height, x_real, y_real, offset_x, offset_y, 0);

    {
      ac_int< 10, false > border_idx = 0;

      ac_int< 16, false > x_real_last[4] = {0, 0, 0, 0};
      ac_int< 16, false > y_real_last[4] = {0, 0, 0, 0};
      ac_int< 16, false > x_ctu_last[4] = {0, 0, 0, 0};
      ac_int< 16, false > y_ctu_last[4] = {0, 0, 0, 0};
      ac_int< 10, false > border_idx_last[4] = {0, 0, 0, 0};
      cu_info cur_cu_last[4] = {0, 0, 0, 0};

      ac_int< 3, false > recurse_pos[4] = {0, 0, 0, 0};
      ac_int< 1, false > recurse_return = 0;

      ac_int< ENCODE_CONFIG_WIDTH + 1, false > temp_out1 = 0;
      ac_int< ENCODE_CONFIG_WIDTH + 1, false > temp_out2 = 0;
      ac_int< 1, false > breakloop = 0;
      // max twice for depths 0->2 because of recursion return and once for depth 3
      // 4*2(32x32) + 16*2(16x16) + 64*2(8x8) + 256(4x4)
      for (int a = 0; a < 106 * 4; a++) {
        cu_info cur_cu;
        ac_int< 6, false > offset = LCU_WIDTH >> (depth + 1);

        bool border_x;
        bool border_y;
        bool border_split_x;
        bool border_split_y;
        bool border;

        if (recurse_return) {
          x_real = x_real_last[depth];
          y_real = y_real_last[depth];

          x_ctu = x_ctu_last[depth];
          y_ctu = y_ctu_last[depth];
          border_idx = border_idx_last[depth];
          cur_cu = cur_cu_last[depth];

          border_x = border_flags[border_idx][0];
          border_y = border_flags[border_idx][1];
          border_split_x = border_flags[border_idx][2];
          border_split_y = border_flags[border_idx][3];
          border = border_flags[border_idx][4];
        }

        if (recurse_return && recurse_pos[depth] == 0) {
          recurse_pos[depth] = 1;
          if (!border_x || border_split_x) {
            x_real += offset;
            x_ctu += offset;
            border_idx = border_idx + border_idx_depth[depth][1];
            depth++;
            recurse_return = 0;
          } else {
            recurse_return = 1;
          }
        } else if (recurse_return && recurse_pos[depth] == 1) {
          recurse_pos[depth] = 2;
          if (!border_y || border_split_y) {
            y_real += offset;
            y_ctu += offset;
            border_idx = border_idx + border_idx_depth[depth][2];
            depth++;
            recurse_return = 0;
          } else {
            recurse_return = 1;
          }
        } else if (recurse_return && recurse_pos[depth] == 2) {
          recurse_pos[depth] = 3;
          if (!border || (border_split_x && border_split_y)) {
            x_real += offset;
            y_real += offset;
            x_ctu += offset;
            y_ctu += offset;
            border_idx = border_idx + border_idx_depth[depth][3];
            depth++;
            recurse_return = 0;
          } else {
            recurse_return = 1;
          }
        } else if (recurse_return && recurse_pos[depth] == 3) {
          if (depth == 0) {
            breakloop = 1;
          }
          recurse_pos[depth] = 0;
          recurse_return = 1;
          depth--;
        } else if (depth == 3) {
          cur_cu = LCU_GET_CU_AT_PX_16x16(&lcu[16 * 16 * ctu_id], x_ctu, y_ctu);

          {
            ac_int< 1, false > temp_value = 0;
            if (cu_get_depth(cur_cu) != 4) {
              temp_value = 1;
#ifndef NO_PRINT
              printf("cur_ctx %d, index %d : ", 10, 0);
              printf("%d %s\n", 1, "part_mode 2Nx2N");
#endif
            } else {
              temp_value = 0;
#ifndef NO_PRINT
              printf("cur_ctx %d, index %d : ", 10, 0);
              printf("%d %s\n", 0, "part_mode NxN");
#endif
            }
            cabac_val temp_cmd = cabac_param_to_val(temp_value, 0, 10, 0, CMD_ENCODE_BIN);
            temp_out1.set_slc(1, temp_cmd);
            temp_out1[0] = 1;
          }

          encode_config_val config_temp =
              encode_config_param_to_val(x_real, y_real, x_ctu, y_ctu, MAX_DEPTH, must_code_qp_delta, qp, ref_qp, cu_get_mode(cur_cu), cu_get_depth(cur_cu), corner, ctu_id, config_qp_values);

          temp_out2.set_slc(1, config_temp);
          temp_out2[0] = 0;

          recurse_return = 1;
          depth--;
        } else {
          cur_cu = LCU_GET_CU_AT_PX_16x16(&lcu[16 * 16 * ctu_id], x_ctu, y_ctu);
          cu_info left_cu;
          ac_int< 1, false > left_valid = 0;

          if (x_real != 0) {
            ac_int< 17, true > x_ctu_minus = x_ctu - 1;

            if (x_ctu_minus == -1) {
              if (y_ctu == -1) {
                left_cu = corner;
              } else {
                left_cu = LCU_GET_CU_LEFT_AT_PX(&lcu_left[16 * ctu_id], y_ctu);
              }
            } else {
              left_cu = LCU_GET_CU_AT_PX_16x16(&lcu[16 * 16 * ctu_id], x_ctu_minus, y_ctu);
            }
            left_valid = 1;
          }

          cu_info above_cu;
          ac_int< 1, false > above_valid = 0;
          if (y_real != 0) {
            ac_int< 17, true > y_ctu_minus = y_ctu - 1;

            if (y_ctu_minus == -1) {
              if (x_ctu == -1) {
                above_cu = corner;
              } else {
                above_cu = LCU_GET_CU_TOP_AT_PX(&lcu_top[16 * ctu_id], x_ctu);
              }
            } else {
              above_cu = LCU_GET_CU_AT_PX_16x16(&lcu[16 * 16 * ctu_id], x_ctu, y_ctu_minus);
            }
            above_valid = 1;
          }

          ac_int< 1, false > split_flag = GET_SPLITDATA(cur_cu, depth);
          uint_8 split_model = 0;

          border_x = border_flags[border_idx][0];
          border_y = border_flags[border_idx][1];
          border_split_x = border_flags[border_idx][2];
          border_split_y = border_flags[border_idx][3];
          border = border_flags[border_idx][4];

          // When not in MAX_DEPTH, insert split flag and split the blocks if needed
          // if (depth != MAX_DEPTH)
          // Implisit split flag when on border
          if (!border) {
            // Get left and top block split_flags and if they are present and true, increase model number
            if (left_valid && GET_SPLITDATA(left_cu, depth) == 1) {
              split_model++;
            }

            if (above_valid && GET_SPLITDATA(above_cu, depth) == 1) {
              split_model++;
            }

            {
#ifndef NO_PRINT
              printf("cur_ctx %d, index %d : ", 2, split_model);
              printf("%d %s\n", split_flag, "SplitFlag");
#endif

              cabac_val temp_cmd = cabac_param_to_val(split_flag, 0, 2, split_model, CMD_ENCODE_BIN);
              temp_out1.set_slc(1, temp_cmd);
              temp_out1[0] = 1;
            }
          }

          if (split_flag || border) {
            x_real_last[depth] = x_real;
            y_real_last[depth] = y_real;
            x_ctu_last[depth] = x_ctu;
            y_ctu_last[depth] = y_ctu;
            cur_cu_last[depth] = cur_cu;
            border_idx_last[depth] = border_idx;
            border_idx = border_idx + border_idx_depth[depth][0];

            depth++;
          } else {
            encode_config_val config_temp =
                encode_config_param_to_val(x_real, y_real, x_ctu, y_ctu, depth, must_code_qp_delta, qp, ref_qp, cu_get_mode(cur_cu), cu_get_depth(cur_cu), corner, ctu_id, config_qp_values);

            ac_int< ENCODE_CONFIG_WIDTH + 1, false > temp_out = 0;

            temp_out2.set_slc(1, config_temp);
            temp_out2[0] = 0;

            if (depth == 0) {
              breakloop = 1;
            }
            recurse_pos[depth] = 0;
            recurse_return = 1;
            depth--;
          }
        }
        if (temp_out1 != 0) {
          encode_intra_config_and_cabac_write.write(temp_out1);
          temp_out1 = 0;
        }
        if (temp_out2 != 0) {
          encode_intra_config_and_cabac_write.write(temp_out2);
          temp_out2 = 0;
        }
        if (breakloop) {
          break;
        }
      }
    }

    {
      ac_int< ENCODE_CONFIG_WIDTH + 1, false > temp_out = 0;
      cabac_val temp_cmd = cabac_param_to_val(0, 0, 0, 0, CMD_STOP);
      temp_out.set_slc(1, temp_cmd);
      temp_out[0] = 1;

      encode_intra_config_and_cabac_write.write(temp_out);
    }
  }
}

void mux_channel(ac_channel< ac_int< ENCODE_CONFIG_WIDTH + 1, false > > &encode_intra_config_and_cabac_write, cabac_chan &cabac_write, encode_config_chan &encode_intra_config) {
#ifndef __SYNTHESIS__
  while (encode_intra_config_and_cabac_write.available(1)) {
#endif
    ac_int< ENCODE_CONFIG_WIDTH + 1, false > temp = encode_intra_config_and_cabac_write.read();
    if (temp[0]) {
      cabac_write.write(temp.slc< CABAC_VAL_WIDTH >(1));
    } else {
      encode_intra_config.write(temp.slc< ENCODE_CONFIG_WIDTH >(1));
    }
#ifndef __SYNTHESIS__
  }
#endif
}

void kvz_encode_coding_tree(cabac_chan &cabac_write, ac_channel< ac_int< 32, false > > &config_in, encode_config_chan &encode_intra_config, cu_info lcu[16 * 16 * LCU_COUNT],
                            cu_info_simple lcu_top[16 * LCU_COUNT], cu_info_simple lcu_left[16 * LCU_COUNT], uint_32 *low, uint_16 *range, uint_8 *buffered_byte, uint_8 *num_buffered_bytes,
                            uint_8 *bits_left, uint_8 *stream_data, uint_8 *stream_cur_bit, uint_8 *stream_zercount, ac_int< 128, false > cabac_metas[LCU_COUNT]) {
  static ac_channel< ac_int< ENCODE_CONFIG_WIDTH + 1, false > > encode_intra_config_and_cabac_write;

  kvz_encode_coding_tree_hier(
      encode_intra_config_and_cabac_write, config_in, lcu, lcu_top, lcu_left, low, range, buffered_byte, num_buffered_bytes, bits_left, stream_data, stream_cur_bit, stream_zercount, cabac_metas);
  mux_channel(encode_intra_config_and_cabac_write, cabac_write, encode_intra_config);
}

void recursive_kvz_encode_coding_tree(cabac_chan &cabac_write, ac_channel< ac_int< 32, false > > &config_in, encode_config_chan &encode_intra_config, cu_info lcu[16 * 16], cu_info_simple lcu_top[16],
                                      cu_info_simple lcu_left[16]) {
  static ac_channel< ac_int< ENCODE_CONFIG_WIDTH + 1, false > > encode_intra_config_and_cabac_write;

  recursive_kvz_encode_coding_tree_hier(encode_intra_config_and_cabac_write, config_in, lcu, lcu_top, lcu_left);
  mux_channel(encode_intra_config_and_cabac_write, cabac_write, encode_intra_config);
}
