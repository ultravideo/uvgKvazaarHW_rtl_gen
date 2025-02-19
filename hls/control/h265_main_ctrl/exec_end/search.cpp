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

#include "search.h"

two_bit get_ctx_cu_split_model(const cu_simple_info_t *left_cu, const cu_simple_info_t *above_cu, two_bit depth) {
  one_bit condA = left_cu && left_cu->depth() > depth;
  one_bit condL = above_cu && above_cu->depth() > depth;
  return (two_bit)condA + (two_bit)condL;
}

ufixed_t cu_rd_cost(cabac_t *cabac, two_bit depth, two_bit color, uint_8 lambda, uint_26 ssd, uint_26 coeff_bits) {
  ufixed_t tr_tree_bits = 0;

  // Add transform_tree split_transform_flag bit cost.
  if (color == COLOR_Y && depth != 3) {
    const cabac_ctx_t uc_state = cabac->trans_subdiv_model()[depth];
    tr_tree_bits += f_entropy_bits[uc_state ^ 0];
  }

  // Add transform_tree cbf_luma bit cost.
  if (color == COLOR_Y || depth != 3) {
    const cabac_ctx_t uc_state = !color ? *cabac->qt_cbf_model_luma() : *cabac->qt_cbf_model_chroma();
    tr_tree_bits += f_entropy_bits[uc_state ^ (coeff_bits != 0)];
  }

  ufixed_t bits = tr_tree_bits + coeff_bits;
  return ssd * (!color ? LUMA_MULT : CHROMA_MULT) + (bits * lambda);
}

ufixed_t calc_mode_bits(const ctu_stack_t &cur_stack, const cu_simple_info_t *left_cu, const cu_simple_info_t *above_cu, cabac_t *cabac, two_bit depth, two_bit color) {
  ufixed_t mode_bits = 0;

  uint_6 intra_preds[3];
  kvz_intra_get_dir_luma_predictor(intra_preds, left_cu, above_cu);

  one_bit mode_in_preds = cur_stack.mode == intra_preds[0] || cur_stack.mode == intra_preds[1] || cur_stack.mode == intra_preds[2];
  one_bit chroma_is_luma = cur_stack.mode_chroma == cur_stack.mode;

  if (!color) {
    const cabac_ctx_t uc_state = *cabac->intra_mode_model();
    if (mode_in_preds) {
      mode_bits = f_entropy_bits[uc_state ^ 1] + ((cur_stack.mode == intra_preds[0]) ? 1 : 2);
    } else {
      mode_bits = f_entropy_bits[uc_state ^ 0] + 5;
    }
  } else {
    const cabac_ctx_t uc_state = *cabac->chroma_pred_model();
    if (chroma_is_luma) {
      mode_bits = f_entropy_bits[uc_state ^ 0];
    } else {
      mode_bits = f_entropy_bits[uc_state ^ 1] + 2;
    }
  }

  return mode_bits;
}

ufixed_t main_cost(const ctu_end_conf_t &conf, const cu_info_mem_t *cu, cabac_t *cabac, ctu_stack_t &cur_stack, two_bit depth, two_bit color, int_192 costs, one_bit &skip) {
  uint_4 cu_x = cur_stack.x / SCU_WIDTH, cu_y = cur_stack.y / SCU_WIDTH;

  cu_simple_info_t _left_cu = cu_simple_info_t(cu->cu[cu_y * LCU_CU_WIDTH + uint_4(cu_x - 1)]);
  const cu_simple_info_t *left_cu = cu_x != 0 ? &_left_cu : (!conf.x_zero ? &cu->cu_ver[cu_y] : NULL);

  cu_simple_info_t _above_cu = cu_simple_info_t(cu->cu[uint_4(cu_y - 1) * LCU_CU_WIDTH + cu_x]);
  const cu_simple_info_t *above_cu = cu_y != 0 ? &_above_cu : (!conf.y_zero ? &cu->cu_hor[cu_x] : NULL);

  // Use dual RD cost for 4x4 blocks
  const one_bit is_4x4 = (depth == 3) || (color && depth == 2);

  // Split luma and chroma costs
  uint_26 ssd[3] = {costs.slc< 26 >(16), costs.slc< 26 >(42), costs.slc< 26 >(68)};
  uint_26 coeff_bits[3] = {costs.slc< 26 >(96), costs.slc< 26 >(128), costs.slc< 26 >(160)};

  // RD Cost
  ufixed_t cost, cost_1st, cost_2nd;
  cost_1st = cu_rd_cost(cabac, depth, color, conf.lambda, ssd[is_4x4 ? 1 : 0], coeff_bits[is_4x4 ? 1 : 0]);
  cost_2nd = cu_rd_cost(cabac, depth, color, conf.lambda, ssd[is_4x4 ? 2 : 0], coeff_bits[is_4x4 ? 2 : 0]);

  // Mode bits
  ufixed_t mode_bits = calc_mode_bits(cur_stack, left_cu, above_cu, cabac, depth, color);

  one_bit has_coeffs;
  if (!color && is_4x4) {
    // Use transform skip for 4x4 luma if it is better and enabled
    one_bit use_trskip = conf.trskip_enable && (cost_2nd < cost_1st);

    cost = use_trskip ? cost_2nd : cost_1st;
    has_coeffs = coeff_bits[use_trskip ? 2 : 1] != 0;

    // Save settings
    cur_stack.trskip = use_trskip;
  } else {
    cost = cost_1st;
    has_coeffs = coeff_bits[is_4x4 ? 1 : 0] != 0;
  }

  cost += mode_bits * conf.lambda;

  // Set has coeffs flag
  cur_stack.coeffs[color] = has_coeffs;

  if (!color && depth != MAX_DEPTH) {
    if (depth != (MAX_DEPTH - 1)) {
      // Ref: if (depth < (MAX_DEPTH - 1))     >  8x8
      two_bit split_model = get_ctx_cu_split_model(left_cu, above_cu, depth);

      const cabac_ctx_t uc_state = cabac->split_flag_model()[split_model];
      cost += f_entropy_bits[uc_state ^ 0] * conf.lambda;
      cur_stack.split_cost += f_entropy_bits[uc_state ^ 1] * conf.lambda;
    } else {
      // Ref: if (depth == (MAX_DEPTH - 1))    == 8x8
      const cabac_ctx_t uc_state = *cabac->part_size_model();
      cost += f_entropy_bits[uc_state ^ 1] * conf.lambda;
      cur_stack.split_cost += f_entropy_bits[uc_state ^ 0] * conf.lambda;
    }
  }

  if (!color) {
    // Skip if no luma coeffs is found
    skip = !cur_stack.coeffs[0];
  } else {
    // Ignore chroma for MD
    cost = 0;
  }

  // Add current cost for this CU
  cur_stack.cost += cost;

  // Return cost for this CU cost
  return cost;
}
