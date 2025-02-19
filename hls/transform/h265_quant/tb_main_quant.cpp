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
#include <cmath>
#include <iostream>
#include <mc_scverify.h>

using namespace std;

#define QUANT_SHIFT 14
typedef int_16 coeff_t;

const int_8 kvz_g_convert_to_bit[LCU_WIDTH + 1] = {-1, -1, -1, -1, 0,  -1, -1, -1, 1,  -1, -1, -1, -1, -1, -1, -1, 2,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3,
                                                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4};

const int_16 kvz_g_inv_quant_scales[6] = {40, 45, 51, 57, 64, 72};
const uint_8 kvz_g_chroma_scale[58] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
                                       29, 29, 30, 31, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

int_32 kvz_get_scaled_qp(int_8 type, int_8 qp, int_8 qp_offset) {
  int_32 qp_scaled = 0;
  if (type == 0) {
    qp_scaled = qp + qp_offset;
  } else {
    qp_scaled = CLIP((int_8)(-qp_offset), (int_8)57, qp);
    if (qp_scaled < 0) {
      qp_scaled = qp_scaled + qp_offset;
    } else {
      qp_scaled = kvz_g_chroma_scale[qp_scaled] + qp_offset;
    }
  }
  return qp_scaled;
}

void kvz_quant_generic(coeff_t *coef, coeff_t *q_coef, int_8 qp, int_32 width, int_32 height, int_8 type) {
  const int_8 bitdepth = 8;
  static const uint_16 kvz_q_quant_scales[6] = {26214, 23302, 20560, 18396, 16384, 14564};

  const int_32 qp_scaled = kvz_get_scaled_qp(type, qp, (bitdepth - 8) * 6);
  const uint_32 log2_tr_size = kvz_g_convert_to_bit[width] + 2;
  const int_32 quant_coeff = kvz_q_quant_scales[qp_scaled % 6];

  const int_32 transform_shift = MAX_TR_DYNAMIC_RANGE - bitdepth - log2_tr_size;
  const int_32 q_bits = QUANT_SHIFT + qp_scaled / 6 + transform_shift;
  const int_32 add = 171 << (q_bits - 9);

  for (int_32 n = 0; n < width * height; n++) {
    int_32 level;
    int_32 sign;

    level = coef[n];
    sign = (level < 0 ? -1 : 1);

    level = ((int_64)std::abs((int)level) * quant_coeff + add) >> q_bits;

    level *= sign;
    q_coef[n] = (coeff_t)(CLIP((int_32)(-32768), (int_32)32767, level));
  }
}

void kvz_dequant_generic(coeff_t *q_coef, coeff_t *coef, int_8 qp, int_32 width, int_32 height, int_8 type) {
  const int_8 bitdepth = 8;

  int_32 shift, add, coeff_q, n;
  int_32 transform_shift = 15 - bitdepth - (kvz_g_convert_to_bit[width] + 2);

  int_32 qp_scaled = kvz_get_scaled_qp(type, qp, (bitdepth - 8) * 6);

  shift = 20 - QUANT_SHIFT - transform_shift;

  int_32 scale = (int_32)kvz_g_inv_quant_scales[qp_scaled % 6] << (qp_scaled / 6);
  add = 1 << (shift - 1);

  for (n = 0; n < width * height; n++) {
    coeff_q = (q_coef[n] * scale + add) >> shift;
    coef[n] = (coeff_t)CLIP((int_32)(-32768), (int_32)32767, coeff_q);
  }
}

class Input {
public:
  Input() { reset(); }

  void operator<<(const int_16 &rhs) {
    src[w_id].set_slc(slc * 16, rhs);

    slc++;
    if (!slc)
      w_id++;
  }

  ac_int< 16 * 32, false > get() { return src[r_id++]; }

  bool empty() { return r_id == w_id; }
  void reset() { slc = r_id = w_id = 0; }

private:
  ac_int< 5, false > slc;
  ac_int< 6, false > r_id;
  ac_int< 6, false > w_id;
  ac_int< 16 * 32, false > src[64];
};

class Output {
public:
  Output() { reset(); }

  void operator<<(const ac_int< 16 * 32, false > &rhs) { src[w_id++] = rhs; }

  int_16 get() {
    int_16 s = src[r_id].slc< 16 >(16 * slc++);
    if (!slc)
      r_id++;

    return s;
  }

  bool empty() { return r_id == w_id; }
  void reset() { slc = r_id = w_id = 0; }

private:
  ac_int< 5, false > slc;
  ac_int< 6, false > r_id;
  ac_int< 6, false > w_id;
  ac_int< 16 * 32, false > src[64];
};

static const unsigned bitdepth = 8;

void main_quant(coeff_port_t &coeff_in, coeff_port_t &coeff_out, coeff_port_t &quant_coeff_out);

