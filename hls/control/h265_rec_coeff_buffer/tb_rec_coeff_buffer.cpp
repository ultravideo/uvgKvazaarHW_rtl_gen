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
#include <time.h>

static uint12 xy_to_zorder(bool is_chroma, uint6 x, uint6 y) {
  uint12 result = 0;

  if (!is_chroma) {
    result += x / 32 * (32 * 32);
    result += y / 32 * (64 * 32);
    x %= 32;
    y %= 32;
  }

  result += x / 16 * (16 * 16);
  result += y / 16 * (32 * 16);
  x %= 16;
  y %= 16;

  result += x / 8 * (8 * 8);
  result += y / 8 * (16 * 8);
  x %= 8;
  y %= 8;

  result += x / 4 * (4 * 4);
  result += y / 4 * (8 * 4);

  return result;
}

void buffer_push(pixel_port_t &rec_in, coeff_port_t &coeff_in, ac_channel< int_192 > &conf_out, pixel_slc_t rec_out[LCU_COUNT][68], coeff_slc_t coeff_out[LCU_COUNT][68], borders_t borders[LCU_COUNT]);

void buffer_pull(ac_channel< ac_int< 17 + 5 + 1, false > > &conf_in, lcu_yuv_t lcu[LCU_COUNT], lcu_rec_t lcu_rec[LCU_COUNT], pixel_slc_t rec_in[LCU_COUNT][68], coeff_slc_t coeff_in[LCU_COUNT][68],
                 borders_t borders[LCU_COUNT], ac_channel< ac_int< 5, false > > &inst_done);

