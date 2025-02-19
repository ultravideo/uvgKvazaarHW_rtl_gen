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
#include <string.h>

void pred_buffer_pull(conf_port_t &conf_in, ac_channel< ac_int< 8, false > > &intra_mode_in, pixel_port_t &pred_in, pixel_port_t &ref_in, resid_port_t &resid_out, pixel_port_t &pred_out,
                      pixel_port_t &ref_out);

void pred_buffer_push(input_port_t &pred_in, ac_channel< ac_int< 8, false > > &intra_mode_in, pixel_port_t &pred_out, pixel_port_t &ref_out);

void pred_buffer(conf_port_t &conf_in, ac_channel< ac_int< 8, false > > &intra_mode_in, ac_channel< ac_int< 8, false > > &intra_mode_in2, input_port_t &pred_in, resid_port_t &resid_out,
                 pixel_port_t &pred_out, pixel_port_t &ref_out) {
  static pixel_port_t pred_hier;
  static pixel_port_t ref_hier;

  pred_buffer_push(pred_in, intra_mode_in, pred_hier, ref_hier);
  pred_buffer_pull(conf_in, intra_mode_in2, pred_hier, ref_hier, resid_out, pred_out, ref_out);
}

CCS_MAIN(int argc, char *argv[]) {
  static const uint_5 loops[2][4] = {{31, 7, 1, 0}, {15, 3, 0, 0}};

  static conf_port_t conf_in;
  static input_port_t pred_in;

  static ac_channel< ac_int< 8, false > > intra_mode_in;
  static ac_channel< ac_int< 8, false > > intra_mode_in2;

  static pixel_port_t ref_out;
  static pixel_port_t pred_out;
  static resid_port_t resid_out;

  one_bit simulation_successful = 1;

  pixel_t src[32 * 32];
  pixel_t dst[32 * 32];

  for (int_8 depth = 0; depth < 4; ++depth) {
    for (uint_8 color = 0; color < 3; ++color) {
      two_bit size = (color && depth != 3) ? two_bit(depth + 1) : (two_bit)depth;
      const uint_6 width = (LCU_WIDTH / 2) >> size;
      const uint_6 mode = rand() % 35;

      for (int i = 0; i < 32 * 32; ++i) {
        src[i] = pixel_t(rand());
      }

      conf_t c;
      c.lcu_id = 0;
      c.depth = depth;
      c.color = color;
      c.intra_mode = mode;
      c.x_pos = 0;
      c.y_pos = 0;

      // Pre Config
      conf_in.write(conf_to_int(c));

      // Set selcted mode to src, others to 0
      for (int i = 0; i < width * width; i += INTRA_WIDTH) {
        input_t input = 0;
        for (int j = 0; j < INTRA_WIDTH; j++) {
          input.set_slc(mode * INTRA_WIDTH * 8 + 8 * j, src[i + j]);
        }
        input.set_slc(36 * 8 * INTRA_WIDTH, (ac_int< 1, false >)((i + INTRA_WIDTH) == (width * width)));

        pred_in.write(input);
      }
      intra_mode_in.write(mode);
      intra_mode_in2.write(mode);

      CCS_DESIGN(pred_buffer)
      (conf_in, intra_mode_in, intra_mode_in2, pred_in, resid_out, pred_out, ref_out);

      for (int i = 0; i < width * width; i += MAX_WIDTH) {
        pixel_slc_t output = pred_out.read();

        for (int j = 0; j < MAX_WIDTH; j++) {
          dst[i + j] = output.slc< 8 >(8 * j);
        }
      }

      for (int i = 0; i < width * width; ++i) {
        if (src[i] != dst[i]) {
          // printf("ERROR at index %d\n", i);
          simulation_successful = 0;
          break;
        }
      }
      if (simulation_successful) {
        printf("depth %d, color %d, mode %d OK\n", depth, color, mode);
      } else {
        printf("Error: depth %d, color %d, mode %d\n", depth, color, mode);
      }
    }
  }

  if (simulation_successful == 1)
    cout << "## SIMULATION PASSED ##" << endl;
  else
    cout << "## SIMULATION FAILED ##" << endl;

  CCS_RETURN(0);
}
