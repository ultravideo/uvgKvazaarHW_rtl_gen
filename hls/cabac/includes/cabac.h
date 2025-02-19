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

#ifndef CABAC_H
#define CABAC_H

#include "ac_channel.h"
#include "ac_fixed.h"
#include "ac_int.h"

typedef ac_int< 8, true > int_8;
typedef ac_int< 8, false > uint_8;
typedef ac_int< 16, true > int_16;
typedef ac_int< 16, false > uint_16;
typedef ac_int< 32, true > int_32;
typedef ac_int< 32, false > uint_32;
typedef ac_int< 64, false > uint_64;
typedef ac_int< 64, true > int_64;

typedef ac_int< 16, true > coeff_t;

typedef ac_int< 8, false > cu_info_simple;
typedef ac_int< 32, false > cu_info;

#define LCU_COUNT 16

#define KVZ_DATA_CHUNK_SIZE 2048

#define CMD_START 1
#define CMD_ENCODE_BINS_EP 2
#define CMD_ENCODE_BIN 3
#define CMD_ENCODE_BIN_EP 4
#define CMD_STOP 5
#define CMD_DONE_BIT 6
#define CMD_SKIP 7

#define CABAC_VAL_WIDTH (16 + 6 + 5 + 6 + 3)
#define ENCODE_CONFIG_WIDTH 16 + 16 + 16 + 16 + 8 + 1 + 8 + 8 + 1 + 3 + 8 + 8 + 5 + 1

#define SIZE_2Nx2N 0
#define SIZE_NxN 3

typedef ac_int< CABAC_VAL_WIDTH > cabac_val;
typedef ac_int< CABAC_VAL_WIDTH * 3 > cabac_val_three;
typedef ac_int< ENCODE_CONFIG_WIDTH, false > encode_config_val;

typedef ac_channel< cabac_val > cabac_chan;
typedef ac_channel< cabac_val_three > cabac_chan_three;
typedef ac_channel< encode_config_val > encode_config_chan;

typedef struct {
  ac_int< 1, false > run_encoding;

  ac_int< 16, false > x_real;
  ac_int< 16, false > y_real;
  ac_int< 16, false > x_ctu;
  ac_int< 16, false > y_ctu;
  uint_8 depth;
  uint_8 intra_pred_mode_chroma;

  ac_int< 32, false > bin_value_s;
  ac_int< 7, false > ctx;
  ac_int< 7, false > ctx_index;
} coding_tree_command;

typedef struct {
  ac_int< 16, false > bin_value_s;
  ac_int< 6, true > num_bins;
  ac_int< 5, false > ctx_index;
  ac_int< 6, false > ctx;
  ac_int< 3, false > cmd;
} cabac_command;

static cabac_command cabac_val_to_struct(cabac_val temp) {
  cabac_command ret;
  ret.bin_value_s = temp.slc< 16 >(20);
  ret.num_bins = temp.slc< 6 >(14);
  ret.ctx_index = temp.slc< 5 >(9);
  ret.ctx = temp.slc< 6 >(3);
  ret.cmd = temp.slc< 3 >(0);

  return ret;
}

static cabac_val cabac_param_to_val(ac_int< 16, false > bin_value_s, ac_int< 6, true > num_bins, ac_int< 6, false > ctx, ac_int< 5, false > ctx_index, ac_int< 3, false > cmd) {
  cabac_val ret = 0;

  ret.set_slc(0, cmd);
  ret.set_slc(3, ctx);
  ret.set_slc(9, ctx_index);
  ret.set_slc(14, num_bins);
  ret.set_slc(20, bin_value_s);

  return ret;
}

