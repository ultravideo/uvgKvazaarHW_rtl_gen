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

#define LCU_WIDTH 64
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

uint_8 kvz_part_mode_num_parts[] = {
    1, // 2Nx2N
    2, // 2NxN
    2, // Nx2N
    4, // NxN
    2, // 2NxnU
    2, // 2NxnD
    2, // nLx2N
    2, // nRx2N
};

#define PU_GET_X(part_mode, cu_width, cu_x, i) ((cu_x) + kvz_part_mode_offsets[(part_mode)][(i)][0] * (cu_width) / 4)

#define PU_GET_Y(part_mode, cu_width, cu_y, i) ((cu_y) + kvz_part_mode_offsets[(part_mode)][(i)][1] * (cu_width) / 4)

// #define GET_SPLITDATA(CU,curDepth) ((CU)->depth > curDepth)
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

template < int width > ac_int< width, false > Abs(ac_int< width, true > a) {
  ac_int< width, false > tmp0 = a, tmp1 = 0;

#pragma hls_unroll yes
  for (int i = 0; i < width; i++)
    tmp1[i] = tmp0[i] ^ tmp0[width - 1];

  return tmp1 + tmp0[width - 1];
}

static uint_16 cbf_masks[NUM_CBF_DEPTHS] = {0x1f, 0x0f, 0x07, 0x03, 0x1};

static uint_8 cu_get_mode(uint_32 cu) { return cu & 0x3F; }

static uint_8 cu_get_depth(uint_32 cu) { return ((cu >> 6) & 0x03) + 1; }

static uint_8 cu_get_tsf(uint_32 cu) { return (cu >> 14) & 0x01; }

static uint_16 cu_get_cbf(uint_32 cu) { return cu >> 16; }

static bool cbf_is_set(uint_16 cbf, uint_8 depth, ac_int< 2, false > plane) { return (cbf & (cbf_masks[depth] << (NUM_CBF_DEPTHS * plane))) != 0; }

int_8 kvz_intra_get_dir_luma_predictor(ac_int< 16, false > x, ac_int< 16, false > y, int_8 *preds, cu_info_simple left_pu, cu_info_simple above_pu, ac_int< 1, false > left_pu_valid,
                                       ac_int< 1, false > above_pu_valid) {
  // The default mode if block is not coded yet is INTRA_DC.
  uint_8 left_intra_dir = 1;
  if (left_pu_valid /* && left_pu->type == CU_INTRA*/) {
    left_intra_dir = cu_get_mode(left_pu);
  }

  int_8 above_intra_dir = 1;
  if (above_pu_valid /*&& above_pu->type == CU_INTRA*/ && y.slc< MAX_DEPTH + MIN_SIZE >(0) != 0) {
    above_intra_dir = cu_get_mode(above_pu);
  }

  // If the predictions are the same, add new predictions
  if (left_intra_dir == above_intra_dir) {
    if (left_intra_dir > 1) { // angular modes
      uint_8 left_intra_dir1 = left_intra_dir + 29;
      uint_8 left_intra_dir2 = left_intra_dir - 1;
      preds[0] = left_intra_dir;
      preds[1] = (int_8)(left_intra_dir1.slc< 5 >(0)) + 2;
      preds[2] = (int_8)(left_intra_dir2.slc< 5 >(0)) + 2;
    } else {         // non-angular
      preds[0] = 0;  // PLANAR_IDX;
      preds[1] = 1;  // DC_IDX;
      preds[2] = 26; // VER_IDX;
    }
  } else {
    // If we have two distinct predictions
    preds[0] = left_intra_dir;
    preds[1] = above_intra_dir;

    // add planar mode if it's not yet present
    if (left_intra_dir && above_intra_dir) {
      preds[2] = 0; // PLANAR_IDX;
    } else {
      // Add DC mode if it's not present, otherwise 26.
      preds[2] = (left_intra_dir + above_intra_dir) < 2 ? 26 : 1;
    }
  }

  return 1;
}

