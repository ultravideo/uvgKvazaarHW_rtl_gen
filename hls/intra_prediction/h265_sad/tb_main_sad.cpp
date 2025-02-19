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
#include <cstdlib>
#include <mc_scverify.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

// template <unsigned N, unsigned M>
class Input {
public:
  Input() { reset(); }

  void operator<<(const ac_int< 8 > &rhs) {
    src[w_id].set_slc(slc * 8, rhs);

    slc++;
    if (!slc)
      w_id++;
  }

  ac_int< PARALLEL_PIX * 8, false > get() { return src[r_id++]; }

  bool empty() { return r_id == w_id; }
  void reset() { slc = r_id = w_id = 0; }

private:
  ac_int< ac::log2_ceil< PARALLEL_PIX >::val, false > slc;
  ac_int< 10, false > r_id;
  ac_int< 10, false > w_id;
  ac_int< PARALLEL_PIX * 8, false > src[1024];
};

// template <unsigned N, unsigned M>
class Output {
public:
  Output() { reset(); }

  void operator<<(const ac_int< PARALLEL_PIX * 8, false > &rhs) { src[w_id++] = rhs; }

  ac_int< 8 > get() {
    ac_int< 8 > s = src[r_id].slc< 8 >(8 * slc++);
    if (!slc)
      r_id++;

    return s;
  }

  bool empty() { return r_id == w_id; }
  void reset() { slc = r_id = w_id = 0; }

private:
  ac_int< ac::log2_ceil< PARALLEL_PIX >::val, false > slc;
  ac_int< 10, false > r_id;
  ac_int< 10, false > w_id;
  ac_int< PARALLEL_PIX * 8, false > src[1024];
};

void main_sad_parallel(conf_t::port_t &conf_in, pred_port_t &pred_out, conf_t::port_t &conf_out, ac_channel< ac_int< 8, false > > &intra_mode_out, ip_port_t &in0, ip_port_t &in1, ip_port_t &in2,
                       ip_port_t &in3, ip_port_t &in4, ip_port_t &in5, ip_port_t &in6, ip_port_t &in7, ip_port_t &in8, ip_port_t &in9, ip_port_t &in10, ip_port_t &in11, ip_port_t &in12,
                       ip_port_t &in13, ip_port_t &in14, ip_port_t &in15, ip_port_t &in16, ip_port_t &in17, ip_port_t &in18, ip_port_t &in19, ip_port_t &in20, ip_port_t &in21, ip_port_t &in22,
                       ip_port_t &in23, ip_port_t &in24, ip_port_t &in25, ip_port_t &in26, ip_port_t &in27, ip_port_t &in28, ip_port_t &in29, ip_port_t &in30, ip_port_t &in31, ip_port_t &in32,
                       ip_port_t &in33, ip_port_t &in34, lcu_yuv_t lcu[LCU_COUNT]);

