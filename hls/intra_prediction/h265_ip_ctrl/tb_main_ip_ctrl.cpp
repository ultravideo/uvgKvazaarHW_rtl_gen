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
#include <stdio.h>
#include <string.h>
#include <time.h>
using namespace std;

#define SWAP(a, b, swaptype)                                                                                                                                                                           \
  {                                                                                                                                                                                                    \
    swaptype tempval;                                                                                                                                                                                  \
    tempval = a;                                                                                                                                                                                       \
    a = b;                                                                                                                                                                                             \
    b = tempval;                                                                                                                                                                                       \
  }

typedef uint_8 kvz_pixel;
static kvz_pixel golden_src1[66] = {245, 204, 118, 71,  49,  188, 241, 245, 214, 124, 87, 125, 220, 11,  53, 74,  55,  81,  68,  35,  162, 255, 90,  150, 160, 81,  200, 94,  226, 236, 203, 184, 202,
                                    114, 47,  125, 214, 183, 83,  182, 145, 186, 125, 48, 13,  174, 192, 99, 254, 103, 221, 102, 102, 94,  22,  126, 4,   121, 120, 226, 228, 99,  230, 14,  18,  0};

static kvz_pixel golden_src2[66] = {245, 19, 211, 238, 45,  202, 217, 123, 8,  231, 249, 215, 57, 15, 40,  88, 236, 121, 37,  184, 68, 2,   73, 122, 161, 135, 33, 247, 85,  132, 111, 107, 32,
                                    56,  79, 23,  208, 190, 245, 183, 225, 38, 139, 146, 214, 45, 18, 235, 81, 254, 30,  237, 157, 93, 135, 8,  129, 190, 198, 67, 156, 250, 226, 79,  150, 0};

const int_8 kvz_g_convert_to_bit[LCU_WIDTH + 1] = {-1, -1, -1, -1, 0,  -1, -1, -1, 1,  -1, -1, -1, -1, -1, -1, -1, 2,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3,
                                                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4};

typedef struct {
  kvz_pixel left[65];
  kvz_pixel top[65];
} kvz_intra_ref;
typedef struct {
  kvz_intra_ref ref;
  kvz_intra_ref filtered_ref;
  bool filtered_initialized;
} kvz_intra_references;

static void intra_filter_reference(int_8 log2_width, kvz_intra_references *refs) {
  if (refs->filtered_initialized) {
    return;
  } else {
    refs->filtered_initialized = true;
  }

  const int_8 ref_width = 2 * (1 << log2_width) + 1;
  kvz_intra_ref *ref = &refs->ref;
  kvz_intra_ref *filtered_ref = &refs->filtered_ref;

  filtered_ref->left[0] = (ref->left[1] + 2 * ref->left[0] + ref->top[1] + 2) / 4;
  filtered_ref->top[0] = filtered_ref->left[0];

  for (int_8 y = 1; y < ref_width - 1; ++y) {
    kvz_pixel *p = &ref->left[y];
    filtered_ref->left[y] = (p[-1] + 2 * p[0] + p[1] + 2) / 4;
  }
  filtered_ref->left[ref_width - 1] = ref->left[ref_width - 1];

  for (int_8 x = 1; x < ref_width - 1; ++x) {
    kvz_pixel *p = &ref->top[x];
    filtered_ref->top[x] = (p[-1] + 2 * p[0] + p[1] + 2) / 4;
  }
  filtered_ref->top[ref_width - 1] = ref->top[ref_width - 1];
}

