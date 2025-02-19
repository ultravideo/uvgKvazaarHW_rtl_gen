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

/**
 * \brief  Convert (x, y) coordinates to z-order index.
 *
 * Only works for widths and coordinates divisible by four. Width must be
 * a power of two in range [4..64].
 *
 * \param width   size of the containing block
 * \param x       x-coordinate
 * \param y       y-coordinate
 * \return        index in z-order
 */
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

#pragma hls_design
void buffer_pull_border(buff_conf_t::port_t &int_conf, lcu_rec_t lcu_rec[LCU_COUNT], borders_t borders[LCU_COUNT]) {
#ifndef __SYNTHESIS__
  while (int_conf.available(1)) {
#endif

    buff_conf_t conf(int_conf.read());

#pragma hls_pipeline_init_interval 1
    for (two_bit r = 1; r < 3; ++r) {
      two_bit color = (conf.color == COLOR_ALL) ? r : conf.color;

      one_bit is_chroma = color != COLOR_Y;
      two_bit size = conf.depth != (two_bit)3 ? (two_bit)(conf.depth + is_chroma) : (two_bit)3;

      uint_6 x_px = conf.x_pos >> is_chroma;
      uint_6 y_px = conf.y_pos >> is_chroma;
      uint_6 offset = ((LCU_WIDTH / 2) >> size) - 1;

      uint_7 border_offset = borders_t::baseOffset(conf.depth, color);
      uint_13 lcu_rec_color_offset = lcu_rec_t::colorOffset(color);

      if (!is_chroma && conf.depth == 3 && conf.tr_skip) {
        // Jump 4x4 pixels to Luma TR Skip
        border_offset += 4;
      }

      lcu_rec_t::slice_t *topBuffer = &borders[conf.lcu_id].top[lcu_rec_t::address(border_offset)];
      lcu_rec_t::slice_t *leftBuffer = &borders[conf.lcu_id].left[lcu_rec_t::address(border_offset)];
      lcu_rec_t::slice_t *topBorder = &lcu_rec[conf.lcu_id].top[lcu_rec_t::address(lcu_rec_color_offset + x_px)];
      lcu_rec_t::slice_t *leftBorder = &lcu_rec[conf.lcu_id].left[lcu_rec_t::address(lcu_rec_color_offset + y_px)];

      pixel_t topleft_top[8], topleft_left[8];
      pixel_t *tlPtr = &lcu_rec[conf.lcu_id].top_left[lcu_rec_t::colorTopLeftOffset(color)];

#pragma hls_pipeline_init_interval 1
      for (int i = 0, j = 0; i < MAX_WIDTH; i += INTRA_WIDTH, j++) {
        lcu_rec_t::slice_t top_slice = topBuffer[j];
        lcu_rec_t::slice_t left_slice = leftBuffer[j];

        topBorder[j] = top_slice;
        leftBorder[j] = left_slice;
        topleft_top[j] = top_slice.slc< 8 >(8 * (INTRA_WIDTH - 1));
        topleft_left[j] = left_slice.slc< 8 >(8 * (INTRA_WIDTH - 1));

        if (i + INTRA_WIDTH == (LCU_WIDTH / 2) >> size)
          break;
      }

#pragma hls_pipeline_init_interval 1
      for (uint_6 y = 0; y < MAX_WIDTH / 4; ++y) {
        // Write left edge of top left pixels
        uint_6 yo = (y_px + 4 * y) >> 2;
        uint_6 xo = (x_px + offset) >> 2;

        tlPtr[((yo * 16) >> is_chroma) + xo] = topleft_left[y];
        if (y + 1 == (LCU_WIDTH / 8) >> size)
          break;
      }

#pragma hls_pipeline_init_interval 1
      for (uint_6 x = 0; x < MAX_WIDTH / 2; ++x) {
        // Break here so last pixel won't be written twice
        if (x + 1 == (LCU_WIDTH / 8) >> size)
          break;

        // Write top edge of top left pixels
        uint_6 yo = (y_px + offset) >> 2;
        uint_6 xo = (x_px + 4 * x) >> 2;

        tlPtr[((yo * 16) >> is_chroma) + xo] = topleft_top[x];
      }
      if (conf.color != COLOR_ALL)
        break;
    }
#ifndef __SYNTHESIS__
  }
#endif
}