CCS_MAIN(int argc, char *argv[]) {
  static coeff_port_t coeff_in;
  static coeff_port_t coeff_out;
  static coeff_port_t quant_coeff_out;

  int_16 src[32 * 32];
  int_16 ref_quant[32 * 32];
  int_16 ref_dequant[32 * 32];
  one_bit simulation_successful = 1;

  Input slice_i;
  Output slice_o;

  one_bit print = 0;

  for (int i = 0; i < 32; ++i) {
    srand(i);

    // Random Input
    for (uint_6 y = 0; y < 32; y++) {
      for (uint_6 x = 0; x < 32; x++) {
        src[y * 32 + x] = int_16((rand() % 65535) - 32768);
      }
    }

    uint_4 depth = 0;
    for (depth = 0; depth < 4; ++depth) {
      uint_4 color = 0;
      for (color = 0; color < 3; ++color) {

        uint_6 QP = rand() % 64; // 10bit: QP Max = 51
        uint_6 qp_scaled = kvz_get_scaled_qp(color, QP, (bitdepth - 8) * 6);

        conf_t duv_conf;
        duv_conf.depth = depth;
        duv_conf.color = color;
        duv_conf.qp_scaled = qp_scaled;

        two_bit size = duv_conf.size();
        uint_6 width = duv_conf.width();
        cout << "Depth: " << depth << " Color: " << color << " Width: " << width << " QP: " << QP << endl;

        // Add zeros for 4x4 blocks
        if (size == 3 && i % 2 == 1) {
          for (uint_6 x = 0; x < 16; x++)
            slice_i << 0;
        }

        // Input
        for (uint_6 y = 0; y < width; y++) {
          for (uint_6 x = 0; x < width; x++) {
            slice_i << src[y * width + x];
          }
        }

        // Add zeros for 4x4 blocks
        if (size == 3 && i % 2 == 0) {
          for (uint_6 x = 0; x < 16; x++)
            slice_i << 0;
        }

        kvz_quant_generic(src, ref_quant, QP, width, width, (!color ? 0 : 2));
        kvz_dequant_generic(ref_quant, ref_dequant, QP, width, width, (!color ? 0 : 2));

        if (print) {
          cout << "Ref Output Quant: " << endl;
          for (uint_6 y = 0; y < width; y++) {
            for (uint_6 x = 0; x < width; x++) {
              std::cout << ref_quant[y * width + x] << ", ";
            }
            std::cout << std::endl;
          }

          cout << "Ref Output DeQuant: " << endl;
          for (uint_6 y = 0; y < width; y++) {
            for (uint_6 x = 0; x < width; x++) {
              std::cout << ref_dequant[y * width + x] << ", ";
            }
            std::cout << std::endl;
          }
        }

        ac_int< 72 > conf = 0;
        conf.set_slc(4, two_bit(depth));
        conf.set_slc(6, two_bit(color));
        conf.set_slc(24, qp_scaled);
        coeff_in.write(conf);

        while (!slice_i.empty()) {
          coeff_in.write(slice_i.get());
        }

        CCS_DESIGN(main_quant)
        (coeff_in, coeff_out, quant_coeff_out);

        // QUANT
        quant_coeff_out.read();
        while (quant_coeff_out.available(1)) {
          slice_o << quant_coeff_out.read();
        }

        if (size == 3 && i % 2 == 1) {
          for (uint_6 x = 0; x < 16; x++)
            slice_o.get();
        }

        for (uint_6 y = 0; y < width; ++y) {
          for (uint_6 x = 0; x < width; ++x) {
            int_16 coeff = slice_o.get();

            if (coeff != ref_quant[y * width + x])
              simulation_successful = 0;
          }
        }

        if (size == 3 && i % 2 == 0) {
          for (uint_6 x = 0; x < 16; x++)
            slice_o.get();
        }

        if (!slice_o.empty()) {
          simulation_successful = 0;
          std::cout << "ERROR Output channel not empty" << std::endl;
        }

        if (!simulation_successful) {
          std::cout << "ERROR QUANT" << std::endl;
          break;
        }

        // DEQUANT
        coeff_out.read();
        while (coeff_out.available(1)) {
          slice_o << coeff_out.read();
        }

        if (size == 3 && i % 2 == 1) {
          for (uint_6 x = 0; x < 16; x++)
            slice_o.get();
        }

        for (uint_6 y = 0; y < width; ++y) {
          for (uint_6 x = 0; x < width; ++x) {
            int_16 coeff = slice_o.get();

            if (coeff != ref_dequant[y * width + x])
              simulation_successful = 0;
          }
        }

        if (size == 3 && i % 2 == 0) {
          for (uint_6 x = 0; x < 16; x++)
            slice_o.get();
        }

        if (!slice_o.empty()) {
          simulation_successful = 0;
          std::cout << "ERROR Output channel not empty" << std::endl;
        }

        if (!simulation_successful) {
          std::cout << "ERROR DEQUANT" << std::endl;
          break;
        }
      }
      if (!simulation_successful)
        break;
    }
    if (!simulation_successful)
      break;
  }

  if (simulation_successful == 1) {
    cout << "## SIMULATION PASSED ##" << endl;
  } else {
    cout << "## SIMULATION FAILED ##" << endl;
  }

  CCS_RETURN(0);
}
