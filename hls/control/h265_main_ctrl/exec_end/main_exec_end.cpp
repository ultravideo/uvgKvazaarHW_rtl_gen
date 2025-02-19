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
#include "intra.h"
#include "search.h"
#include "tables.h"
#include "types.h"

// --------------------------------------------------------------INLINE-------------------------------------------------------------------------------- \\

void set_cu(cu_info_mem_t *cu, const ctu_stack_t &cur_cu, two_bit depth, two_bit color) {
  cu_info_t *const cu_ptr = LCU_GET_CU_AT_PX(cu, cur_cu.x, cur_cu.y);

  // Take a local copy to avoid read writes
  cu_info_t cu_out = *cu_ptr;

  if (!color) {
    // Clear old value
    cu_out.set_cbf(0);
  }

  if (!color) {
    cu_out.cbf_clear_color(COLOR_Y);
    if (cur_cu.coeffs[COLOR_Y])
      cu_out.cbf_set(depth, COLOR_Y);
  }
  if (color == COLOR_U || color == COLOR_ALL) {
    cu_out.cbf_clear_color(COLOR_U);
    if (cur_cu.coeffs[COLOR_U])
      cu_out.cbf_set(depth, COLOR_U);
  }
  if (color == COLOR_V || color == COLOR_ALL) {
    cu_out.cbf_clear_color(COLOR_V);
    if (cur_cu.coeffs[COLOR_V])
      cu_out.cbf_set(depth, COLOR_V);
  }

  if (!color) {
  set_cu:
#pragma hls_pipeline_init_interval 1
    for (int y = 0; y < 8; ++y) {
      for (int x = 0; x < 8; ++x) {
        uint_6 x_cur = cur_cu.x + 4 * x;
        uint_6 y_cur = cur_cu.y + 4 * y;

        // Child CU
        cu_info_t *to = LCU_GET_CU_AT_PX(cu, x_cur, y_cur);

        to->set_cu_info(cur_cu.mode, cur_cu.mode_chroma, depth, cur_cu.trskip, (y == 0 && x == 0) ? cu_out.get_cbf() : (uint_16)0);

        if (x == (7 >> depth))
          break;
      }
      if (y == (7 >> depth))
        break;
    }
  } else {
    cu_ptr->set_cbf(cu_out.get_cbf());
  }
}

two_bit select_cu(ctu_stack_t::ac_t *ctu_stack, two_bit str_depth, two_bit end_depth, ctu_stack_t &selected_cu) {
  two_bit selected_depth = str_depth;

select_cu:
  for (two_bit i = 0; i < 3; ++i) {
    two_bit cur_depth = str_depth - 1 - i;
    two_bit top_depth = str_depth - 2 - i;

    ctu_stack_t cur_cu(ctu_stack[cur_depth]);
    ctu_stack_t top_cu(ctu_stack[top_depth]);

    // Disable split if smaller CUs were skipped
    bool no_split = !cur_cu.split_valid;

    // Split if split_cost is smaller
    fixed_t diff = cur_cu.cost - cur_cu.split_cost;

    if (no_split || diff <= 0) {
      selected_cu = cur_cu;
      selected_depth = cur_depth;
    } else {
      // Update top CU split cost
      top_cu.split_cost -= diff;
    }

    ctu_stack[top_depth] = top_cu.toInt();
    if (cur_depth == end_depth)
      break;
  }

  return selected_depth;
}

void fill_cu(ctu_stack_t &cur_cu, two_bit color, uint_6 x_px, uint_6 y_px, uint_6 mode) {
  if (color == COLOR_Y) {
    cur_cu.x = x_px;
    cur_cu.y = y_px;
    cur_cu.mode = mode;
    cur_cu.mode_chroma = mode;

    // Empty old values
    cur_cu.trskip = 0;
    cur_cu.coeffs[0] = 0;
    cur_cu.coeffs[1] = 0;
    cur_cu.coeffs[2] = 0;

    cur_cu.cost = 0;
    cur_cu.split_cost = 0;

    cur_cu.split_valid = false;
  } else {
    cur_cu.x = x_px;
    cur_cu.y = y_px;

    cur_cu.mode_chroma = mode;
  }
}