ac_int< 2, false > encode_transform_unit(ac_int< 5, false > ctu_id, ac_channel< ac_int< 32 + 16 + 16 + 8 + 2 + 5, false > > &cabac_main_config, cu_info lcu[16 * 16], ac_int< 16, false > x,
                                         ac_int< 16, false > y, uint_8 depth) {
  ac_int< 2, false > done_reads = 0;

  cu_info cur_pu = LCU_GET_CU_AT_PX_16x16(lcu, x, y);

  bool cbf_y = cbf_is_set(cu_get_cbf(cur_pu), depth, /*COLOR_Y*/ 0);

  if (cbf_y) {
    ac_int< 32 + 16 + 16 + 8 + 2 + 5, false > conf_temp;

    conf_temp = cur_pu;

    conf_temp.set_slc(32, x);
    conf_temp.set_slc(32 + 16, y);

    conf_temp.set_slc(32 + 16 + 16, depth);
    // COLOR_Y
    conf_temp.set_slc(32 + 16 + 16 + 8, ac_int< 2, false >(0));

    conf_temp.set_slc(32 + 16 + 16 + 8 + 2, ctu_id);

    cabac_main_config.write(conf_temp);

    done_reads++;
  }

  if (depth == MAX_DEPTH + 1) {
    // For size 4x4 luma transform the corresponding chroma transforms are
    // also of size 4x4 covering 8x8 luma pixels. The residual is coded in
    // the last transform unit.
    if (x % 8 == 0 || y % 8 == 0) {
      // Not the last luma transform block so there is nothing more to do.
      return done_reads;
    } else {
      // Time to to code the chroma transform blocks. Move to the top-left
      // corner of the block.
      x -= 4;
      y -= 4;

      cur_pu = LCU_GET_CU_AT_PX_16x16(lcu, x, y);
    }
  }

  bool chroma_cbf_set = cbf_is_set(cu_get_cbf(cur_pu), depth, /*COLOR_U*/ 1) || cbf_is_set(cu_get_cbf(cur_pu), depth, /*COLOR_V*/ 2);
  if (chroma_cbf_set) {
    int x_local = (x >> 1) % LCU_WIDTH_C;
    int y_local = (y >> 1) % LCU_WIDTH_C;

    if (cbf_is_set(cu_get_cbf(cur_pu), depth, /*COLOR_U*/ 1)) {
      ac_int< 32 + 16 + 16 + 8 + 2 + 5, false > conf_temp;

      conf_temp = cur_pu;

      conf_temp.set_slc(32, x);
      conf_temp.set_slc(32 + 16, y);

      conf_temp.set_slc(32 + 16 + 16, depth);
      // COLOR_U
      conf_temp.set_slc(32 + 16 + 16 + 8, ac_int< 2, false >(1));

      conf_temp.set_slc(32 + 16 + 16 + 8 + 2, ctu_id);

      cabac_main_config.write(conf_temp);

      done_reads++;
    }

    if (cbf_is_set(cu_get_cbf(cur_pu), depth, /*COLOR_V*/ 2)) {
      ac_int< 32 + 16 + 16 + 8 + 2 + 5, false > conf_temp;

      conf_temp = cur_pu;

      conf_temp.set_slc(32, x);
      conf_temp.set_slc(32 + 16, y);

      conf_temp.set_slc(32 + 16 + 16, depth);
      // COLOR_V
      conf_temp.set_slc(32 + 16 + 16 + 8, ac_int< 2, false >(2));

      conf_temp.set_slc(32 + 16 + 16 + 8 + 2, ctu_id);

      cabac_main_config.write(conf_temp);

      done_reads++;
    }
  }

  return done_reads;
}

void kvz_cabac_write_ep_ex_golomb(cabac_chan &cabac_write, ac_int< 7, false > ctx, ac_int< 6, false > ctx_index, ac_int< 9, false > symbol, ac_int< 4, false > count) {
  ac_int< 9, false > bins = 0;
  ac_int< 6, false > num_bins = 0;

  if (symbol >= (uint_32)(1 << count)) {
    for (ac_int< 6, false > i = 0; i < 9; i++) {
      bins = 2 * bins + 1;
      ++num_bins;
      symbol -= 1 << count;
      ++count;
      if (symbol < (uint_32)(1 << count))
        break;
    }
  }
  bins = 2 * bins;
  ++num_bins;

  bins = (bins << count) | symbol;
  num_bins += count;

  {
    cabac_val temp_cmd = cabac_param_to_val(bins, num_bins, ctx, ctx_index, CMD_ENCODE_BINS_EP);
    cabac_write.write(temp_cmd);
  }
}