const kvz_intra_ref *kvz_intra_predict(kvz_intra_references *refs, int_8 log2_width, int_8 mode, two_bit color) {
  const int_8 width = 1 << log2_width;

  const kvz_intra_ref *used_ref = &refs->ref;
  if (color || mode == 1 || width == 4) {
    // For chroma, DC and 4x4 blocks, always use unfiltered reference.
  } else if (mode == 0) {
    // Otherwise, use filtered for planar.
    used_ref = &refs->filtered_ref;
  } else {
    // Angular modes use smoothed reference pixels, unless the mode is close
    // to being either vertical or horizontal.
    static const int kvz_intra_hor_ver_dist_thres[5] = {0, 7, 1, 0, 0};
    int filter_threshold = kvz_intra_hor_ver_dist_thres[kvz_g_convert_to_bit[width]];

    int dist_from_vert_or_hor = MIN(std::abs(mode - 26), std::abs(mode - 10));
    if (dist_from_vert_or_hor > filter_threshold) {
      used_ref = &refs->filtered_ref;
    }
  }

  if (used_ref == &refs->filtered_ref && !refs->filtered_initialized) {
    intra_filter_reference(log2_width, refs);
  }

  return used_ref;
}

uint_8 get_bytes(uint_6 mode, uint_8 width) {
  if (mode >= 2 && mode < 10)
    return 2 * width;

  if (mode == 0)
    return width;

  return width;
}

void main_ip_ctrl(input_port_t &conf_in, ac_channel< int_64 > &sad_config, ip_planar_port_t &planar_out, ip_dc_port_t &dc_out, ip_pos_port_t &ang_pos_out2, ip_pos_port_t &ang_pos_out3,
                  ip_pos_port_t &ang_pos_out4, ip_pos_port_t &ang_pos_out5, ip_pos_port_t &ang_pos_out6, ip_pos_port_t &ang_pos_out7, ip_pos_port_t &ang_pos_out8, ip_pos_port_t &ang_pos_out9,
                  ip_zero_port_t &ang_zero_out, ip_neg_port_t &ang_neg_out11, ip_neg_port_t &ang_neg_out12, ip_neg_port_t &ang_neg_out13, ip_neg_port_t &ang_neg_out14, ip_neg_port_t &ang_neg_out15,
                  ip_neg_port_t &ang_neg_out16, ip_neg_port_t &ang_neg_out17, ip_neg_port_t &ang_neg_out18,

                  output_t ref0[4][64 / OP_WIDTH], output_t ref1[4][64 / OP_WIDTH], output_t ref2[4][128 / OP_WIDTH], output_t ref3[4][128 / OP_WIDTH], output_t ref4[4][128 / OP_WIDTH],
                  output_t ref5[4][128 / OP_WIDTH], output_t ref6[4][128 / OP_WIDTH], output_t ref7[4][128 / OP_WIDTH], output_t ref8[4][128 / OP_WIDTH], output_t ref9[4][128 / OP_WIDTH],
                  output_t ref10[4][64 / OP_WIDTH], output_t ref11[4][64 / OP_WIDTH], output_t ref12[4][64 / OP_WIDTH], output_t ref13[4][64 / OP_WIDTH], output_t ref14[4][64 / OP_WIDTH],
                  output_t ref15[4][64 / OP_WIDTH], output_t ref16[4][64 / OP_WIDTH], output_t ref17[4][64 / OP_WIDTH], output_t ref18[4][64 / OP_WIDTH]);