void flush_buffer(ac_channel< ac_int< 17 + 5 + 1, false > > *conf_out, const ctu_stack_t &cur_cu, uint_4 lcu_id, two_bit depth, two_bit color, ac_int< 5, false > inst_done, one_bit flush) {
  ac_int< 17 + 5 + 1, false > output;

  output.set_slc(0, lcu_id);
  output.set_slc(4, depth);
  output.set_slc(6, color);

  output.set_slc(8, uint_4(cur_cu.x >> 2));
  output.set_slc(12, uint_4(cur_cu.y >> 2));

  output.set_slc(16, cur_cu.trskip);
  output.set_slc(17, inst_done);
  output.set_slc(22, flush);

  conf_out->write(output);
}

// --------------------------------------------------------------BLOCKS-------------------------------------------------------------------------------- \\

#pragma hls_design
void exec_end(exec_port_t &inst_input, ac_channel< int_192 > &result_in, ctu_end_conf_t::ac_t ctu_end[LCU_COUNT], cu_info_mem_t cu[LCU_COUNT], ac_int< 128 > cabac[LCU_COUNT],
              ac_channel< ac_int< 17 + 5 + 1, false > > &buffer) {
#ifndef __SYNTHESIS__
  while (inst_input.available(1) || result_in.available(1)) {
#endif
    static ctu_stack_t::ac_t stack[LCU_COUNT][4];

    const exec_t inst = inst_input.read();

    one_bit skip = false;
    two_bit depth = inst.depth;
    two_bit color = inst.color;
    one_bit flush = 0;

    // LCU Configuration
    const ctu_end_conf_t conf = ctu_end_conf_t(ctu_end[inst.lcu_id]);

    cu_info_mem_t *ctu_cu = &cu[inst.lcu_id];

    ac_int< 128 > cabac_temp = cabac[inst.lcu_id];
    cabac_t cabac_reg;
#pragma hls_unroll yes
    for (ac_int< 5, false > aa = 0; aa < 12; aa++) {
      cabac_reg.cabac[aa] = cabac_temp.slc< 8 >(aa * 8);
    }
    cabac_t *ctu_cabac = &cabac_reg;

    ctu_stack_t::ac_t *ctu_stack = stack[inst.lcu_id];

    ctu_stack_t cur_cu(ctu_stack[depth]);
    ctu_stack_t top_cu(ctu_stack[two_bit(depth - 1)]);

    switch (inst.op_code) {
    case IP: {
      for (two_bit r = 1; r < 3; ++r) {
        color = (inst.color == COLOR_ALL) ? r : inst.color;

        const int_192 result = result_in.read();
        const uint_6 intra_mode = result >> 8;

        fill_cu(cur_cu, color, inst.x, inst.y, intra_mode);

        ufixed_t cost = main_cost(conf, ctu_cu, ctu_cabac, cur_cu, depth, color, result, skip);

        // Update split cost and mark split as valid for top CU
        top_cu.split_cost += cost;
        top_cu.split_valid = true;

        if (inst.color != COLOR_ALL)
          break;
      }

      ctu_stack[depth] = cur_cu.toInt();
      ctu_stack[two_bit(depth - 1)] = top_cu.toInt();
      break;
    }
    case CMP:
      // Compare and Select best CU
      depth = select_cu(ctu_stack, conf.max_depth, inst.depth, cur_cu);
      break;
    }

    if ((inst.op_code == IP && (inst.color == COLOR_ALL || skip || inst.depth >= conf.max_depth)) || (inst.op_code == CMP && depth != conf.max_depth)) {
      flush = 1;
      set_cu(ctu_cu, cur_cu, depth, inst.color);
    }

    ac_int< 5, false > inst_done_temp = skip;
    inst_done_temp <<= 4;
    inst_done_temp |= inst.lcu_id;

    flush_buffer(&buffer, cur_cu, inst.lcu_id, depth, inst.color, inst_done_temp, flush);
#ifndef __SYNTHESIS__
  }
#endif
}