static encode_config_val encode_config_param_to_val(ac_int< 16, false > x_real, ac_int< 16, false > y_real, ac_int< 16, false > x_ctu, ac_int< 16, false > y_ctu, ac_int< 8, true > depth,
                                                    ac_int< 1, false > must_code_qp_delta, ac_int< 8, false > qp, ac_int< 8, false > ref_qp, ac_int< 8, false > mode, ac_int< 3, false > cur_cu_depth,
                                                    ac_int< 8, false > corner, ac_int< 5, false > ctu_id, ac_int< 1, false > &config_qp_values) {
  encode_config_val ret = 64;

  ret.set_slc(0, x_real);
  ret.set_slc(16, y_real);

  ret.set_slc(16 + 16, x_ctu);
  ret.set_slc(16 + 16 + 16, y_ctu);

  ret.set_slc(16 + 16 + 16 + 16, depth);
  ret.set_slc(16 + 16 + 16 + 16 + 8, must_code_qp_delta);
  ret.set_slc(16 + 16 + 16 + 16 + 8 + 1, qp);
  ret.set_slc(16 + 16 + 16 + 16 + 8 + 1 + 8, ref_qp);

  if (config_qp_values == 1) {
    ret[16 + 16 + 16 + 16 + 8 + 1 + 8 + 8] = 1;
    config_qp_values = 0;
  }

  ret.set_slc(16 + 16 + 16 + 16 + 8 + 1 + 8 + 8 + 1, cur_cu_depth);
  ret.set_slc(16 + 16 + 16 + 16 + 8 + 1 + 8 + 8 + 1 + 3, mode);
  ret.set_slc(16 + 16 + 16 + 16 + 8 + 1 + 8 + 8 + 1 + 3 + 8, corner);
  ret.set_slc(16 + 16 + 16 + 16 + 8 + 1 + 8 + 8 + 1 + 3 + 8 + 8, ctu_id);
  ret.set_slc(16 + 16 + 16 + 16 + 8 + 1 + 8 + 8 + 1 + 3 + 8 + 8 + 5, (ac_int< 1, false >)1);

  return ret;
}

static cabac_val cabac_struct_to_val(cabac_command temp) {
  cabac_val ret;

  ret.set_slc(0, temp.cmd);
  ret.set_slc(3, temp.ctx);
  ret.set_slc(9, temp.ctx_index);
  ret.set_slc(14, temp.num_bins);
  ret.set_slc(20, temp.bin_value_s);

  return ret;
}

typedef struct {
  uint_8 uc_state;
} cabac_ctx_t;

typedef struct bitstream_t {
  /// \brief Total number of complete bytes.
  ac_int< 10, false > len;

  /// \brief The incomplete byte.
  uint_8 data;

  /// \brief Number of bits in the incomplete byte.
  uint_8 cur_bit;

  uint_8 zerocount;
} bitstream_t;

typedef struct {
  uint_32 low;
  uint_32 range;
  uint_32 buffered_byte;
  int_32 num_buffered_bytes;
  int_32 bits_left;
  int_8 only_count;

  // CONTEXTS
  struct {
    cabac_ctx_t sao_merge_flag_model;
    cabac_ctx_t sao_type_idx_model;
    cabac_ctx_t split_flag_model[3]; //!< \brief split flag context models
    cabac_ctx_t intra_mode_model;    //!< \brief intra mode context models
    cabac_ctx_t chroma_pred_model[2];
    cabac_ctx_t inter_dir[5];
    cabac_ctx_t trans_subdiv_model[3]; //!< \brief intra mode context models
    cabac_ctx_t qt_cbf_model_luma[4];
    cabac_ctx_t qt_cbf_model_chroma[4];
    cabac_ctx_t cu_qp_delta_abs[4];
    cabac_ctx_t part_size_model[4];
    cabac_ctx_t cu_sig_coeff_group_model[4];
    cabac_ctx_t cu_sig_model_luma[27];
    cabac_ctx_t cu_sig_model_chroma[15];
    cabac_ctx_t cu_ctx_last_y_luma[15];
    cabac_ctx_t cu_ctx_last_y_chroma[15];
    cabac_ctx_t cu_ctx_last_x_luma[15];
    cabac_ctx_t cu_ctx_last_x_chroma[15];
    cabac_ctx_t cu_one_model_luma[16];
    cabac_ctx_t cu_one_model_chroma[8];
    cabac_ctx_t cu_abs_model_luma[4];
    cabac_ctx_t cu_abs_model_chroma[2];
    cabac_ctx_t cu_pred_mode_model;
    cabac_ctx_t cu_skip_flag_model[3];
    cabac_ctx_t cu_merge_idx_ext_model;
    cabac_ctx_t cu_merge_flag_ext_model;
    cabac_ctx_t cu_transquant_bypass;
    cabac_ctx_t cu_mvd_model[2];
    cabac_ctx_t cu_ref_pic_model[2];
    cabac_ctx_t mvp_idx_model[2];
    cabac_ctx_t cu_qt_root_cbf_model;
    cabac_ctx_t transform_skip_model_luma;
    cabac_ctx_t transform_skip_model_chroma;
  } ctx;
} cabac_data_input;

typedef struct {
  ac_int< 9, false > range;

  ac_int< 32, false > low;
  ac_int< 8, false > buffered_byte;
  ac_int< 3, false > num_buffered_bytes;
  ac_int< 6, false > bits_left;
  ac_int< 8, false > only_count;
  uint_8 *cur_ctx;
  bitstream_t stream;

} cabac_data_t;