#pragma hls_design
void buffer_pull_rec(ac_channel< ac_int< 17 + 5 + 1, false > > &conf_in, lcu_yuv_t lcu[LCU_COUNT], pixel_slc_t rec_in[LCU_COUNT][68], coeff_slc_t coeff_in[LCU_COUNT][68],
                     buff_conf_t::port_t &int_conf, ac_channel< ac_int< 5, false > > &inst_done) {
#ifndef __SYNTHESIS__
  while (conf_in.available(1)) {
#endif
    static const uint_4 slice_loops[4] = {0, 1, 3, 3};

    // Read config
    ac_int< 17 + 5 + 1, false > conf_temp = conf_in.read();
    buff_conf_t conf(conf_temp.slc< 17 >(0));
    ac_int< 5, false > inst_done_in = conf_temp.slc< 5 >(17);
    one_bit flush = conf_temp.slc< 1 >(22);

    if (flush) {
#pragma hls_pipeline_init_interval 1
      for (two_bit r = 1; r < 3; ++r) {
        two_bit color = (conf.color == COLOR_ALL) ? r : conf.color;

        one_bit is_chroma = color != COLOR_Y;
        two_bit size = conf.depth != (two_bit)3 ? (two_bit)(conf.depth + is_chroma) : (two_bit)3;
        uint_6 x_px = conf.x_pos >> is_chroma;
        uint_6 y_px = conf.y_pos >> is_chroma;
        uint_12 cord_offset = ((uint_12)y_px << (is_chroma ? 5 : 6)) + x_px;

        // Coefficient Z-Order start posttion
        uint_12 z_cord = xy_to_zorder(is_chroma, x_px, y_px);

        // Address offsets
        uint_7 buffer_offset = offset[color][conf.depth];
        uint_13 lcu_color_offset = lcu_yuv_t::colorOffset(color);

        byte_en_t rec_byte_en = 0xFFFFFFFF; // All ones

        switch (size) {
        case 3:
          rec_byte_en >>= 4;
        case 2:
          rec_byte_en >>= 8;
        case 1:
          rec_byte_en >>= 16;
        default:
          rec_byte_en <<= (x_px & 0x1F);
        }

        if (r == 1) {
          // Write config to border unit
          int_conf.write(conf.toInt());
        }

#pragma hls_pipeline_init_interval 1
        for (uint_6 y = 0; y < MAX_WIDTH; ++y) {
          pixel_slc_t _rec = rec_in[conf.lcu_id][buffer_offset + y];
          coeff_slc_t_byte_en _coeff = coeff_in[conf.lcu_id][buffer_offset + y];

          // TR Skip for luma
          if (!is_chroma && conf.tr_skip) {
            _rec >>= 128;
            _coeff >>= 256;
          }

          // Byte Enable for 4x4 Coeffs
          bool coeff_first_half = !((z_cord >> ac::log2_ceil< 4 * 4 >::val) & 0x01);

          if (size != 3) {
            // No byte enable for blocks bigger than 4x4
            _coeff[16 * MAX_WIDTH] = 1;
            _coeff[16 * MAX_WIDTH + 1] = 1;

          } else if (coeff_first_half) {
            _coeff[16 * MAX_WIDTH] = 1;
            _coeff[16 * MAX_WIDTH + 1] = 0;

          } else {
            _coeff <<= 256;
            _coeff[16 * MAX_WIDTH] = 0;
            _coeff[16 * MAX_WIDTH + 1] = 1;
          }
#ifndef __SYNTHESIS__
          coeff_slc_t_byte_en temp = lcu[conf.lcu_id].coeff[lcu_yuv_t::address(z_cord + lcu_color_offset)];

          if (_coeff[16 * MAX_WIDTH] == 1 && _coeff[16 * MAX_WIDTH + 1] == 1) {
            for (int a = 0; a < MAX_WIDTH; a++) {
              temp.set_slc(a * 16, _coeff.slc< 16 >(a * 16));
            }
          } else if (_coeff[16 * MAX_WIDTH] == 1 && _coeff[16 * MAX_WIDTH + 1] == 0) {
            for (int a = 0; a < (MAX_WIDTH / 2); a++) {
              temp.set_slc(a * 16, _coeff.slc< 16 >(a * 16));
            }
          } else if (_coeff[16 * MAX_WIDTH] == 0 && _coeff[16 * MAX_WIDTH + 1] == 1) {
            for (int a = (MAX_WIDTH / 2); a < MAX_WIDTH; a++) {
              temp.set_slc(a * 16, _coeff.slc< 16 >(a * 16));
            }
          }
          lcu[conf.lcu_id].coeff[lcu_yuv_t::address(z_cord + lcu_color_offset)] = temp;

#else
        lcu[conf.lcu_id].coeff[lcu_yuv_t::address(z_cord + lcu_color_offset)] = _coeff;
#endif
          // Move Z Cordinate
          z_cord += MAX_WIDTH;

#pragma hls_pipeline_init_interval 1
          for (uint_4 x = 0; x < 4; ++x) {
            // Full coordinate for first pixel in current set
            uint_12 cord = ((uint_12)((y << size) + x) << (is_chroma ? 5 : 6)) + cord_offset;

            // Offset for how many pixels input needs to moved to be in correct position
            uint_5 offset = ((uint_5(cord) >> (5 - size)) - x) << (5 - size);

            // Move values to correct positions in steps of four
            pixel_slc_t_byte_en rec = ctl::RotateL< 256, 32 >(_rec, offset >> 2);

// Round coordinate and write
#ifndef __SYNTHESIS__
            pixel_slc_t_byte_en temp = lcu[conf.lcu_id].rec[lcu_yuv_t::address(cord + lcu_color_offset)];
            for (int a = 0; a < MAX_WIDTH; a++) {
              if ((rec_byte_en >> a) & 1 == 1) {
                temp.set_slc(a * 8, rec.slc< 8 >(a * 8));
              }
            }
            lcu[conf.lcu_id].rec[lcu_yuv_t::address(cord + lcu_color_offset)] = temp;
#else
          rec.set_slc(8 * MAX_WIDTH, rec_byte_en);
          lcu[conf.lcu_id].rec[lcu_yuv_t::address(cord + lcu_color_offset)] = rec;
#endif

            if (x == slice_loops[size])
              break;
          }
          if (y == loops[size])
            break;
        }

        if (conf.color != COLOR_ALL)
          break;
      }
    }
    inst_done.write(inst_done_in);
#ifndef __SYNTHESIS__
  }
#endif
}

#pragma hls_design top
void buffer_pull(ac_channel< ac_int< 17 + 5 + 1, false > > &conf_in, lcu_yuv_t lcu[LCU_COUNT], lcu_rec_t lcu_rec[LCU_COUNT], pixel_slc_t rec_in[LCU_COUNT][68], coeff_slc_t coeff_in[LCU_COUNT][68],
                 borders_t borders[LCU_COUNT], ac_channel< ac_int< 5, false > > &inst_done) {
  static buff_conf_t::port_t int_conf;

  buffer_pull_rec(conf_in, lcu, rec_in, coeff_in, int_conf, inst_done);
  buffer_pull_border(int_conf, lcu_rec, borders);
}