void kvz_cabac_write_unary_max_symbol(cabac_chan &cabac_write, ac_int< 7, false > ctx, ac_int< 6, false > &ctx_index, ac_int< 8, false > symbol, uint_32 offset, uint_32 max_symbol) {
  bool code_last = max_symbol > symbol;

  if (max_symbol == 0)
    return;

  {
    cabac_val temp_cmd = cabac_param_to_val(symbol != 0, 0, ctx, ctx_index, CMD_ENCODE_BIN);
    cabac_write.write(temp_cmd);
  }

  if (symbol == 0)
    return;

  if (symbol != 1) {
    symbol--;

    for (ac_int< 3, false > aa = 0; aa < 5; aa++) {
      ctx_index = offset;
      {
        cabac_val temp_cmd = cabac_param_to_val(1, 0, ctx, ctx_index, CMD_ENCODE_BIN);
        cabac_write.write(temp_cmd);
      }
      if (symbol - 1 == 0) {
        break;
      }
      symbol--;
    }
  }

  if (code_last) {
    ctx_index = offset;
    {
      cabac_val temp_cmd = cabac_param_to_val(0, 0, ctx, ctx_index, CMD_ENCODE_BIN);
      cabac_write.write(temp_cmd);
    }
  }
}

void encode_intra_coding_unit_and_transform_coeff_hier(encode_config_chan &config_in, cabac_chan &cabac_write, ac_channel< ac_int< 32 + 16 + 16 + 8 + 2 + 5, false > > &cabac_main_config,
                                                       ac_channel< ac_int< 1, false > > &cabac_done, cu_info lcu[16 * 16 * LCU_COUNT], cu_info_simple lcu_top[16 * LCU_COUNT],
                                                       cu_info_simple lcu_left[16 * LCU_COUNT], ac_int< 1, false > *active) {
  uint_8 intra_pred_mode_chroma;

  ac_int< 1, false > dummy = 0;

  static bool must_code_qp_delta;
  static uint_8 qp;
  static uint_8 ref_qp;

  uint_8 parent_coeff_u = 0;
  uint_8 parent_coeff_v = 0;

  ac_int< 3, false > cur_cu_depth = 0;

  cu_info corner;

  *active = 0;

  encode_config_val temp = config_in.read();

  ac_int< 16, false > x_real = temp.slc< 16 >(0);
  ac_int< 16, false > y_real = temp.slc< 16 >(16);

  ac_int< 16, false > x = temp.slc< 16 >(16 + 16);
  ac_int< 16, false > y = temp.slc< 16 >(16 + 16 + 16);

  ac_int< 2, false > done_reads = 0;

  uint_8 depth = temp.slc< 8 >(16 + 16 + 16 + 16);

  // Reset these values at the start of a new CTU (31th bit is one)
  if (temp[16 + 16 + 16 + 16 + 8 + 1 + 8 + 8] == 1) {
    must_code_qp_delta = temp.slc< 1 >(16 + 16 + 16 + 16 + 8);
    qp = temp.slc< 8 >(16 + 16 + 16 + 16 + 8 + 1);
    ref_qp = temp.slc< 8 >(16 + 16 + 16 + 16 + 8 + 1 + 8);
  }

  cur_cu_depth = temp.slc< 3 >(16 + 16 + 16 + 16 + 8 + 1 + 8 + 8 + 1);
  intra_pred_mode_chroma = temp.slc< 8 >(16 + 16 + 16 + 16 + 8 + 1 + 8 + 8 + 1 + 3);
  corner = temp.slc< 8 >(16 + 16 + 16 + 16 + 8 + 1 + 8 + 8 + 1 + 3 + 8);
  ac_int< 5, false > ctu_id = temp.slc< 5 >(16 + 16 + 16 + 16 + 8 + 1 + 8 + 8 + 1 + 3 + 8 + 8);
  *active = temp.slc< 1 >(16 + 16 + 16 + 16 + 8 + 1 + 8 + 8 + 1 + 3 + 8 + 8 + 5);

  {

    /*################## encode_intra_coding_unit START ###################*/

    uint_8 intra_pred_mode_actual[4];
    uint_8 *intra_pred_mode = intra_pred_mode_actual;

    int_8 intra_preds[4][3] = {{-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}};
    int_8 mpm_preds[4] = {-1, -1, -1, -1};
    ac_int< 1, false > flag[4] = {0};

    // PREDINFO CODING
    // If intra prediction mode is found from the predictors,
    // it can be signaled with two EP's. Otherwise we can send
    // 5 EP bins with the full predmode
    ac_int< 3, false > num_pred_units = 1;
    ac_int< 3, false > part_size = SIZE_2Nx2N;
    if (cur_cu_depth == 4) {
      num_pred_units = 4;
      part_size = SIZE_NxN;
    }

    int cu_width = LCU_WIDTH >> depth;

#pragma hls_pipeline_init_interval 3
    for (ac_int< 3, false > j = 0; j < num_pred_units; ++j) {
      ac_int< 16, false > pu_x_real = PU_GET_X(part_size, cu_width, x_real, j);
      ac_int< 16, false > pu_y_real = PU_GET_Y(part_size, cu_width, y_real, j);

      ac_int< 16, false > pu_x_ctu = PU_GET_X(part_size, cu_width, x, j);
      ac_int< 16, false > pu_y_ctu = PU_GET_Y(part_size, cu_width, y, j);

      cu_info cur_pu = LCU_GET_CU_AT_PX_16x16(&lcu[16 * 16 * ctu_id], pu_x_ctu, pu_y_ctu);

      cu_info left_pu = 0;
      cu_info above_pu = 0;

      ac_int< 1, false > left_pu_valid = 0;
      ac_int< 1, false > above_pu_valid = 0;

      if (pu_x_real != 0) {
        ac_int< 17, true > pu_x_ctu_minus = pu_x_ctu - 1;

        if (pu_x_ctu_minus == -1) {
          if (pu_y_ctu == -1) {
            left_pu = corner;
          } else {
            left_pu = LCU_GET_CU_LEFT_AT_PX(&lcu_left[16 * ctu_id], pu_y_ctu);
          }
        } else {
          left_pu = LCU_GET_CU_AT_PX_16x16(&lcu[16 * 16 * ctu_id], pu_x_ctu_minus, pu_y_ctu);
        }
        left_pu_valid = 1;
      }
      // Don't take the above PU across the LCU boundary.
      if (pu_y_real.slc< MAX_DEPTH + MIN_SIZE >(0) != 0 && pu_y_real != 0) {
        ac_int< 17, true > pu_y_ctu_minus = pu_y_ctu - 1;

        if (pu_y_ctu_minus == -1) {
          if (pu_x_ctu == -1) {
            above_pu = corner;
          } else {
            above_pu = LCU_GET_CU_TOP_AT_PX(&lcu_top[16 * ctu_id], pu_x_ctu);
          }
        } else {
          above_pu = LCU_GET_CU_AT_PX_16x16(&lcu[16 * 16 * ctu_id], pu_x_ctu, pu_y_ctu_minus);
        }
        above_pu_valid = 1;
      }

      kvz_intra_get_dir_luma_predictor(pu_x_real, pu_y_real, intra_preds[j], left_pu, above_pu, left_pu_valid, above_pu_valid);

      intra_pred_mode_actual[j] = cu_get_mode(cur_pu);

#pragma hls_unroll yes
      for (ac_int< 3, false > i = 0; i < 3; i++) {
        if (intra_preds[j][i] == intra_pred_mode[j]) {
          mpm_preds[j] = i;
          break;
        }
      }
      flag[j] = (mpm_preds[j] == -1) ? 0 : 1;
    }

#pragma hls_pipeline_init_interval 1
    for (ac_int< 3, false > j = 0; j < num_pred_units; ++j) {
#ifndef NO_PRINT
      printf("cur_ctx %d, index %d : ", 3, 0);
      printf("%d %s\n", flag[j], "prev_intra_luma_pred_flag");
#endif
      cabac_val temp_cmd = cabac_param_to_val(flag[j], 0, 3, 0, CMD_ENCODE_BIN);
      cabac_write.write(temp_cmd);
    }

#pragma hls_pipeline_init_interval 2
    for (ac_int< 3, false > j = 0; j < num_pred_units; ++j) {
      if (flag[j]) {
        {
#ifndef NO_PRINT
          printf("cur_ctx %d, index %d : ", 3, 0);
          printf("%d %s\n", mpm_preds[j] == 0 ? 0 : 1, "mpm_idx");
#endif

          cabac_val temp_cmd = cabac_param_to_val(mpm_preds[j] == 0 ? 0 : 1, 0, 3, 0, CMD_ENCODE_BIN_EP);
          cabac_write.write(temp_cmd);
        }

        if (mpm_preds[j] != 0) {
          {
#ifndef NO_PRINT
            printf("cur_ctx %d, index %d : ", 3, 0);
            printf("%d %s\n", mpm_preds[j] == 1 ? 0 : 1, "mpm_idx");
#endif

            cabac_val temp_cmd = cabac_param_to_val(mpm_preds[j] == 1 ? 0 : 1, 0, 3, 0, CMD_ENCODE_BIN_EP);
            cabac_write.write(temp_cmd);
          }
        }
      } else {
        // Signal the actual prediction mode.
        uint_8 tmp_pred = intra_pred_mode[j];

        // Sort prediction list from lowest to highest.
        if (intra_preds[j][0] > intra_preds[j][1])
          SWAP(intra_preds[j][0], intra_preds[j][1], int_8);
        if (intra_preds[j][0] > intra_preds[j][2])
          SWAP(intra_preds[j][0], intra_preds[j][2], int_8);
        if (intra_preds[j][1] > intra_preds[j][2])
          SWAP(intra_preds[j][1], intra_preds[j][2], int_8);

// Reduce the index of the signaled prediction mode according to the
// prediction list, as it has been already signaled that it's not one
// of the prediction modes.
#pragma hls_unroll yes
        for (ac_int< 3, false > i = 0; i < 3; i++) {
          tmp_pred = (tmp_pred > intra_preds[j][2 - i] ? (uint_8)(tmp_pred - 1) : tmp_pred);
        }

        {
#ifndef NO_PRINT
          printf("cur_ctx %d, index %d : ", 3, 0);
          printf("%d %d %s\n", tmp_pred, 5, "rem_intra_luma_pred_mode");
#endif

          cabac_val temp_cmd = cabac_param_to_val(tmp_pred, 5, 3, 0, CMD_ENCODE_BINS_EP);
          cabac_write.write(temp_cmd);
        }
      }
    }

    // Code chroma prediction mode.
    {
      uint_8 pred_mode = 5;
      uint_8 chroma_pred_modes[4] = {0, 26, 10, 1};

      if (intra_pred_mode_chroma == intra_pred_mode_actual[0]) {
        pred_mode = 4;
      } else if (intra_pred_mode_chroma == 34) {
// Angular 34 mode is possible only if intra pred mode is one of the
// possible chroma pred modes, in which case it is signaled with that
// duplicate mode.
#pragma hls_unroll yes
        for (int i = 0; i < 4; ++i) {
          if (intra_pred_mode_actual[0] == chroma_pred_modes[i])
            pred_mode = i;
        }
      } else {
#pragma hls_unroll yes
        for (int i = 0; i < 4; ++i) {
          if (intra_pred_mode_chroma == chroma_pred_modes[i])
            pred_mode = i;
        }
      }

      if (pred_mode == 4) {
        {
#ifndef NO_PRINT
          printf("cur_ctx %d, index %d : ", 4, 0);
          printf("%d %s\n", 0, "intra_chroma_pred_mode");
#endif

          cabac_val temp_cmd = cabac_param_to_val(0, 0, 4, 0, CMD_ENCODE_BIN);
          cabac_write.write(temp_cmd);
        }
      } else {
        {
#ifndef NO_PRINT
          printf("cur_ctx %d, index %d : ", 4, 0);
          printf("%d %s\n", 1, "intra_chroma_pred_mode");
#endif

          cabac_val temp_cmd = cabac_param_to_val(1, 0, 4, 0, CMD_ENCODE_BIN);
          cabac_write.write(temp_cmd);
        }

        {

#ifndef NO_PRINT
          printf("cur_ctx %d, index %d : ", 4, 0);
          printf("%d %d %s\n", pred_mode, 2, "intra_chroma_pred_mode");
#endif

          cabac_val temp_cmd = cabac_param_to_val(pred_mode, 2, 4, 0, CMD_ENCODE_BINS_EP);
          cabac_write.write(temp_cmd);
        }
      }
    }

    /*################## encode_intra_coding_unit END ###################*/

    ac_int< 16, false > x_save = 0;
    ac_int< 16, false > y_save = 0;

    bool cb_flag_y_save = 0;
    bool cb_flag_u_save = 0;
    bool cb_flag_v_save = 0;

    bool recurse_return = 0;
    ac_int< 3, false > recurse_pos = 0;

    ac_int< 2, false > tr_depth = 0;

    for (ac_int< 4, false > split_loop = 0; split_loop < 9; split_loop++) {
      bool split = 0;
      bool cb_flag_y = 0;
      bool cb_flag_u = 0;
      bool cb_flag_v = 0;
      if (recurse_return) {
        depth--;
        tr_depth--;
        x = x_save;
        y = y_save;
        cb_flag_y = cb_flag_y_save;
        cb_flag_u = cb_flag_u_save;
        cb_flag_v = cb_flag_v_save;
        recurse_return = 0;
        split = 1;
      } else {
        cu_info cur_pu = LCU_GET_CU_AT_PX_16x16(&lcu[16 * 16 * ctu_id], x, y);
        // Round coordinates down to a multiple of 8 to get the location of the
        // containing CU.

        ac_int< 16, false > x_rnd = x;
        ac_int< 16, false > y_rnd = y;

        x_rnd[0] = 0;
        x_rnd[1] = 0;
        x_rnd[2] = 0;

        y_rnd[0] = 0;
        y_rnd[1] = 0;
        y_rnd[2] = 0;

        cu_info cur_cu = LCU_GET_CU_AT_PX_16x16(&lcu[16 * 16 * ctu_id], x_rnd, y_rnd);

        split = (cu_get_depth(cur_cu) > depth);

        cb_flag_y = cbf_is_set(cu_get_cbf(cur_pu), depth, 0 /*COLOR_Y*/);
        cb_flag_u = cbf_is_set(cu_get_cbf(cur_cu), depth, 1 /*COLOR_U*/);
        cb_flag_v = cbf_is_set(cu_get_cbf(cur_cu), depth, 2 /*COLOR_V*/);

        // Chroma cb flags are not signaled when one of the following:
        // - transform size is 4 (2x2 chroma transform doesn't exist)
        // - they have already been signaled to 0 previously
        // When they are not present they are inferred to be 0, except for size 4
        // when the flags from previous level are used.
        if (depth < MAX_PU_DEPTH /* && state->encoder_control->chroma_format != KVZ_CSP_400*/) {

          if (tr_depth == 0 || cb_flag_u_save) {
            {
#ifndef NO_PRINT
              printf("cur_ctx %d, index %d : ", 8, tr_depth);
              printf("%d %s\n", cb_flag_u, "cbf_cb");
#endif

              cabac_val temp_cmd = cabac_param_to_val(cb_flag_u, 0, 8, tr_depth, CMD_ENCODE_BIN);
              cabac_write.write(temp_cmd);
            }
          }
          if (tr_depth == 0 || cb_flag_v_save) {
            {
#ifndef NO_PRINT
              printf("cur_ctx %d, index %d : ", 8, tr_depth);
              printf("%d %s\n", cb_flag_v, "cbf_cr");
#endif

              cabac_val temp_cmd = cabac_param_to_val(cb_flag_v, 0, 8, tr_depth, CMD_ENCODE_BIN);
              cabac_write.write(temp_cmd);
            }
          }
        }
      }
      if (split && recurse_pos != 4) {
        ac_int< 8, false > offset = LCU_WIDTH >> (depth + 1);
        ac_int< 16, false > x2 = x + offset;
        ac_int< 16, false > y2 = y + offset;

        depth++;
        tr_depth++;
        x_save = x;
        y_save = y;
        cb_flag_y_save = cb_flag_y;
        cb_flag_u_save = cb_flag_u;
        cb_flag_v_save = cb_flag_v;
        if (recurse_pos == 0) {
          x = x;
          y = y;
        } else if (recurse_pos == 1) {
          x = x2;
          y = y;
        } else if (recurse_pos == 2) {
          x = x;
          y = y2;
        } else if (recurse_pos == 3) {
          x = x2;
          y = y2;
        }
        recurse_pos++;

      } else {

        // Luma coded block flag is signaled when one of the following:
        // - prediction mode is intra
        // - transform depth > 0
        // - we have chroma coefficients at this level
        // When it is not present, it is inferred to be 1.
        // if (1) //Always intra /*cur_cu->type == CU_INTRA*/ /*|| tr_depth > 0*/ /*||*/ cb_flag_u || cb_flag_v) {
        {
#ifndef NO_PRINT
          printf("cur_ctx %d, index %d : ", 7, tr_depth == 0);
          printf("%d %s\n", cb_flag_y, "cbf_luma");
#endif

          cabac_val temp_cmd = cabac_param_to_val(cb_flag_y, 0, 7, tr_depth == 0, CMD_ENCODE_BIN);
          cabac_write.write(temp_cmd);
        }

        //}

        if (cb_flag_y | cb_flag_u | cb_flag_v) {
          if (must_code_qp_delta) {
            ac_int< 9, true > qp_delta = qp - ref_qp;
            ac_int< 9, false > qp_delta_abs = Abs< 9 >(qp_delta);

            ac_int< 6, false > ctx_index = 0;

            kvz_cabac_write_unary_max_symbol(cabac_write, 9, ctx_index, MIN(qp_delta_abs, (ac_int< 9, false >)5), 1, 5);

            if (qp_delta_abs >= 5) {
              // cu_qp_delta_abs suffix
              kvz_cabac_write_ep_ex_golomb(cabac_write, 9, ctx_index, qp_delta_abs - 5, 0);
            }

            if (qp_delta != 0) {
              {
#ifndef NO_PRINT
                printf("cur_ctx %d, index %d : ", 9, 0);
                printf("%d %s\n", (qp_delta >= 0 ? 0 : 1), "qp_delta_sign_flag");
#endif

                cabac_val temp_cmd = cabac_param_to_val((qp_delta >= 0 ? 0 : 1), 0, 9, ctx_index, CMD_ENCODE_BIN_EP);
                cabac_write.write(temp_cmd);
              }
            }

            must_code_qp_delta = 0;
            ref_qp = qp;
          }
          done_reads = encode_transform_unit(ctu_id, cabac_main_config, &lcu[16 * 16 * ctu_id], x, y, depth);
        } else {
          done_reads = 0;
        }

        if (done_reads != 0) {
#pragma hls_pipeline_init_interval 1
          for (ac_int< 2, false > aa = 0; aa < 3; aa++) {
            dummy = cabac_done.read();
            if (aa + 1 == done_reads)
              break;
          }
        }
        if ((split == 0 && recurse_pos == 0) || (recurse_pos == 4)) {
          break;
        }
        recurse_return = 1;
      }
    }
    if (dummy == 1 || done_reads == 0) {
      *active = 0;
    }
  }
}