CCS_MAIN(int argc, char *argv[]) {
  srand(time(NULL));

  static ip_planar_port_t in0;
  static ip_dc_port_t in1;
  static ip_pos_port_t in2;
  static ip_pos_port_t in3;
  static ip_pos_port_t in4;
  static ip_pos_port_t in5;
  static ip_pos_port_t in6;
  static ip_pos_port_t in7;
  static ip_pos_port_t in8;
  static ip_pos_port_t in9;
  static ip_zero_port_t in10;
  static ip_neg_port_t in11;
  static ip_neg_port_t in12;
  static ip_neg_port_t in13;
  static ip_neg_port_t in14;
  static ip_neg_port_t in15;
  static ip_neg_port_t in16;
  static ip_neg_port_t in17;
  static ip_neg_port_t in18;

  static output_t ref0[4][64 / OP_WIDTH];
  static output_t ref1[4][64 / OP_WIDTH];
  static output_t ref2[4][128 / OP_WIDTH];
  static output_t ref3[4][128 / OP_WIDTH];
  static output_t ref4[4][128 / OP_WIDTH];
  static output_t ref5[4][128 / OP_WIDTH];
  static output_t ref6[4][128 / OP_WIDTH];
  static output_t ref7[4][128 / OP_WIDTH];
  static output_t ref8[4][128 / OP_WIDTH];
  static output_t ref9[4][128 / OP_WIDTH];
  static output_t ref10[4][64 / OP_WIDTH];
  static output_t ref11[4][64 / OP_WIDTH];
  static output_t ref12[4][64 / OP_WIDTH];
  static output_t ref13[4][64 / OP_WIDTH];
  static output_t ref14[4][64 / OP_WIDTH];
  static output_t ref15[4][64 / OP_WIDTH];
  static output_t ref16[4][64 / OP_WIDTH];
  static output_t ref17[4][64 / OP_WIDTH];
  static output_t ref18[4][64 / OP_WIDTH];

  static input_port_t config;
  static ac_channel< int_64 > sad_config;

  int ip_id = 0;

  one_bit print = 0;

  one_bit error = 0;
  one_bit simulation_succesful = 1;

  for (uint_8 rnd = 0; rnd < 32; rnd++) {
    for (uint_4 depth = 0; depth < 4; ++depth) {
      for (uint_4 color = 0; color < 3; ++color) {
        const two_bit size = (color && depth != 3) ? two_bit(depth + 1) : (two_bit)depth;
        const uint_6 width = (LCU_WIDTH / 2) >> size;

        std::cout << "Width " << width << " Color: " << color << std::endl;

        kvz_intra_references refs;
        refs.filtered_initialized = false;
        refs.ref.top[0] = refs.ref.left[0] = rand() % 256;

        for (uint_8 i = 1; i < 65; ++i) {
          refs.ref.top[i] = rand() % 256;
          refs.ref.left[i] = rand() % 256;
        }

        if (print) {
          kvz_intra_predict(&refs, kvz_g_convert_to_bit[width] + 2, 0, color);

          std::cout << "Unfilt:\n Left:\n";
          for (uint_8 i = 0; i < 65; ++i)
            std::cout << refs.ref.left[i] << " ";
          std::cout << "\n Top:\n";
          for (uint_8 i = 0; i < 65; ++i)
            std::cout << refs.ref.top[i] << " ";
          std::cout << std::endl;

          std::cout << "Filt:\n Left:\n";
          for (uint_8 i = 0; i < 65; ++i)
            std::cout << refs.filtered_ref.left[i] << " ";
          std::cout << "\n Top:\n";
          for (uint_8 i = 0; i < 65; ++i)
            std::cout << refs.filtered_ref.top[i] << " ";
          std::cout << "\n" << std::endl;
        }

        ac_int< 88, false > conf;
        conf.set_slc(4, depth);
        conf.set_slc(6, color);
        conf.set_slc(64, refs.ref.left[0]);
        conf.set_slc(72, refs.ref.left[1]);
        conf.set_slc(80, refs.ref.top[1]);

        config.write(conf);

        for (int i = 0; i < 64; i += OP_WIDTH) {
          input_t refs_out = 0;

          for (int j = 0; j < OP_WIDTH; j++)
            refs_out.set_slc(8 * j, refs.ref.left[i + j + 2]);

          config.write(refs_out);
          if (i + OP_WIDTH >= width * 2)
            break;
        }

        for (int i = 0; i < 64; i += OP_WIDTH) {
          input_t refs_out = 0;

          for (int j = 0; j < OP_WIDTH; j++)
            refs_out.set_slc(8 * j, refs.ref.top[i + j + 2]);

          config.write(refs_out);
          if (i + OP_WIDTH >= width * 2)
            break;
        }

        output_t *out_ref[19] = {ref0[ip_id],
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

        CCS_DESIGN(main_ip_ctrl)
        (config,
         sad_config,
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
         ref0,
         ref1,
         ref2,
         ref3,
         ref4,
         ref5,
         ref6,
         ref7,
         ref8,
         ref9,
         ref10,
         ref11,
         ref12,
         ref13,
         ref14,
         ref15,
         ref16,
         ref17,
         ref18);

        if (ip_id == 3) {
          ip_id = 0;
        } else {
          ip_id += 1;
        }

        for (int mode = 0; mode < 19; ++mode, error = 0) {
          const kvz_intra_ref *ref = kvz_intra_predict(&refs, kvz_g_convert_to_bit[width] + 2, mode, color);
          const uint_8 ref_bytes = get_bytes(mode, width);

          if (print) {
            std::cout << "\nWidth " << width << " Mode " << mode << " Color " << color << " Bytes " << ref_bytes << std::endl;

            for (int y = 0; y < 128 / OP_WIDTH; ++y) {
              output_t out = out_ref[mode][y];
              for (int x = 0; x < OP_WIDTH; ++x) {
                std::cout << out.slc< 8 >(8 * x) << " ";
              }
              std::cout << std::endl;
            }
            std::cout << std::endl << "DUV:\n";
          }

          for (two_bit r = 0; r < 2; ++r) {
            int offset = (r == 0) ? 0 : 32 / OP_WIDTH;

            if (mode >= 2 && mode < 10)
              offset *= 2;
            int max_temp = ref_bytes / OP_WIDTH;
            if (max_temp < 1) {
              max_temp = 1;
            }
            for (int y = 0; y < max_temp; ++y) {
              output_t out = out_ref[mode][y + offset];

              for (int x = 0; x < OP_WIDTH; ++x) {
                if (r == 0) { // Left
                  if (out.slc< 8 >(8 * x) != ref->left[y * OP_WIDTH + x + 1])
                    error = 1;
                } else {
                  if (out.slc< 8 >(8 * x) != ref->top[y * OP_WIDTH + x + 1])
                    error = 1;
                }

                if (print)
                  std::cout << out.slc< 8 >(8 * x) << " ";
              }

              if (print) {
                std::cout << std::endl;
                if (r == 0) { // Left
                  for (int x = 0; x < OP_WIDTH; ++x)
                    std::cout << ref->left[y * OP_WIDTH + x + 1] << " ";
                } else {
                  for (int x = 0; x < OP_WIDTH; ++x)
                    std::cout << ref->top[y * OP_WIDTH + x + 1] << " ";
                }
                std::cout << std::endl;
              }
            }

            if (error == 1) {
              cout << "ERROR: Width " << width << " Mode " << mode << endl;

              simulation_succesful = 0;
              break;
            }
          }

          if (mode == 0) {
            output_t planar = in0.read();
            pixel_t last_ver = planar.slc< 8 >(4);
            pixel_t last_hor = planar.slc< 8 >(12);

            if (last_ver != ref->left[width + 1] || last_hor != ref->top[width + 1]) {
              cout << "ERROR: Planar last pixel fail" << endl;
              cout << "Ref: " << ref->left[width + 1] << " " << ref->top[width + 1];
              cout << "\nDuv: " << last_ver << " " << last_hor << endl;

              simulation_succesful = 0;
            }
          }

          if (mode == 1) {
            output_t dc = in1.read();
            pixel_t dc_val = dc.slc< 8 >(4);

            int_16 ref_sum = width;
            for (int x = 0; x < width; ++x) {
              ref_sum += ref->left[x + 1] + ref->top[x + 1];
            }

            pixel_t ref_dc_val = ref_sum >> (6 - size);

            if (dc_val != ref_dc_val) {
              cout << "ERROR: DC Val fail" << endl;
              cout << "Ref: " << ref_dc_val;
              cout << "\nDuv: " << dc_val << endl;

              simulation_succesful = 0;
            }
          }

          if (simulation_succesful == 0)
            break;
        }

        if (simulation_succesful == 0)
          break;
      }
      if (simulation_succesful == 0)
        break;
    }
    if (simulation_succesful == 0)
      break;
  }

  if (simulation_succesful == 1) {
    cout << "### SIMULATION PASSED ###" << endl;
  } else {
    cout << "### SIMULATION FAILED ###" << endl;
  }
  CCS_RETURN(0);
}
