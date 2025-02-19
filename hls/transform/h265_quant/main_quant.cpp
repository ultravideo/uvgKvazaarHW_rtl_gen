#include "global.h"

#define QUANT_SHIFT 14

// { 40, 45, 51, 57, 64, 72 }[qp_scaled % 6] << (qp_scaled / 6)
/*static const uint_16 kvz_g_inv_quant_scales[64] = {
    40, 45, 51, 57, 64, 72, 80, 90, 102, 114, 128, 144, 160, 180, 204, 228,
    256, 288, 320, 360, 408, 456, 512, 576, 640, 720, 816, 912, 1024, 1152, 1280, 1440,
    1632, 1824, 2048, 2304, 2560, 2880, 3264, 3648, 4096, 4608, 5120, 5760, 6528, 7296, 8192, 9216,
    10240, 11520, 13056, 14592, 16384, 18432, 20480, 23040, 26112, 29184, 32768, 36864, 40960, 46080, 52224, 58368,
};*/

static const uint_16 kvz_g_inv_quant_scales[6] = {40, 45, 51, 57, 64, 72};

static const uint_16 kvz_q_quant_scales[6] = {26214, 23302, 20560, 18396, 16384, 14564};

template < int N > ac_int< N, false > abs(ac_int< N > din) {
  ac_int< N > tmp0 = 0, tmp1 = 0;
  tmp0 = din;
Abs:
#pragma hls_unroll yes
  for (int i = 0; i < N; i++)
    tmp1[i] = tmp0[i] ^ tmp0[N - 1];

  return tmp1 + tmp0[N - 1];
}

template < int w_i, int w_o > ac_int< w_o, true > clip_signed(ac_int< w_i, true > a) {
  static const ac_int< w_o, true > int_min = 1 << (w_o - 1);
  static const ac_int< w_o, true > int_max = ac_int< w_o, false >(-1) >> 1;

  ac_int< w_o, true > r = a;

  one_bit MSB = a[w_i - 1];
  ac_int< w_i - w_o - 1, true > top_bits = a >> (w_o - 1);

  if (MSB) {
    // Signed
    if (top_bits != -1)
      r = int_min;
  } else {
    // Unsigned
    if (top_bits)
      r = int_max;
  }

  return r;
}

int_16 quant(int_16 coeff, uint_16 scale, uint_8 add, uint_4 shift) {
  int_19 level = ((abs< 16 >(coeff) * scale + ((uint_28(add) << 5) << shift)) >> 14) >> shift;
  level = coeff < 0 ? (int_19)(-level) : level;
  return clip_signed< 19, 16 >(level);
}

int_16 dequant(int_16 coeff, uint_16 scale, uint_8 add, uint_3 shift) {
  int_32 coeff_q = (coeff * scale + add) >> shift;
  return clip_signed< 32, 16 >(coeff_q);
}

#pragma hls_design top
void main_quant(coeff_port_t &coeff_in, coeff_port_t &coeff_out, coeff_port_t &quant_out) {
  static const uint_5 loops[2][4] = {{31, 7, 1, 0}, {15, 3, 0, 0}};
  static const uint_4 bitdepth = 8;

  // Read and write config
  const conf_t conf(coeff_in.read());
  quant_out.write(conf.toInt());
  coeff_out.write(conf.toInt());

  uint_4 q_shift = 10 - bitdepth + (conf.qp_scaled / 6) + conf.size();
  uint_3 d_shift = 10 - QUANT_SHIFT + bitdepth - conf.size();

  uint_8 q_add = 171;
  uint_8 d_add = 1 << (d_shift - 1);

  uint_16 q_scale = kvz_q_quant_scales[conf.qp_scaled % 6];
  uint_16 d_scale = kvz_g_inv_quant_scales[conf.qp_scaled % 6] << (conf.qp_scaled / 6);

#pragma hls_pipeline_init_interval 1
  for (uint_6 y = 0; y < MAX_WIDTH; ++y) {
    coeff_slice_t quant_input = coeff_in.read();
    coeff_slice_t quant_output, coeff_output;
  Quant:
#pragma hls_unroll yes
    for (uint_6 x = 0; x < MAX_WIDTH; ++x) {
      quant_output.set_slc(16 * x, quant(quant_input.slc< 16 >(16 * x), q_scale, q_add, q_shift));
    }
  Dequant:
#pragma hls_unroll yes
    for (uint_6 x = 0; x < MAX_WIDTH; ++x) {
      coeff_output.set_slc(16 * x, dequant(quant_output.slc< 16 >(16 * x), d_scale, d_add, d_shift));
    }

    quant_out.write(quant_output);
    coeff_out.write(coeff_output);
    if (y == loops[conf.color == 3][conf.size()])
      break;
  }
}