static uint_8 kvz_g_auc_next_state_mps[128] = {2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,
                                               28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,
                                               54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
                                               80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99,  100, 101, 102, 103, 104, 105,
                                               106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 124, 125, 126, 127};

static uint_8 kvz_g_auc_next_state_lps[128] = {1,  0,  0,  1,  2,  3,  4,  5,  4,  5,  8,  9,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 18, 19, 22, 23, 22, 23, 24,  25,
                                               26, 27, 26, 27, 30, 31, 30, 31, 32, 33, 32, 33, 36, 37, 36, 37, 38, 39, 38, 39, 42, 43, 42, 43, 44, 45, 44, 45, 46, 47, 48,  49,
                                               48, 49, 50, 51, 52, 53, 52, 53, 54, 55, 54, 55, 56, 57, 58, 59, 58, 59, 60, 61, 60, 61, 60, 61, 62, 63, 64, 65, 64, 65, 66,  67,
                                               66, 67, 66, 67, 68, 69, 68, 69, 70, 71, 70, 71, 70, 71, 72, 73, 72, 73, 72, 73, 74, 75, 74, 75, 74, 75, 76, 77, 76, 77, 126, 127};

static uint_8 kvz_g_auc_lpst_table[64][4] = {
    {128, 176, 208, 240}, {128, 167, 197, 227}, {128, 158, 187, 216}, {123, 150, 178, 205}, {116, 142, 169, 195}, {111, 135, 160, 185}, {105, 128, 152, 175}, {100, 122, 144, 166},
    {95, 116, 137, 158},  {90, 110, 130, 150},  {85, 104, 123, 142},  {81, 99, 117, 135},   {77, 94, 111, 128},   {73, 89, 105, 122},   {69, 85, 100, 116},   {66, 80, 95, 110},
    {62, 76, 90, 104},    {59, 72, 86, 99},     {56, 69, 81, 94},     {53, 65, 77, 89},     {51, 62, 73, 85},     {48, 59, 69, 80},     {46, 56, 66, 76},     {43, 53, 63, 72},
    {41, 50, 59, 69},     {39, 48, 56, 65},     {37, 45, 54, 62},     {35, 43, 51, 59},     {33, 41, 48, 56},     {32, 39, 46, 53},     {30, 37, 43, 50},     {29, 35, 41, 48},
    {27, 33, 39, 45},     {26, 31, 37, 43},     {24, 30, 35, 41},     {23, 28, 33, 39},     {22, 27, 32, 37},     {21, 26, 30, 35},     {20, 24, 29, 33},     {19, 23, 27, 31},
    {18, 22, 26, 30},     {17, 21, 25, 28},     {16, 20, 23, 27},     {15, 19, 22, 25},     {14, 18, 21, 24},     {14, 17, 20, 23},     {13, 16, 19, 22},     {12, 15, 18, 21},
    {12, 14, 17, 20},     {11, 14, 16, 19},     {11, 13, 15, 18},     {10, 12, 15, 17},     {10, 12, 14, 16},     {9, 11, 13, 15},      {9, 11, 12, 14},      {8, 10, 12, 14},
    {8, 9, 11, 13},       {7, 9, 11, 12},       {7, 9, 10, 12},       {7, 8, 10, 11},       {6, 8, 9, 11},        {6, 7, 9, 10},        {6, 7, 8, 9},         {2, 2, 2, 2}};

static ac_int< 3, false > kvz_g_auc_renorm_table[32] = {6, 5, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

void print_cabac(cabac_data_t *src);
void copy_cabac_ctx(cabac_data_t *dst, uint_8 *src);
void copy_cabac_ctx_back(uint_8 *dst, cabac_data_t *src);

#define CABAC_BIN(data, value, name) kvz_cabac_encode_bin((data), (value));
#define CABAC_BINS_EP(data, value, bins, name) kvz_cabac_encode_bins_ep((data), (value), (bins));
#define CABAC_BIN_EP(data, value, name) kvz_cabac_encode_bin_ep((data), (value));

#define CTX_STATE(ctx) ((ctx).slc< 7 >(1))
#define CTX_MPS(ctx) ((ctx).slc< 1 >(0))
#define CTX_UPDATE_LPS(ctx, ctx_val)                                                                                                                                                                   \
  { (ctx) = kvz_g_auc_next_state_lps[(ctx_val)]; }
#define CTX_UPDATE_MPS(ctx, ctx_val)                                                                                                                                                                   \
  { (ctx) = kvz_g_auc_next_state_mps[(ctx_val)]; }

#endif
