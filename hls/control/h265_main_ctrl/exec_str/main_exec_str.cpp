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
#include "tables.h"
#include "types.h"

// --------------------------------------------------------------INLINE-------------------------------------------------------------------------------- \\

uint_6 get_scaled_qp(two_bit color, int_8 qp, int_8 qp_offset) {
  static const uint_6 kvz_g_chroma_scale[58] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
                                                29, 29, 30, 31, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

  uint_6 qp_scaled = 0;

  if (color == COLOR_Y) {
    qp_scaled = qp + qp_offset;
  } else {
    qp_scaled = CLIP(int_8(-qp_offset), int_8(57), qp);
    if (qp < 0) {
      qp_scaled = qp_scaled + qp_offset;
    } else {
      qp_scaled = kvz_g_chroma_scale[qp_scaled] + qp_offset;
    }
  }
  return qp_scaled;
}

void write_ip_conf(ac_channel< int_48 > *ip_ctrl, const ctu_str_conf_t &conf, const cu_info_str_mem_t *cu, const exec_t &inst) {
  uint_6 intra_preds[3];
  uint_6 intra_mode_chroma = 0;
  {
    uint_4 cu_x = (inst.x >> 2), cu_y = (inst.y >> 2);

    // Select left and top CUs if they are available.
    // Top CU is not available across LCU boundary.
    const cu_simple_info_t *top_cu = cu_y != 0 ? LCU_GET_CU_AT_PX(cu, inst.x, uint_6(inst.y - SCU_WIDTH)) : NULL;
    const cu_simple_info_t *left_cu = cu_x != 0 ? LCU_GET_CU_AT_PX(cu, uint_6(inst.x - SCU_WIDTH), inst.y) : (!conf.x_zero ? &cu->cu_hor[cu_y] : NULL);

    // Get intra pred modes
    kvz_intra_get_dir_luma_predictor(intra_preds, left_cu, top_cu);
  }
  // U and V reconstruction according to luma intra mode
  if (inst.color == COLOR_ALL) {
    const cu_simple_info_t *current = LCU_GET_CU_AT_PX(cu, inst.x, inst.y);
    intra_mode_chroma = current->intra_mode();
  }

#pragma hls_pipeline_init_interval 1
  for (two_bit r = 1; r < 3; ++r) {
    two_bit color = (inst.color == COLOR_ALL) ? r : inst.color;

  write_ip:
#pragma hls_pipeline_init_interval 1
    for (two_bit i = 0; i < 2; i++) {
      switch (i) {
      case 0: {
        ip_conf_t ip_conf(inst);
        ip_conf.color = color;

        ip_conf.labda = conf.lambda_sqrt;
        ip_conf.qp_scaled = get_scaled_qp(color, conf.qp, 0);

        ip_conf.x_limit = uint_7(conf.x_limit - inst.x);
        ip_conf.x_zero = one_bit(conf.x_zero && inst.x == 0);
        ip_conf.y_limit = uint_7(conf.y_limit - inst.y);
        ip_conf.y_zero = one_bit(conf.y_zero && inst.y == 0);

        ip_ctrl->write(ip_conf.toInt());
        break;
      }
      case 1: {
        int_48 ip_conf = 0;
        ip_conf.set_slc(0, intra_preds[0]);
        ip_conf.set_slc(6, intra_preds[1]);
        ip_conf.set_slc(12, intra_preds[2]);
        ip_conf.set_slc(18, intra_mode_chroma);
        ip_conf.set_slc(24, conf.cabac);

        ip_ctrl->write(ip_conf);
        break;
      }
      }
    }

    if (inst.color != COLOR_ALL)
      break;
  }
}

// --------------------------------------------------------------BLOCKS-------------------------------------------------------------------------------- \\

#pragma hls_design top
void exec_str(exec_port_t &inst_input, exec_port_t &inst_out, const ctu_str_conf_t::ac_t ctu_str[LCU_COUNT], const cu_info_str_mem_t cu[LCU_COUNT], ac_channel< int_48 > &ip_ctrl, irq_t *irq,
              ac_channel< ac_int< 44 + 4, false > > &init_chroma) {
  const exec_t inst = inst_input.read();
  static one_bit luma_done[LCU_COUNT];
  static bool E = ac::init_array< AC_VAL_0, 1, false >(luma_done, LCU_COUNT);

  *irq = irq_t(0);
  if (inst.op_code != END) {
    inst_out.write(inst);
  }

  // Config
  const ctu_str_conf_t conf(ctu_str[inst.lcu_id]);

  switch (inst.op_code) {
  case IP:
    // Write IP Config
    write_ip_conf(&ip_ctrl, conf, &cu[inst.lcu_id], inst);
    break;
  case END:
    // Signal CTU Done
    if (luma_done[inst.lcu_id]) {
      luma_done[inst.lcu_id] = 0;
      *irq = irq_t(1) << inst.lcu_id;
    } else {
      luma_done[inst.lcu_id] = 1;
      ac_int< 44 + 4, false > init_chroma_conf = conf.toInt();
      init_chroma_conf <<= 4;
      init_chroma_conf |= inst.lcu_id;
      init_chroma.write(init_chroma_conf);
    }
    break;
  }
}