CCS_MAIN(int argc, char *argv[]) {
  srand(time(NULL));

  static ip_port_t in0;
  static ip_port_t in1;
  static ip_port_t in2;
  static ip_port_t in3;
  static ip_port_t in4;
  static ip_port_t in5;
  static ip_port_t in6;
  static ip_port_t in7;
  static ip_port_t in8;
  static ip_port_t in9;
  static ip_port_t in10;
  static ip_port_t in11;
  static ip_port_t in12;
  static ip_port_t in13;
  static ip_port_t in14;
  static ip_port_t in15;
  static ip_port_t in16;
  static ip_port_t in17;
  static ip_port_t in18;
  static ip_port_t in19;
  static ip_port_t in20;
  static ip_port_t in21;
  static ip_port_t in22;
  static ip_port_t in23;
  static ip_port_t in24;
  static ip_port_t in25;
  static ip_port_t in26;
  static ip_port_t in27;
  static ip_port_t in28;
  static ip_port_t in29;
  static ip_port_t in30;
  static ip_port_t in31;
  static ip_port_t in32;
  static ip_port_t in33;
  static ip_port_t in34;

  static conf_t::port_t config;
  static pred_port_t preds_out;
  static conf_t::port_t conf_out;

  static pixel_t ref[4096 + 1024 + 1024];
  static pixel_t ref_pred[35][32 * 32];
  static lcu_yuv_t lcu[LCU_COUNT];

  static ac_channel< ac_int< 8, false > > intra_mode_out;

  uint_32 sad[35];
  one_bit simulation_successful = 1;

  static ip_port_t *const inputs[35] = {&in0,  &in1,  &in2,  &in3,  &in4,  &in5,  &in6,  &in7,  &in8,  &in9,  &in10, &in11, &in12, &in13, &in14, &in15, &in16, &in17,
                                        &in18, &in19, &in20, &in21, &in22, &in23, &in24, &in25, &in26, &in27, &in28, &in29, &in30, &in31, &in32, &in33, &in34};

  Input /*<8, 4>*/ preds_i[35];
  Output /*<8, 4>*/ preds_o[35];

  bool print = 1;

  for (int i = 0; i < 1; ++i) {
    srand(i);

    for (int x = 0; x < 4096 + 1024 + 1024; ++x) {
      ref[x] = pixel_t(rand());
    }

    const uint_4 lcu_id = rand() % LCU_COUNT;

    ac_int< 4, true > depth = 0;
    for (depth = 3; depth >= 0; --depth) {
      uint_4 color = 0;
      for (color = 0; color < 3; ++color) {

        conf_t conf(0);
        conf.lcu_id = lcu_id;
        conf.depth = depth;
        conf.color = color;
        conf.x_pos = 0;
        conf.y_pos = 0;

        conf.uc_state = 0x09;
        conf.intra_mode = 0x07; // Lambda
        conf.intra_pred0 = 0x00;
        conf.intra_pred1 = 0x01;
        conf.intra_pred2 = 0x1A;

        const uint_6 width = conf.width();
        conf.x_pos = width; // x one block offset
        std::cout << "Width: " << width << " Depth: " << depth << " Color: " << color << " ID: " << lcu_id << std::endl;

        for (int x = 0; x < 768; ++x) {
          lcu[lcu_id].ref[x].set_slc(0, ref[PARALLEL_PIX * x + 0]);
          lcu[lcu_id].ref[x].set_slc(8, ref[PARALLEL_PIX * x + 1]);
          lcu[lcu_id].ref[x].set_slc(16, ref[PARALLEL_PIX * x + 2]);
          lcu[lcu_id].ref[x].set_slc(24, ref[PARALLEL_PIX * x + 3]);
        }

        uint_6 best_mode = 0;
        uint_32 best_cost = uint_32(-1);

        for (int i = 0; i < 35; ++i) {
          uint_32 cost = 0;

          for (int y = 0; y < width; ++y) {
            for (int x = 0; x < width; ++x) {
              pixel_t pred = pixel_t(rand());

              int_9 resid = ref[y * 64 + x + conf.x_pos] - pred;

              preds_i[i] << pred;
              ref_pred[i][y * width + x] = pred;

              cost += std::abs((int)resid);

              if (print && !i & 0)
                std::cout << pred << " ";
            }
            if (print && !i & 0)
              std::cout << std::endl;
          }

          sad[i] = cost;
          while (!preds_i[i].empty()) {
            {
              inputs[i]->write(preds_i[i].get());
            }
          }

          if (cost < best_cost) {
            best_cost = cost;
            best_mode = i;
          }
        }

        conf.intra_mode_chroma = best_mode;
        config.write(conf.toInt());

        CCS_DESIGN(main_sad_parallel)
        (config,
         preds_out,
         conf_out,
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

        // Read first conf
        conf_out.read();

        uint_8 intra_mode = 0;
        // Read all but last output
        while (preds_out.available(1)) {
          preds_t output = preds_out.read();
          one_bit last = output.slc< 1 >(36 * PARALLEL_PIX * 8);
          for (int i = 0; i < 35; ++i) {
            {
              preds_o[i] << output.slc< PARALLEL_PIX * 8 >(PARALLEL_PIX * 8 * i);
            }
          }

          if (last) {
            intra_mode = intra_mode_out.read();
            break;
          }
        }

        for (int i = 0; i < 35; ++i) {
          for (int y = 0; y < width; ++y) {
            for (int x = 0; x < width; ++x) {
              pixel_t ref = ref_pred[i][y * width + x];
              pixel_t duv = preds_o[i].get();

              if (ref != duv) {
                printf("ref %d, duv %d\n", ref, duv);
                std::cout << "Error: Faulty prediction output" << std::endl;
                simulation_successful = 0;
              }

              if (!simulation_successful)
                break;
            }

            if (!simulation_successful)
              break;
          }

          if (!preds_o[i].empty()) {
            std::cout << "Error: TB Output buffer not empty" << std::endl;
            simulation_successful = 0;
          }

          if (!simulation_successful)
            break;
        }

        if (intra_mode != best_mode) {
          std::cout << "Error: Ref: " << best_mode << " Duv: " << intra_mode << std::endl;
          simulation_successful = 0;
        }

        if (!simulation_successful)
          break;
      }
      if (!simulation_successful)
        break;
    }
    if (!simulation_successful)
      break;
  }

  if (simulation_successful == 1)
    cout << "## SIMULATION PASSED ##" << endl;
  else
    cout << "## SIMULATION FAILED ##" << endl;

  CCS_RETURN(0);
}
