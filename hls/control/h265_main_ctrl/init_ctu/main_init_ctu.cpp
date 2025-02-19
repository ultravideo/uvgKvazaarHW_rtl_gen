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

template < int Depth > void init_op(const ctu_str_conf_t *conf, uint_6 x_base, uint_6 y_base, inst_t::ac_t *insts, pc_t &pc, one_bit chroma, bool force_search) {
  const static uint_6 width = (LCU_WIDTH / 2) >> Depth;

  for (uint_4 i = 0; i < 4; i++) {
    uint_6 x_local = x_base + (i.slc< 1 >(0) ? width : uint_6(0));
    uint_6 y_local = y_base + (i.slc< 1 >(1) ? width : uint_6(0));

    pc_t pc_local = 0;
    bool search = false;

    bool in_depth = Depth >= conf->min_depth && Depth <= conf->max_depth;
    bool in_frame = (x_local + width) <= conf->x_limit && (y_local + width) <= conf->y_limit;

    // Enable search if CU is in frame and within assingned depths
    if ((in_depth || force_search) && in_frame) {
      search = true;
      pc_local = pc;
      pc += 1;
    }

    init_op< Depth + 1 >(conf, x_local, y_local, insts, pc, chroma, (in_depth || force_search) && !in_frame);

    if (search) {
      if ((i != 3 || Depth == conf->min_depth) && Depth < conf->max_depth) {
        insts[pc] = inst_t(CMP, Depth, COLOR_Y, x_local, y_local, pc + 1).toInt();
        pc += 1;
      }
      // Intra Prediction
      insts[pc_local] = inst_t(IP, Depth, COLOR_Y, x_local, y_local, pc).toInt();
    }
  }
}

template <> void init_op< 3 >(const ctu_str_conf_t *conf, uint_6 x_base, uint_6 y_base, inst_t::ac_t *insts, pc_t &pc, one_bit chroma, bool force_search) {
  const static uint_6 width = 4;
  const static two_bit depth = 3;

#pragma hls_pipeline_init_interval 1
  for (uint_4 i = 0; i < 4; i++) {
    uint_6 x_local = x_base + (i.slc< 1 >(0) ? width : uint_6(0));
    uint_6 y_local = y_base + (i.slc< 1 >(1) ? width : uint_6(0));

    // Search only if CU is completely inside the frame
    if ((x_local + width) <= conf->x_limit && (y_local + width) <= conf->y_limit && conf->max_depth == depth) {
      insts[pc] = inst_t(IP, depth, COLOR_Y, x_local, y_local, pc + 1).toInt();
      pc += 1;
    }
  }
}

// --------------------------------------------------------------BLOCKS-------------------------------------------------------------------------------- \\

uint_8 cu_scan[256] = {
    0,   1,   16,  17,  2,   3,   18,  19,  32,  33,  48,  49,  34,  35,  50,  51,  4,   5,   20,  21,  6,   7,   22,  23,  36,  37,  52,  53,  38,  39,  54,  55,  64,  65,  80,  81,  66,
    67,  82,  83,  96,  97,  112, 113, 98,  99,  114, 115, 68,  69,  84,  85,  70,  71,  86,  87,  100, 101, 116, 117, 102, 103, 118, 119, 8,   9,   24,  25,  10,  11,  26,  27,  40,  41,
    56,  57,  42,  43,  58,  59,  12,  13,  28,  29,  14,  15,  30,  31,  44,  45,  60,  61,  46,  47,  62,  63,  72,  73,  88,  89,  74,  75,  90,  91,  104, 105, 120, 121, 106, 107, 122,
    123, 76,  77,  92,  93,  78,  79,  94,  95,  108, 109, 124, 125, 110, 111, 126, 127, 128, 129, 144, 145, 130, 131, 146, 147, 160, 161, 176, 177, 162, 163, 178, 179, 132, 133, 148, 149,
    134, 135, 150, 151, 164, 165, 180, 181, 166, 167, 182, 183, 192, 193, 208, 209, 194, 195, 210, 211, 224, 225, 240, 241, 226, 227, 242, 243, 196, 197, 212, 213, 198, 199, 214, 215, 228,
    229, 244, 245, 230, 231, 246, 247, 136, 137, 152, 153, 138, 139, 154, 155, 168, 169, 184, 185, 170, 171, 186, 187, 140, 141, 156, 157, 142, 143, 158, 159, 172, 173, 188, 189, 174, 175,
    190, 191, 200, 201, 216, 217, 202, 203, 218, 219, 232, 233, 248, 249, 234, 235, 250, 251, 204, 205, 220, 221, 206, 207, 222, 223, 236, 237, 252, 253, 238, 239, 254, 255,
};

