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

void setTopPixels(const pixel_slc_t &rec, pixel_slc_t &top, int y, two_bit size) {
  if (y == loops[size]) {
    switch (size) {
    case 0:
      top.set_slc(0, rec);
      break;
    case 1:
      top.set_slc(0, rec.slc< 16 * 8 >(16 * 8));
      break;
    case 2:
      top.set_slc(0, rec.slc< 8 * 8 >(16 * 8 + 8 * 8));
      break;
    case 3:
      top.set_slc(0, rec.slc< 4 * 8 >(8 * 8 + 4 * 8));

      // Luma TR Skip
      top.set_slc(4 * 8, rec.slc< 32 >(16 * 8 + 8 * 8 + 4 * 8));
      break;
    }
  }
}

void setLeftPixels(const pixel_slc_t &rec, pixel_slc_t &left, int y, two_bit size) {
  switch (size) {
  case 0:
    left.set_slc(8 * y, rec.slc< 8 >(248));
    break;
  case 1:
    left.set_slc(8 * (2 * y), rec.slc< 8 >(120));
    left.set_slc(8 * (2 * y + 1), rec.slc< 8 >(248));
    break;
  case 2:
    left.set_slc(8 * (4 * y), rec.slc< 8 >(56));
    left.set_slc(8 * (4 * y + 1), rec.slc< 8 >(120));
    left.set_slc(8 * (4 * y + 2), rec.slc< 8 >(184));
    left.set_slc(8 * (4 * y + 3), rec.slc< 8 >(248));
    break;
  case 3:
    left.set_slc(8 * 0, rec.slc< 8 >(24));
    left.set_slc(8 * 1, rec.slc< 8 >(56));
    left.set_slc(8 * 2, rec.slc< 8 >(88));
    left.set_slc(8 * 3, rec.slc< 8 >(120));

    // Luma TR Skip
    left.set_slc(8 * 4, rec.slc< 8 >(152));
    left.set_slc(8 * 5, rec.slc< 8 >(184));
    left.set_slc(8 * 6, rec.slc< 8 >(216));
    left.set_slc(8 * 7, rec.slc< 8 >(248));
    break;
  }
}

#pragma hls_design
void buffer_push_border(conf_t::port_t &int_conf, pixel_port_t &top_in, pixel_port_t &left_in, borders_t borders[LCU_COUNT]) {
#ifndef __SYNTHESIS__
  while (int_conf.available(1)) {
#endif
    conf_t conf(int_conf.read());

    pixel_slc_t top = top_in.read();
    pixel_slc_t left = left_in.read();

    uint_7 border_offset = borders_t::baseOffset(conf.depth, conf.color);
    lcu_rec_t::slice_t *topBuffer = &borders[conf.lcu_id].top[lcu_rec_t::address(border_offset)];
    lcu_rec_t::slice_t *leftBuffer = &borders[conf.lcu_id].left[lcu_rec_t::address(border_offset)];

    int border_size = (LCU_WIDTH / 2) >> conf.size();
    if (!conf.color && conf.depth == 3) {
      // Add 4x4 Luma TR Skip to end of the slice
      border_size += 4;
    }

// Write borders to separate buffer
#pragma hls_pipeline_init_interval 1
    for (int i = 0, j = 0; i < MAX_WIDTH; i += INTRA_WIDTH, ++j) {
      topBuffer[j] = top.slc< 8 * INTRA_WIDTH >(i * 8);
      leftBuffer[j] = left.slc< 8 * INTRA_WIDTH >(i * 8);

      if (i + INTRA_WIDTH == border_size)
        break;
    }
#ifndef __SYNTHESIS__
  }
#endif
}

#pragma hls_design
void buffer_push_rec(pixel_port_t &rec_in, coeff_port_t &coeff_in, ac_channel< int_192 > &conf_out, pixel_slc_t rec_out[LCU_COUNT][68], coeff_slc_t coeff_out[LCU_COUNT][68], conf_t::port_t &int_conf,
                     pixel_port_t &top_out, pixel_port_t &left_out) {
#ifndef __SYNTHESIS__
  while (rec_in.available(1)) {
#endif
    conf_t conf(rec_in.read());
    pixel_slc_t top = 0, left = 0;

    uint_7 buffer_offset = offset[conf.color][conf.depth];

#pragma hls_pipeline_init_interval 1
    for (int y = 0; y < MAX_WIDTH; ++y) {
      pixel_slc_t rec = rec_in.read();
      coeff_slc_t coeff = coeff_in.read();

      setTopPixels(rec, top, y, conf.size());
      setLeftPixels(rec, left, y, conf.size());

      rec_out[conf.lcu_id][buffer_offset + y] = rec;
      coeff_out[conf.lcu_id][buffer_offset + y] = coeff;
      if (y == loops[conf.size()])
        break;
    }

    // Config & Intra mode
    int_192 res_out;
    res_out.set_slc(0, conf.lcu_id);
    res_out.set_slc(4, conf.depth);
    res_out.set_slc(6, conf.color);
    res_out.set_slc(8, conf.intra_mode);

    // SSDs
    int_96 ssd_costs = rec_in.read();
    res_out.set_slc(16, ssd_costs.slc< 26 >(0));
    res_out.set_slc(42, ssd_costs.slc< 26 >(32));
    res_out.set_slc(68, ssd_costs.slc< 26 >(64));
    // Coeff bits
    res_out.set_slc(96, int_96(coeff_in.read()));

    top_out.write(top);
    left_out.write(left);

    conf_out.write(res_out);
    int_conf.write(conf.toInt());
#ifndef __SYNTHESIS__
  }
#endif
}

#pragma hls_design top
void buffer_push(pixel_port_t &rec_in, coeff_port_t &coeff_in, ac_channel< int_192 > &conf_out, pixel_slc_t rec_out[LCU_COUNT][68], coeff_slc_t coeff_out[LCU_COUNT][68],
                 borders_t borders[LCU_COUNT]) {
  static conf_t::port_t int_conf;
  static pixel_port_t int_top;
  static pixel_port_t int_left;

  buffer_push_rec(rec_in, coeff_in, conf_out, rec_out, coeff_out, int_conf, int_top, int_left);
  buffer_push_border(int_conf, int_top, int_left, borders);
}