void cabac_main_config_dc(ac_channel< ac_int< 32 + 16 + 16 + 8 + 2 + 5, false > > &cabac_main_config_in, ac_channel< ac_int< 32 + 16 + 16 + 8 + 2 + 5, false > > &cabac_main_config_out) {
#ifndef __SYNTHESIS__
  while (cabac_main_config_in.available(1)) {
#endif

    cabac_main_config_out.write(cabac_main_config_in.read());

#ifndef __SYNTHESIS__
  }
#endif
}

void encode_intra_coding_unit_and_transform_coeff(cabac_chan &cabac_write, encode_config_chan &config_in, ac_channel< ac_int< 32 + 16 + 16 + 8 + 2 + 5, false > > &cabac_main_config,
                                                  ac_channel< ac_int< 1, false > > &cabac_done, cu_info lcu[16 * 16 * LCU_COUNT], cu_info_simple lcu_top[16 * LCU_COUNT],
                                                  cu_info_simple lcu_left[16 * LCU_COUNT], ac_int< 1, false > *active) {
  static ac_channel< ac_int< 32 + 16 + 16 + 8 + 2 + 5, false > > cabac_main_config_hier;
  encode_intra_coding_unit_and_transform_coeff_hier(config_in, cabac_write, cabac_main_config_hier, cabac_done, lcu, lcu_top, lcu_left, active);
  cabac_main_config_dc(cabac_main_config_hier, cabac_main_config);
}
