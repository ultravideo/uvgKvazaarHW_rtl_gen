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

#define SHIFT ac::log2_ceil< OP_WIDTH >::val

static const ac_int< 19, false > dist_thres[4] = {
    523261, // 0b11111111011111111 01
    520701, // 0b11111110001111111 01
    262149, // 0b10000000000000001 01
    0,      // 0b00000000000000000 00
};
static const uint_6 abs_angle[19] = {0, 0, 32, 26, 21, 17, 13, 9, 5, 2, 0, 2, 5, 9, 13, 17, 21, 26, 32};
static const uint_4 distances[19] = {0, 0, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8};

class CFilter {
public:
  CFilter(pixel_t zero, pixel_t pos, pixel_t neg) : next(pos), cur(zero), prev(neg) {}

  void operator<<(const pixel_t &rhs) {
    prev = cur;
    cur = next;
    next = rhs;
  }

  pixel_t get_filt() { return (prev + (2 * cur) + next + 2) >> 2; }
  pixel_t get_ufilt() { return cur; }

private:
  pixel_t cur, prev, next;
};

#pragma hls_design top
void main_ip_ctrl(input_port_t &conf_in, ac_channel< int_64 > &sad_config, ip_planar_port_t &planar_out, ip_dc_port_t &dc_out, ip_pos_port_t &ang_pos_out2, ip_pos_port_t &ang_pos_out3,
                  ip_pos_port_t &ang_pos_out4, ip_pos_port_t &ang_pos_out5, ip_pos_port_t &ang_pos_out6, ip_pos_port_t &ang_pos_out7, ip_pos_port_t &ang_pos_out8, ip_pos_port_t &ang_pos_out9,
                  ip_zero_port_t &ang_zero_out, ip_neg_port_t &ang_neg_out11, ip_neg_port_t &ang_neg_out12, ip_neg_port_t &ang_neg_out13, ip_neg_port_t &ang_neg_out14, ip_neg_port_t &ang_neg_out15,
                  ip_neg_port_t &ang_neg_out16, ip_neg_port_t &ang_neg_out17, ip_neg_port_t &ang_neg_out18,

                  output_t ref0[4][64 / OP_WIDTH], output_t ref1[4][64 / OP_WIDTH], output_t ref2[4][128 / OP_WIDTH], output_t ref3[4][128 / OP_WIDTH], output_t ref4[4][128 / OP_WIDTH],
                  output_t ref5[4][128 / OP_WIDTH], output_t ref6[4][128 / OP_WIDTH], output_t ref7[4][128 / OP_WIDTH], output_t ref8[4][128 / OP_WIDTH], output_t ref9[4][128 / OP_WIDTH],
                  output_t ref10[4][64 / OP_WIDTH], output_t ref11[4][64 / OP_WIDTH], output_t ref12[4][64 / OP_WIDTH], output_t ref13[4][64 / OP_WIDTH], output_t ref14[4][64 / OP_WIDTH],
                  output_t ref15[4][64 / OP_WIDTH], output_t ref16[4][64 / OP_WIDTH], output_t ref17[4][64 / OP_WIDTH], output_t ref18[4][64 / OP_WIDTH]) {
#ifndef __SYNTHESIS__
  static int ip_id = 0;
#else
  static two_bit ip_id = 0;
#endif

  output_t *const refs[19] = {ref0[ip_id],
                              ref1[ip_id],
                              ref2[ip_id],
                              ref3[ip_id],
                              ref4[ip_id],
                              ref5[ip_id],
                              ref6[ip_id],
                              ref7[ip_id],
                              ref8[ip_id],
                              ref9[ip_id],
                              ref10[ip_id],
                              ref11[ip_id],
                              ref12[ip_id],
                              ref13[ip_id],
                              ref14[ip_id],
                              ref15[ip_id],
                              ref16[ip_id],
                              ref17[ip_id],
                              ref18[ip_id]};

  // Read config
  const ac_int< 88, false > config = conf_in.read();
  const conf_t conf = to_struct(config);

  const two_bit size = (conf.color && conf.depth != 3) ? two_bit(conf.depth + 1) : conf.depth;
  const uint_6 block_size = (LCU_WIDTH / 2) >> size;

  const ac_int< 19, false > filt_thres = dist_thres[conf.color ? (two_bit)3 : conf.depth];

  pixel_t last_ver;
  pixel_t last_hor;
  uint_16 ref_sum = block_size;

  CFilter zfilter(config.slc< 8 >(64), config.slc< 8 >(72), config.slc< 8 >(80));

  pixel_t zero_filt = zfilter.get_filt();
  pixel_t zero_ufilt = zfilter.get_ufilt();

  uint_6 last_pixel_pos = MIN(block_size, (uint_6)OP_WIDTH);                // Last pixel within block
  uint_6 final_pixel_pos = MIN((uint_7)(block_size * 2), (uint_7)OP_WIDTH); // Final pixel to send

#pragma hls_pipeline_init_interval 1
  for (two_bit r = 0; r < 2; ++r) {
    uint_8 offset = (r == 0) ? 0 : 32;
    pixel_t first_pixel = (r == 0) ? config.slc< 8 >(72) : config.slc< 8 >(80);

    one_bit half_sent = 0;
    one_bit last_pixel_set = 0;

    CFilter filter(zero_ufilt, first_pixel, 0);

    // Sad ip_conf & intra preds
    if (r == 0)
      sad_config.write(config);

#pragma hls_pipeline_init_interval 1
    for (uint_8 y = 0; y < 64; y += OP_WIDTH) {
      output_t filt, unfilt;

      // Read reference pixels
      input_t ref = conf_in.read();

    filer:
#pragma hls_unroll yes
      for (uint_8 j = 0; j < OP_WIDTH; ++j) {
        // Filter reference pixels
        filter << ref.slc< 8 >(j * 8);

        filt.set_slc(8 * j, filter.get_filt());
        unfilt.set_slc(8 * j, filter.get_ufilt());
      }

      if (y + OP_WIDTH >= 2 * block_size) {
        filt.set_slc(8 * (final_pixel_pos - 1), unfilt.slc< 8 >(8 * (final_pixel_pos - 1)));
      }

    pos_modes:
#pragma hls_unroll yes
      for (uint_6 i = 2; i < 10; ++i) {
        refs[i][(y + 2 * offset) >> SHIFT] = filt_thres[i] ? filt : unfilt;
      }

      if (!half_sent) {
        ref0[ip_id][(y + offset) >> SHIFT] = filt_thres[0] ? filt : unfilt;
        ref1[ip_id][(y + offset) >> SHIFT] = unfilt;
        ref10[ip_id][(y + offset) >> SHIFT] = unfilt;

      neg_modes:
#pragma hls_unroll yes
        for (uint_6 i = 11; i < 19; ++i) {
          refs[i][(y + offset) >> SHIFT] = filt_thres[i] ? filt : unfilt;
        }
      }

      // Sum of unfilered references for DC
      if (!half_sent) {
        uint_10 ref_sum_slice[4] = {0, 0, 0, 0};
      ref_sum:
#pragma hls_unroll yes
        for (uint_8 j = 0; j < OP_WIDTH; ++j) {
          ref_sum_slice[j / 4] += (pixel_t)unfilt.slc< 8 >(8 * j);
        }

        switch (size) {
        case 3:
          ref_sum += ref_sum_slice[0];
          break;
        case 2:
          ref_sum += ref_sum_slice[0] + ref_sum_slice[1];
          break;
        default:
          ref_sum += ref_sum_slice[0] + ref_sum_slice[1] + ref_sum_slice[2] + ref_sum_slice[3];
          break;
        }
      }

      // Left bottom and top left pixels for planar
      if (!last_pixel_set) {
        if (last_pixel_pos == OP_WIDTH && half_sent) {
          last_pixel_set = 1;

          if (r == 0) {
            last_ver = (filt_thres[0] ? filt : unfilt).slc< 8 >(0);
          } else {
            last_hor = (filt_thres[0] ? filt : unfilt).slc< 8 >(0);
          }
        } else if (last_pixel_pos != OP_WIDTH && !half_sent) {
          last_pixel_set = 1;

          if (r == 0) {
            last_ver = (filt_thres[0] ? filt : unfilt).slc< 8 >(8 * last_pixel_pos);
          } else {
            last_hor = (filt_thres[0] ? filt : unfilt).slc< 8 >(8 * last_pixel_pos);
          }
        }
      }

      if (y + OP_WIDTH == block_size)
        half_sent = 1;
      if (y + OP_WIDTH == 2 * block_size)
        break;
    }
  }

  ip_conf_t ip_conf;
  ip_conf.ip_id = ip_id;
  ip_conf.size = size;

#ifndef __SYNTHESIS__
  if (ip_id == 3) {
    ip_id = 0;
  } else {
    ip_id += 1;
  }
#else
  // Move to next slot
  ip_id += 1;
#endif

  planar_out.write(planar_to_ac_int(ip_conf, last_ver, last_hor));
  dc_out.write(dc_to_ac_int(ip_conf, ref_sum >> (6 - size), conf.color == 0 && conf.depth != 0));
  ang_pos_out2.write(pos_to_ac_int(ip_conf, abs_angle[2]));
  ang_pos_out3.write(pos_to_ac_int(ip_conf, abs_angle[3]));
  ang_pos_out4.write(pos_to_ac_int(ip_conf, abs_angle[4]));
  ang_pos_out5.write(pos_to_ac_int(ip_conf, abs_angle[5]));
  ang_pos_out6.write(pos_to_ac_int(ip_conf, abs_angle[6]));
  ang_pos_out7.write(pos_to_ac_int(ip_conf, abs_angle[7]));
  ang_pos_out8.write(pos_to_ac_int(ip_conf, abs_angle[8]));
  ang_pos_out9.write(pos_to_ac_int(ip_conf, abs_angle[9]));
  ang_zero_out.write(zero_to_ac_int(ip_conf, zero_ufilt, conf.color == 0 && conf.depth != 0));
  ang_neg_out11.write(neg_to_ac_int(ip_conf, distances[11], abs_angle[11], filt_thres[11] ? zero_filt : zero_ufilt));
  ang_neg_out12.write(neg_to_ac_int(ip_conf, distances[12], abs_angle[12], filt_thres[12] ? zero_filt : zero_ufilt));
  ang_neg_out13.write(neg_to_ac_int(ip_conf, distances[13], abs_angle[13], filt_thres[13] ? zero_filt : zero_ufilt));
  ang_neg_out14.write(neg_to_ac_int(ip_conf, distances[14], abs_angle[14], filt_thres[14] ? zero_filt : zero_ufilt));
  ang_neg_out15.write(neg_to_ac_int(ip_conf, distances[15], abs_angle[15], filt_thres[15] ? zero_filt : zero_ufilt));
  ang_neg_out16.write(neg_to_ac_int(ip_conf, distances[16], abs_angle[16], filt_thres[16] ? zero_filt : zero_ufilt));
  ang_neg_out17.write(neg_to_ac_int(ip_conf, distances[17], abs_angle[17], filt_thres[17] ? zero_filt : zero_ufilt));
  ang_neg_out18.write(neg_to_ac_int(ip_conf, distances[18], abs_angle[18], filt_thres[18] ? zero_filt : zero_ufilt));
}