CCS_MAIN(int argc, char *argv[]) {
  srand(time(NULL));
  int simulation_successful = 1;

  static pixel_port_t rec_port;
  static coeff_port_t coeff_port;

  static ac_channel< int_192 > conf_out;
  static ac_channel< ac_int< 17 + 5 + 1, false > > ctrl_in;

  static ac_channel< ac_int< 5, false > > inst_done;

  static ac_channel< byte_en_t > rbyte;

  static lcu_yuv_t lcu[LCU_COUNT];
  static lcu_rec_t lcu_rec[LCU_COUNT];

  static pixel_slc_t rec_in[LCU_COUNT][68];
  static coeff_slc_t coeff_in[LCU_COUNT][68];
  static borders_t borders[LCU_COUNT];

  memset(lcu, 0, sizeof(lcu));
  memset(lcu_rec, 0, sizeof(lcu_rec));

  memset(rec_in, 0, sizeof(rec_in));
  memset(coeff_in, 0, sizeof(coeff_in));
  memset(borders, 0, sizeof(borders));

  // Run 64 randomized tests
  for (int tests = 0; tests < 64; tests++) {

    // Generate random configuration
    conf_t conf;
    conf.lcu_id = rand() % 16;
    conf.color = rand() % 3;
    if (conf.color != 0) {
      conf.depth = rand() % 3;
    } else {
      conf.depth = rand() % 4;
    }
    conf.x_pos = (32 >> conf.depth) * (rand() % (conf.depth + 2));
    conf.y_pos = (32 >> conf.depth) * (rand() % (conf.depth + 2));

    printf("config: lcu_id = %d, color = %d, depth = %d, x_pos = %d, y_pos = %d\n", conf.lcu_id, conf.color, conf.depth, conf.x_pos, conf.y_pos);

    buff_conf_t buff_conf;
    buff_conf.lcu_id = conf.lcu_id;
    buff_conf.depth = conf.depth;
    buff_conf.color = conf.color;
    buff_conf.x_pos = conf.x_pos;
    buff_conf.y_pos = conf.y_pos;
    buff_conf.tr_skip = 0;

    static const uint_5 loops[4] = {31, 7, 1, 0};

    // Config
    rec_port.write(conf.toInt());
    ac_int< 17 + 5 + 1, false > conf_temp = buff_conf.toInt();
    conf_temp[22] = 1; // flush
    ctrl_in.write(conf_temp);

    pixel_t src[32 * 32];

    // Generate source test data
    pixel_t counter = 0;
    for (int y = 0; y < 32; ++y) {
      for (int x = 0; x < 32; ++x) {
        src[y * 32 + x] = (rand() + 128) % 256;
      }
    }

    // Write source data to rec and coeff ports
    pixel_slc_t slice;
    coeff_slc_t coeff_slice;
    for (int y = 0; y < MAX_WIDTH; ++y) {
      for (int x = 0; x < MAX_WIDTH; ++x) {
        slice.set_slc(8 * x, src[y * MAX_WIDTH + x]);
        coeff_slice.set_slc(16 * x, (ac_int< 16, false >)src[y * MAX_WIDTH + x]);
      }

      rec_port.write(slice);
      coeff_port.write(coeff_slice);
      if (y == loops[conf.size()])
        break;
    }

    // costs are just passed thru in the DUT
    rec_port.write(0);
    coeff_port.write(0);

    while (rec_port.available(1)) {
      CCS_DESIGN(buffer_push)
      (rec_port, coeff_port, conf_out, rec_in, coeff_in, borders);
    }

    CCS_DESIGN(buffer_pull)
    (ctrl_in, lcu, lcu_rec, rec_in, coeff_in, borders, inst_done);

    int is_chroma = conf.color != COLOR_Y;

    int i_src = 0;
    conf.y_pos = conf.y_pos >> is_chroma;
    conf.x_pos = conf.x_pos >> is_chroma;

    for (int i = conf.y_pos; i < conf.y_pos + conf.width(); ++i) {
      int j = conf.x_pos / (32 >> is_chroma);
      int j_src = 0;
      for (int k = conf.x_pos - j * (32 >> is_chroma); k < (conf.x_pos - j * (32) + conf.width()); ++k) {

        if (lcu[conf.lcu_id].rec[lcu_yuv_t::address(lcu_yuv_t::colorOffset(conf.color)) + i * ((64 >> is_chroma) / MAX_WIDTH) + j].slc< 8 >(8 * k) != src[i_src * conf.width() + j_src]) {
          printf("Error: Rec Array incorrect\n");
          simulation_successful = 0;
          break;
        }
        j_src++;
      }
      if (simulation_successful == 0) {
        break;
      }
      i_src++;
    }

    if (simulation_successful == 0) {
      break;
    }

    uint_12 z_cord = xy_to_zorder(is_chroma, conf.x_pos, conf.y_pos);
    bool coeff_second_half = ((z_cord >> ac::log2_ceil< 4 * 4 >::val) & 0x01);
    i_src = 0;
    for (int y = 0; y < conf.width() * conf.width(); y = y + MAX_WIDTH) {
      for (int x = 16 * (coeff_second_half); x < MAX_WIDTH; x++) {
        if (lcu[conf.lcu_id].coeff[lcu_yuv_t::address(z_cord + lcu_yuv_t::colorOffset(conf.color))].slc< 16 >(16 * x) != src[i_src++]) {
          printf("Error: Coeff Array incorrect\n");
          simulation_successful = 0;
          break;
        }
        if ((conf.depth == 3 || (conf.depth == 2 && is_chroma)) && !coeff_second_half && x == 15) {
          break;
        }
      }
      if (simulation_successful == 0) {
        break;
      }
      z_cord += MAX_WIDTH;
    }

    if (simulation_successful == 0) {
      break;
    }

    lcu_rec_t::slice_t *topBorder = &lcu_rec[conf.lcu_id].top[lcu_rec_t::address(lcu_rec_t::colorOffset(conf.color) + conf.x_pos)];

    lcu_rec_t::slice_t *leftBorder = &lcu_rec[conf.lcu_id].left[lcu_rec_t::address(lcu_rec_t::colorOffset(conf.color) + conf.y_pos)];

    int tl_offset = (conf.x_pos >> 2) + (((16 >> is_chroma) * conf.y_pos) >> 2);
    pixel_t *tlPtr = &lcu_rec[conf.lcu_id].top_left[tl_offset + lcu_rec_t::colorTopLeftOffset(conf.color)];

    for (int a = 0; a < conf.width() / INTRA_WIDTH; a++) {
      for (int i = 0; i < INTRA_WIDTH; i++) {
        if (topBorder[a].slc< 8 >(i * 8) != src[conf.width() * (conf.width() - 1) + (a * INTRA_WIDTH + i)]) {
          printf("Error: Top Border incorrect\n");
          simulation_successful = 0;
          break;
        }
      }
      if (simulation_successful == 0) {
        break;
      }
    }

    if (simulation_successful == 0) {
      break;
    }

    for (int a = 0; a < conf.width() / INTRA_WIDTH; a++) {
      for (int i = 0; i < INTRA_WIDTH; i++) {
        if (leftBorder[a].slc< 8 >(i * 8) != src[conf.width() - 1 + conf.width() * (a * INTRA_WIDTH + i)]) {
          printf("Error: Left Border incorrect\n");
          simulation_successful = 0;
          break;
        }
      }
      if (simulation_successful == 0) {
        break;
      }
    }

    if (simulation_successful == 0) {
      break;
    }

    for (int a = 0; a < conf.width() / 4; a++) {
      if (tlPtr[(conf.width() / 4) - 1 + (16 >> is_chroma) * a] != src[conf.width() - 1 + conf.width() * ((a + 1) * 4 - 1)]) {
        printf("Error: TopLeft left part incorrect\n");
        simulation_successful = 0;
        break;
      }
      if (simulation_successful == 0) {
        break;
      }
    }

    if (simulation_successful == 0) {
      break;
    }

    for (int a = 0; a < conf.width() / 4; a++) {
      if (tlPtr[((conf.width().to_uint() / 4) - 1) * (16 >> is_chroma) + a] != src[conf.width() * (conf.width() - 1) + ((a + 1) * 4 - 1)]) {
        printf("Error: TopLeft top part incorrect\n");
        simulation_successful = 0;
        break;
      }
      if (simulation_successful == 0) {
        break;
      }
    }

    if (simulation_successful == 0) {
      break;
    }
  }
  if (simulation_successful) {
    cout << "## SIMULATION PASSED ##" << endl;
  } else {
    cout << "## SIMULATION FAILED ##" << endl;
  }

  CCS_RETURN(0);
}