#pragma hls_design top
void init_ctu(ac_channel< int_32 > &conf_in, ctu_init_t::port_t &scheduler, inst_t::ac_t inst[LCU_COUNT][CTU_INST_STACK_SIZE], ctu_str_conf_t::ac_t ctu_str[LCU_COUNT],
              ctu_end_conf_t::ac_t ctu_end[LCU_COUNT], cu_simple_info_t cu_feedback[LCU_COUNT][256], ac_channel< ac_int< 44 + 4, false > > &init_chroma_in, one_bit conf_in_lz,
              one_bit init_chroma_lz) {
  ctu_str_conf_t str_conf;
  ctu_end_conf_t end_conf;
  ctu_init_t ctu_conf;

  uint_4 lcu_id = 0;
  // one_bit chroma_search = 0;
  one_bit luma_start = 0;
  one_bit chroma_start = 0;

  static one_bit chroma_search[LCU_COUNT];
  static bool E = ac::init_array< AC_VAL_0, 1, false >(chroma_search, LCU_COUNT);

  if (conf_in_lz) {
    int_32 conf = conf_in.read();
    lcu_id = conf.slc< 4 >(0);

    chroma_search[lcu_id] = conf.slc< 1 >(9);
    str_conf.min_depth = end_conf.min_depth = conf.slc< 2 >(4);
    str_conf.max_depth = end_conf.max_depth = conf.slc< 2 >(6);

    // Flags
    end_conf.trskip_enable = conf.slc< 1 >(8);

    // Horizontal limits
    str_conf.x_limit = conf.slc< 7 >(16);
    str_conf.x_zero = end_conf.x_zero = conf.slc< 1 >(23);

    // Vertical limits
    str_conf.y_limit = conf.slc< 7 >(24);
    str_conf.y_zero = end_conf.y_zero = conf.slc< 1 >(31);

    conf = conf_in.read();
    str_conf.qp = conf.slc< 8 >(0);
    str_conf.cabac = conf.slc< 8 >(8);

    // Lambda
    end_conf.lambda = conf.slc< 8 >(16);
    str_conf.lambda_sqrt = conf.slc< 8 >(24);
    luma_start = 1;
  } else if (init_chroma_lz) {
    uint_7 cu_offset[4] = {64, 16, 4, 4};
    ac_int< 44 + 4, false > init_chroma_conf = init_chroma_in.read();
    lcu_id = init_chroma_conf;
    init_chroma_conf >>= 4;
    str_conf = ctu_str_conf_t(init_chroma_conf);
    pc_t inst_pc = 0;
    inst_t::ac_t *inst_set = &inst[lcu_id][inst_pc];
    uint_9 index = 0;
    if (chroma_search[lcu_id]) {
      two_bit depth = 0;

#pragma hls_pipeline_init_interval 2
      for (uint_9 i = 0; i < 256; i++) {
        uint_8 pos = cu_scan[index];
        uint_6 x_local = pos.slc< 4 >(0) * 4;
        uint_6 y_local = pos.slc< 4 >(4) * 4;
        bool in_frame = (x_local < str_conf.x_limit) && (y_local < str_conf.y_limit);
        cu_simple_info_t cu_temp = cu_feedback[lcu_id][pos];

        if (in_frame) {
          depth = cu_temp.depth();
          inst_set[inst_pc] = inst_t(IP, depth, COLOR_ALL, x_local, y_local, inst_pc + 1).toInt();
          inst_pc++;
        }

        index = index + cu_offset[depth];

        if (index >= 256) {
          break;
        }
      }
    }
    inst_set[inst_pc] = inst_t(END, 0, 0, 0, 0, 0).toInt();
    chroma_start = 1;
  }

  if (luma_start) {
    // If configuration matches a preset, use that instead of generating it
    if (str_conf.x_limit >= LCU_WIDTH && str_conf.y_limit >= LCU_WIDTH) {
      // Use preset only when lcu is completely within the frame
      if (str_conf.min_depth == 0 && str_conf.max_depth == 0) {
        // pu-depth-intra 1-1
        ctu_conf.inst_set = 0;
        ctu_conf.use_preset = 1;
      } else if (str_conf.min_depth == 0 && str_conf.max_depth == 1) {
        // pu-depth-intra 1-2
        ctu_conf.inst_set = 1;
        ctu_conf.use_preset = 1;
      } else if (str_conf.min_depth == 0 && str_conf.max_depth == 2) {
        // pu-depth-intra 1-3
        ctu_conf.inst_set = 2;
        ctu_conf.use_preset = 1;
      } else if (str_conf.min_depth == 0 && str_conf.max_depth == 3) {
        // pu-depth-intra 1-4
        ctu_conf.inst_set = 3;
        ctu_conf.use_preset = 1;
      } else if (str_conf.min_depth == 1 && str_conf.max_depth == 1) {
        // pu-depth-intra 2-2
        ctu_conf.inst_set = 4;
        ctu_conf.use_preset = 1;
      } else if (str_conf.min_depth == 1 && str_conf.max_depth == 2) {
        // pu-depth-intra 2-3
        ctu_conf.inst_set = 5;
        ctu_conf.use_preset = 1;
      } else if (str_conf.min_depth == 1 && str_conf.max_depth == 3) {
        // pu-depth-intra 2-4
        ctu_conf.inst_set = 6;
        ctu_conf.use_preset = 1;
      } else if (str_conf.min_depth == 2 && str_conf.max_depth == 2) {
        // pu-depth-intra 3-3
        ctu_conf.inst_set = 7;
        ctu_conf.use_preset = 1;
      } else if (str_conf.min_depth == 2 && str_conf.max_depth == 3) {
        // pu-depth-intra 3-4
        ctu_conf.inst_set = 8;
        ctu_conf.use_preset = 1;
      } else if (str_conf.min_depth == 3 && str_conf.max_depth == 3) {
        // pu-depth-intra 4-4
        ctu_conf.inst_set = 9;
        ctu_conf.use_preset = 1;
      }
    }

    if (!ctu_conf.use_preset) {
      // Generate instruction set
      pc_t inst_pc = 0;
      inst_t::ac_t *inst_set = &inst[lcu_id][inst_pc];

      init_op< 0 >(&str_conf, 0, 0, inst_set, inst_pc, chroma_search, false);

      // Add END instruction
      inst_set[inst_pc] = inst_t(END, 0, 0, 0, 0, 0).toInt();
    }
  }

  if (luma_start || chroma_start) {
    ctu_conf.lcu_id = lcu_id;
    scheduler.write(ctu_conf);
    ctu_str[lcu_id] = str_conf.toInt();
    ctu_end[lcu_id] = end_conf.toInt();
  }
}
