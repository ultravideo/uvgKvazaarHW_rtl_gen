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

// 15 Bits for decimals
typedef ac_fixed< 26, 11, false > ufixed_26;
typedef ac_fixed< 18, 3, false > ufixed_18;

const ufixed_18 entropy_bits[128] = {
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

#pragma hls_design
void main_sad_parallel_hier(ac_channel< ac_int< 18 * 35 + 2 + 6 + 12 + 6 + 11 + 4 + 8, false > > &cost_init, pred_port_t &pred_out, ac_channel< ac_int< 8, false > > &intra_mode_out, ip_port_t &in0,
                            ip_port_t &in1, ip_port_t &in2, ip_port_t &in3, ip_port_t &in4, ip_port_t &in5, ip_port_t &in6, ip_port_t &in7, ip_port_t &in8, ip_port_t &in9, ip_port_t &in10,
                            ip_port_t &in11, ip_port_t &in12, ip_port_t &in13, ip_port_t &in14, ip_port_t &in15, ip_port_t &in16, ip_port_t &in17, ip_port_t &in18, ip_port_t &in19, ip_port_t &in20,
                            ip_port_t &in21, ip_port_t &in22, ip_port_t &in23, ip_port_t &in24, ip_port_t &in25, ip_port_t &in26, ip_port_t &in27, ip_port_t &in28, ip_port_t &in29, ip_port_t &in30,
                            ip_port_t &in31, ip_port_t &in32, ip_port_t &in33, ip_port_t &in34, lcu_yuv_t lcu[LCU_COUNT]) {
  static ip_port_t *const inputs[35] = {&in0,  &in1,  &in2,  &in3,  &in4,  &in5,  &in6,  &in7,  &in8,  &in9,  &in10, &in11, &in12, &in13, &in14, &in15, &in16, &in17,
                                        &in18, &in19, &in20, &in21, &in22, &in23, &in24, &in25, &in26, &in27, &in28, &in29, &in30, &in31, &in32, &in33, &in34};

  preds_t output;
  uint_18 costs[35];

  // Read config
  ac_int< 18 * 35 + 2 + 6 + 12 + 6 + 11 + 4 + 8, false > cost_temp = cost_init.read();
mode_bits:
#pragma hls_unroll yes
  for (uint_6 i = 0; i < 35; ++i) {

    costs[i] = cost_temp.slc< 18 >(18 * i);
  }

  two_bit color = cost_temp.slc< 2 >(35 * 18);

  bool is_chroma = color != COLOR_Y;
  uint_6 x_pos = cost_temp.slc< 6 >(35 * 18 + 2);
  uint_12 cord_offset = cost_temp.slc< 12 >(35 * 18 + 2 + 6);
  uint_6 block_size = cost_temp.slc< 6 >(35 * 18 + 2 + 6 + 12);
  ac_int< 11, false > loop_limit = cost_temp.slc< 11 >(35 * 18 + 2 + 6 + 12 + 6);
  uint_4 lcu_id = cost_temp.slc< 4 >(35 * 18 + 2 + 6 + 12 + 6 + 11);
  uint_8 intra_mode = cost_temp.slc< 8 >(35 * 18 + 2 + 6 + 12 + 6 + 11 + 4);

#pragma hls_pipeline_init_interval 1
  {
    for (ac_int< 11, false > loops = 0; loops < MAX_WIDTH * MAX_WIDTH; loops += PARALLEL_PIX) {
      uint_6 x = loops & (0x1f >> block_size);
      uint_6 y = loops >> (5 - block_size);
      uint_12 cord = ((uint_12)y << (is_chroma ? 5 : 6)) + x + cord_offset;

      pixel_slc_t refs = lcu[lcu_id].ref[lcu_yuv_t::address(cord + lcu_yuv_t::colorOffset(color))];

    read_preds:
#pragma hls_unroll yes
      for (uint_6 i = 0; i < 35; ++i) {
        pixel_slc_t preds = inputs[i]->read();

      calc_resid:
#pragma hls_unroll yes
        for (uint_4 z = 0; z < PARALLEL_PIX; ++z) {
          pixel_t ref = refs.slc< 8 >(z * 8);
          pixel_t pred = preds.slc< 8 >(z * 8);
          int_9 resid = ref - pred;
          costs[i] += ctl::Abs< 9 >(resid);
        }

        output.set_slc(i * 8 * PARALLEL_PIX, preds);
      }
      output.set_slc(35 * 8 * PARALLEL_PIX, refs);

      if (!color) {
        intra_mode = ctl::MinIndex< 35 >(costs);
      }
      output.set_slc(36 * 8 * PARALLEL_PIX, (ac_int< 1, false >)(((loops + PARALLEL_PIX) == loop_limit)));

      pred_out.write(output);

      if ((loops + PARALLEL_PIX) == loop_limit) {
        intra_mode_out.write(intra_mode);
        break;
      }
    }
  }
}

#pragma hls_design
void pre_calc(conf_t::port_t &conf_in, conf_t::port_t &conf_out, ac_channel< ac_int< 18 * 35 + 2 + 6 + 12 + 6 + 11 + 4 + 8, false > > &cost_init) {

  // Read config
  conf_t::ac_t conf_data = conf_in.read();
  conf_t conf(conf_data);
  conf_out.write(conf_data);

  ac_int< 18 * 35 + 2 + 6 + 12 + 6 + 11 + 4 + 8, false > cost_temp = 0;

  ufixed_26 mode_bits[3];

  mode_bits[0] = (entropy_bits[conf.uc_state ^ 1] + 1) * conf.intra_mode;
  mode_bits[1] = (entropy_bits[conf.uc_state ^ 1] + 2) * conf.intra_mode;
  mode_bits[2] = (entropy_bits[conf.uc_state ^ 0] + 5) * conf.intra_mode;

mode_bits:
#pragma hls_unroll yes
  for (uint_6 i = 0; i < 35; ++i) {
    ufixed_26 bits;

    if (i == conf.intra_pred0)
      bits = mode_bits[0];
    else if (i == conf.intra_pred1 || i == conf.intra_pred2)
      bits = mode_bits[1];
    else
      bits = mode_bits[2];

    cost_temp.set_slc(18 * i, (uint_18)bits.to_int());
  }

  const bool is_chroma = conf.color != COLOR_Y;
  const uint_6 x_px = conf.x_pos >> is_chroma;
  const uint_6 y_px = conf.y_pos >> is_chroma;
  const uint_12 cord_offset = ((uint_12)y_px << (is_chroma ? 5 : 6)) + x_px;

  uint_6 block_size = conf.size();
  ac_int< 11, false > loop_limit = 1024 >> (block_size * 2);

  cost_temp.set_slc(35 * 18, conf.color);
  cost_temp.set_slc(35 * 18 + 2, conf.x_pos);
  cost_temp.set_slc(35 * 18 + 2 + 6, cord_offset);
  cost_temp.set_slc(35 * 18 + 2 + 6 + 12, block_size);
  cost_temp.set_slc(35 * 18 + 2 + 6 + 12 + 6, loop_limit);
  cost_temp.set_slc(35 * 18 + 2 + 6 + 12 + 6 + 11, conf.lcu_id);
  cost_temp.set_slc(35 * 18 + 2 + 6 + 12 + 6 + 11 + 4, conf.intra_mode_chroma);

  cost_init.write(cost_temp);
}

#pragma hls_design top
void main_sad_parallel(conf_t::port_t &conf_in, pred_port_t &pred_out, conf_t::port_t &conf_out, ac_channel< ac_int< 8, false > > &intra_mode_out, ip_port_t &in0, ip_port_t &in1, ip_port_t &in2,
                       ip_port_t &in3, ip_port_t &in4, ip_port_t &in5, ip_port_t &in6, ip_port_t &in7, ip_port_t &in8, ip_port_t &in9, ip_port_t &in10, ip_port_t &in11, ip_port_t &in12,
                       ip_port_t &in13, ip_port_t &in14, ip_port_t &in15, ip_port_t &in16, ip_port_t &in17, ip_port_t &in18, ip_port_t &in19, ip_port_t &in20, ip_port_t &in21, ip_port_t &in22,
                       ip_port_t &in23, ip_port_t &in24, ip_port_t &in25, ip_port_t &in26, ip_port_t &in27, ip_port_t &in28, ip_port_t &in29, ip_port_t &in30, ip_port_t &in31, ip_port_t &in32,
                       ip_port_t &in33, ip_port_t &in34, lcu_yuv_t lcu[LCU_COUNT]) {
  static ac_channel< ac_int< 18 * 35 + 2 + 6 + 12 + 6 + 11 + 4 + 8, false > > cost_init;

  pre_calc(conf_in, conf_out, cost_init);
  main_sad_parallel_hier(cost_init,
                         pred_out,
                         intra_mode_out,
                         in0,
                         in1,
                         in2,
                         in3,
                         in4,
                         in5,
                         in6,
                         in7,
                         in8,
                         in9,
                         in10,
                         in11,
                         in12,
                         in13,
                         in14,
                         in15,
                         in16,
                         in17,
                         in18,
                         in19,
                         in20,
                         in21,
                         in22,
                         in23,
                         in24,
                         in25,
                         in26,
                         in27,
                         in28,
                         in29,
                         in30,
                         in31,
                         in32,
                         in33,
                         in34,
                         lcu);
}
