#include "cabac.h"

uint_8 cabac_indexes[33] = {0, 1, 2, 5, 6, 8, 13, 16, 20, 24, 28, 32, 36, 63, 78, 93, 108, 123, 138, 154, 162, 166, 168, 169, 172, 173, 174, 175, 177, 179, 181, 182, 183};

// Cabac command functions
ac_int< 33, false > kvz_cabac_encode_bin(uint_8 ctx_cur, ac_int< 9, false > *range, ac_int< 1, false > bin_value) {
  uint_8 lps = kvz_g_auc_lpst_table[CTX_STATE(ctx_cur)][(*range).slc< 2 >(6)];

  ac_int< 9, false > range_new = *range - lps;
  ac_int< 1, false > range_shift = 0;
  ac_int< 33, false > temp = 0;

  if (bin_value != CTX_MPS(ctx_cur)) {
    ac_int< 3, false > num_bits = kvz_g_auc_renorm_table[lps >> 3];

    temp[0] = 1;
    temp.set_slc(1, range_new);
    temp.set_slc(1 + 24, num_bits);

    range_new = (ac_int< 16, false >)lps << num_bits;
  } else if (range_new.slc< 1 >(8) == 0) {
    {
      temp[0] = 0;
      temp.set_slc(1, (ac_int< 1, false >)0);
      temp.set_slc(1 + 24, (ac_int< 1, false >)1);
      range_shift = 1;
    }
  }

  *range = range_new << range_shift;

  return temp;
}

/**
 * \brief
 */

ac_int< 64 + 2, false > kvz_cabac_encode_bins_ep(ac_int< 9, false > range, ac_int< 16, false > bin_values, ac_int< 6, false > num_bins) {
  ac_int< 8, false > pattern;

  ac_int< 64 + 2, false > temp = 0;
  if (num_bins != 8 && num_bins.slc< 5 >(3) != 0) {
    {
      num_bins -= 8;
      pattern = bin_values.slc< 8 >(num_bins);

      temp[0] = 0;
      temp.set_slc(1, (ac_int< 24, false >)(range * pattern));
      temp.set_slc(1 + 24, (ac_int< 8, false >)8);
    }
  }

  temp[33] = 0;
  temp.set_slc(1 + 33, (ac_int< 24, false >)(range * (uint_8)(bin_values & (0xff >> (8 - num_bins)))));
  temp.set_slc(1 + 33 + 24, num_bins);

  return temp;
}

ac_int< 33, false > kvz_cabac_encode_bin_ep(ac_int< 9, false > range, ac_int< 1, false > bin_value) {
  ac_int< 33, false > temp = 0;
  temp[0] = 0;
  temp.set_slc(1 + 24, (ac_int< 1, false >)1);

  if (bin_value) {
    temp.set_slc(1, range);
  }

  return temp;
}

// Cabac state indexes written down    Actual Order

/*uint_8_t &sao_merge_flag_model,      0      0
uint_8_t &sao_type_idx_model,          1      1
uint_8_t split_flag_model[3],          2      2
uint_8_t &intra_mode_model,            5      3
uint_8_t chroma_pred_model[2],         6      4
uint_8_t inter_dir[5],                 8      5
uint_8_t trans_subdiv_model[3],        13     6
uint_8_t qt_cbf_model_luma[4],         16     7
uint_8_t qt_cbf_model_chroma[4],       20     8
uint_8_t cu_qp_delta_abs[4],           24     9
uint_8_t part_size_model[4],           28     10
uint_8_t cu_sig_coeff_group_model[4],  32     11
uint_8_t cu_sig_model_luma[27],        36     12
uint_8_t cu_sig_model_chroma[15],      63     13
uint_8_t cu_ctx_last_y_luma[15],       78     14
uint_8_t cu_ctx_last_y_chroma[15],     93     15
uint_8_t cu_ctx_last_x_luma[15],       108    16
uint_8_t cu_ctx_last_x_chroma[15],     123    17
uint_8_t cu_one_model_luma[16],        138    18
uint_8_t cu_one_model_chroma[8],       154    19
uint_8_t cu_abs_model_luma[4],         162    20
uint_8_t cu_abs_model_chroma[2],       166    21
uint_8_t &cu_pred_mode_model,          168    22
uint_8_t cu_skip_flag_model[3],        169    23
uint_8_t &cu_merge_idx_ext_model,      172    24
uint_8_t &cu_merge_flag_ext_model,     173    25
uint_8_t &cu_transquant_bypass,        174    26
uint_8_t cu_mvd_model[2],              175    27
uint_8_t cu_ref_pic_model[2],          177    28
uint_8_t mvp_idx_model[2],             179    29
uint_8_t &cu_qt_root_cbf_model,        181    30
uint_8_t &transform_skip_model_luma,   182    31
uint_8_t &transform_skip_model_chroma, 183    32*/

// Pipeline block for reading and updating cabac_states
#pragma hls_design top
void cabac_write_main_state(uint_8 cabac_states[256 * LCU_COUNT], cabac_chan &cabac_write, cabac_chan &cabac_write_out) {
  static ac_int< 5, false > ctu_id = 0;
  cabac_val temp = 0;

  temp = cabac_write.read();

  cabac_command command = cabac_val_to_struct(temp);

  ac_int< 8, false > cabac_idx = cabac_indexes[command.ctx] + command.ctx_index;
  if (command.cmd == CMD_START) {
    ctu_id = command.ctx_index;
  }
  if (command.cmd == CMD_ENCODE_BIN) {
    uint_8 cur_ctx = cabac_states[256 * ctu_id + cabac_idx];

    uint_8 cur_ctx_new = 0;
    uint_8 lps = kvz_g_auc_next_state_lps[cur_ctx];
    uint_8 mps = kvz_g_auc_next_state_mps[cur_ctx];

    if ((ac_int< 1, false >)command.bin_value_s != CTX_MPS(cur_ctx)) {
      cur_ctx_new = lps;
    } else {
      cur_ctx_new = mps;
    }
    cabac_states[256 * ctu_id + cabac_idx] = cur_ctx_new;
    temp.set_slc(3, cur_ctx);
  }
  cabac_write_out.write(temp);
}

// Pipeline block for processing the actual encoding command and updating range
#pragma hls_design top
void cabac_write_main_cmd(ac_int< 32, false > low, ac_int< 16, false > range, ac_int< 8, false > buffered_byte, ac_int< 8, false > num_buffered_bytes, ac_int< 8, false > bits_left, uint_8 stream_data,
                          uint_8 stream_cur_bit, uint_8 stream_zercount, cabac_chan &cabac_write, ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > &cabac_write_command) {
  static ac_int< 9, false > range_local = 0;
  static ac_int< 5, false > ctu_id = 0;
  cabac_val temp = 0;
  temp = cabac_write.read();

  cabac_command command = cabac_val_to_struct(temp);

  ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > temp_data = 0;

  if (command.cmd == CMD_START) {
    ctu_id = command.ctx_index;
    range_local = range;

    temp_data.set_slc(2, low);
    temp_data.set_slc(2 + 32 + 16, buffered_byte);
    temp_data.set_slc(2 + 32 + 16 + 8, num_buffered_bytes);
    temp_data.set_slc(2 + 32 + 16 + 8 + 8, bits_left);
    temp_data.set_slc(2 + 32 + 16 + 8 + 8 + 8, stream_zercount);
    temp_data.set_slc(2 + 32 + 16 + 8 + 8 + 8 + 8 + 8, ctu_id);
    temp_data[0] = 1;
  } else if (command.cmd == CMD_STOP) {
    temp_data[1] = 1;
    temp_data.set_slc(2 + 32, range_local);
  } else if (command.cmd == CMD_ENCODE_BINS_EP) {
    temp_data.set_slc(2, kvz_cabac_encode_bins_ep(range_local, command.bin_value_s, (ac_int< 6, false >)command.num_bins));
  } else if (command.cmd == CMD_ENCODE_BIN) {
    uint_8 cur_ctx = temp.slc< 8 >(3);
    temp_data.set_slc(2, kvz_cabac_encode_bin(cur_ctx, &range_local, (ac_int< 1, false >)command.bin_value_s));
  } else if (command.cmd == CMD_ENCODE_BIN_EP) {
    temp_data.set_slc(2, kvz_cabac_encode_bin_ep(range_local, (ac_int< 1, false >)command.bin_value_s));
  }

  cabac_write_command.write(temp_data);
}

// Pipeline block for updating bits_left
void cabac_bits(ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > &cabac_write_command_in,
                ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > &cabac_write_command_out) {
#ifndef __SYNTHESIS__
  while (cabac_write_command_in.available(1)) {
#endif

    static ac_int< 6, false > bits_left = 0;

    ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > command_out = 0;

    ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > command = cabac_write_command_in.read();
    if (command[0] == 1) {
      bits_left = command.slc< 6 >(2 + 32 + 16 + 8 + 8);
      cabac_write_command_out.write(command);
    } else if (command[1] == 1) {
      command.set_slc(2 + 32 + 16 + 8 + 8, bits_left);
      cabac_write_command_out.write(command);
    } else {
      ac_int< 32, false > bit_mask_temp = 0xffffffff;
      command_out = command;

      bits_left -= command.slc< 6 >(2 + 1 + 24);
      command_out.set_slc(2 + 1 + 24 + 8 + 32, (ac_int< 6, false >)(24 - bits_left));

      if ((bits_left) < 12) {
        bit_mask_temp = 0x00ffffff >> (bits_left);
        bits_left += 8;
      }

      command_out.set_slc(2 + 1 + 24 + 8, bit_mask_temp);
      cabac_write_command_out.write(command_out);
    }
#ifndef __SYNTHESIS__
  }
#endif
}

// Helper functions for updating low
ac_int< 32, false > calc_low1(ac_int< 32, false > low, ac_int< 24, false > add, ac_int< 6, false > shift) { return (low << shift) + add; }

ac_int< 32, false > calc_low2(ac_int< 32, false > low, ac_int< 24, false > add, ac_int< 6, false > shift) { return (low + add) << shift; }

// Pipeline block for updating low
void cabac_low(ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > &cabac_write_command_in,
               ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > &cabac_write_command_out) {
#ifndef __SYNTHESIS__
  while (cabac_write_command_in.available(1)) {
#endif

    static ac_int< 32, false > low = 0;

    ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > command_out = 0;

    ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > command = cabac_write_command_in.read();
    if (command[0] == 1) {
      low = command.slc< 32 >(2);
      cabac_write_command_out.write(command);
    } else if (command[1] == 1) {
      command.set_slc(2, low);
      cabac_write_command_out.write(command);
    } else {
      ac_int< 32, false > bit_mask_temp = command.slc< 32 >(2 + 1 + 24 + 8);
      ac_int< 6, false > lead_shift = command.slc< 6 >(2 + 1 + 24 + 8 + 32);
      if (command[2] == 0) {
        low = calc_low1(low, command.slc< 24 >(2 + 1), command.slc< 8 >(2 + 1 + 24));
      } else {
        low = calc_low2(low, command.slc< 24 >(2 + 1), command.slc< 8 >(2 + 1 + 24));
      }

      ac_int< 9, false > lead_byte = (low >> lead_shift);
      if (bit_mask_temp != 0xffffffff) {
        command_out.set_slc(2, lead_byte);
        cabac_write_command_out.write(command_out);
      }

      low &= bit_mask_temp;
    }
#ifndef __SYNTHESIS__
  }
#endif
}

// Pipeline block for buffering bytes
void kvz_cabac_write(ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > &command_in, ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > &bitstream_put_byte) {
#ifndef __SYNTHESIS__
  while (command_in.available(1)) {
#endif

    static ac_int< 8, false > buffered_byte = 0;
    static ac_int< 8, false > num_buffered_bytes = 0;

    ac_int< 1, false > finish = 0;
    ac_int< 1, false > send_bytes = 0;
    ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > bitstream_put_byte_temp = 0;

    ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > command = command_in.read();

    if (command[0] == 1) {
      buffered_byte = command.slc< 8 >(2 + 32 + 16);
      num_buffered_bytes = command.slc< 8 >(2 + 32 + 16 + 8);

      bitstream_put_byte.write(command);
    } else if (command[1] == 1) {
      command.set_slc(2 + 32 + 16, buffered_byte);
      command.set_slc(2 + 32 + 16 + 8, num_buffered_bytes);

      bitstream_put_byte.write(command);
    } else {
      {
        ac_int< 9, false > lead_byte = command.slc< 9 >(2);

        if (lead_byte == (ac_int< 16, false >)0xff) {
          num_buffered_bytes++;
        } else {
          {
            ac_int< 9, false > byte = buffered_byte + lead_byte.slc< 1 >(8);
            buffered_byte = lead_byte.slc< 8 >(0);
            ac_int< 9, false > byte2 = lead_byte.slc< 1 >(8) ? 0 : 0xff;

            if (num_buffered_bytes != 0) {
              bitstream_put_byte_temp.set_slc(2, byte + 1);

#pragma hls_unroll yes
              for (ac_int< 4, false > aa = 0; aa < 8; aa++) {
                if (num_buffered_bytes - aa > 1) {
                  bitstream_put_byte_temp.set_slc(2 + 9 + aa * 9, byte2 + 1);
                }
              }
              bitstream_put_byte.write(bitstream_put_byte_temp);
            }
            num_buffered_bytes = 1;
          }
        }
      }
    }

#ifndef __SYNTHESIS__
  }
#endif
}

// Pipeline block for updating bytes written (len) and updating zerocount
void kvz_bitstream_put_byte(ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > &bitstream_put_byte, ac_channel< ac_int< 16 + 1, false > > &bs_data,
                            ac_channel< ac_int< 5, false > > &ctu_done, ac_int< 128, false > cabac_metas[LCU_COUNT * 2]) {
#ifndef __SYNTHESIS__
  while (bitstream_put_byte.available(1)) {
#endif
    static ac_int< 16, false > len = 0;
    static ac_int< 8, false > zerocount = 0;
    static ac_int< 5, false > ctu_id = 0;

    ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > command = bitstream_put_byte.read();
    if (command[0] == 1) {
      len = 0;
      zerocount = command.slc< 8 >(2 + 32 + 16 + 8 + 8 + 8);
      ctu_id = command.slc< 5 >(2 + 32 + 16 + 8 + 8 + 8 + 8 + 8);
      ac_int< 16 + 1, false > bs_data_temp = ctu_id;
      bs_data_temp <<= 1;
      bs_data_temp |= 1;
      bs_data.write(bs_data_temp);
    } else if (command[1] == 1) {
      ac_int< 128, false > cabac_metas_temp = 0;

      cabac_metas_temp.set_slc(0, command.slc< 32 >(2));                                // low
      cabac_metas_temp.set_slc(32, command.slc< 16 >(2 + 32));                          // range
      cabac_metas_temp.set_slc(32 + 16, command.slc< 8 >(2 + 32 + 16));                 // buffered_byte
      cabac_metas_temp.set_slc(32 + 16 + 8, command.slc< 8 >(2 + 32 + 16 + 8));         // num_buffered_bytes
      cabac_metas_temp.set_slc(32 + 16 + 8 + 8, command.slc< 8 >(2 + 32 + 16 + 8 + 8)); // bits_left
      cabac_metas_temp.set_slc(32 + 16 + 8 + 8 + 8, zerocount);
      cabac_metas_temp.set_slc(32 + 16 + 8 + 8 + 8 + 8, len);

      cabac_metas[ctu_id * 2] = cabac_metas_temp;
      ctu_done.write(ctu_id);
    } else {
      uint_8 emulation_prevention_three_byte = 0x03;
      {
        uint_8 byte = command.slc< 8 >(2);
        ac_int< 16 + 1, false > temp_data = 0;

        temp_data.set_slc(9, byte);
        ac_int< 2, false > add_len = 0;
        if ((zerocount == 2) && byte.slc< 8 - 2 >(2) == 0) {
          temp_data.set_slc(1, emulation_prevention_three_byte);
          zerocount = (byte == 0);
          len += 2;
        } else {
          zerocount = byte == 0 ? (uint_8)(zerocount + 1) : (uint_8)0;
          len += 1;
        }
        bs_data.write(temp_data);
      }
    }
#ifndef __SYNTHESIS__
  }
#endif
}

// Pipeline block for writing bytes to bitstream memory
void cabac_write_bitstream(uint_8 bitstream_last_data[KVZ_DATA_CHUNK_SIZE * LCU_COUNT], ac_channel< ac_int< 16 + 1, false > > &bs_data) {
  static ac_int< 16, false > len = 0;
  static ac_int< 5, false > ctu_id = 0;
#ifndef __SYNTHESIS__
  while (bs_data.available(1)) {
#endif
    ac_int< 16 + 1, false > temp = bs_data.read();
    if (temp[0] == 1) {
      ctu_id = temp.slc< 5 >(1);
      len = 0;
    } else {
      {
        if (temp.slc< 8 >(1) != 0) {
          bitstream_last_data[KVZ_DATA_CHUNK_SIZE * ctu_id + len++] = temp.slc< 8 >(1);
        }
        bitstream_last_data[KVZ_DATA_CHUNK_SIZE * ctu_id + len++] = temp.slc< 8 >(8 + 1);
      }
    }
#ifndef __SYNTHESIS__
  }
#endif
}

// Pipeline serializer between cabac_write_main_cmd and cabac_bits
void cabac_low_shift_add_buf(ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > &cabac_write_command_in,
                             ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > &cabac_write_command_out) {
#ifndef __SYNTHESIS__
  while (cabac_write_command_in.available(1)) {
#endif
    ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > temp = cabac_write_command_in.read();
    if (temp.slc< 2 >(0) == 0) {
      temp >>= 2;
      for (ac_int< 3, false > aa = 0; aa < 2; aa++) {
        ac_int< 33, false > low_shift_add = temp.slc< 33 >(0);
        temp >>= 33;
        if (low_shift_add != 0) {
          ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > temp_out = 0;
          temp_out.set_slc(2, low_shift_add);
          cabac_write_command_out.write(temp_out);
        }
        if (temp == 0) {
          break;
        }
      }

    } else {
      cabac_write_command_out.write(temp);
    }

#ifndef __SYNTHESIS__
  }
#endif
}

// Pipeline serializer between kvz_cabac_write and kvz_bitstream_put_byte
void kvz_bitstream_put_byte_buf(ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > &bitstream_put_byte_in,
                                ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > &bitstream_put_byte_out) {
#ifndef __SYNTHESIS__
  while (bitstream_put_byte_in.available(1)) {
#endif
    ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > temp = bitstream_put_byte_in.read();
    if (temp.slc< 2 >(0) == 0) {
      temp >>= 2;
      for (ac_int< 4, false > aa = 0; aa < 9; aa++) {
        ac_int< 9, false > byte = temp.slc< 9 >(0);
        temp >>= 9;
        if (byte != 0) {
          ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > temp_out = 0;
          temp_out.set_slc(2, byte - 1);
          bitstream_put_byte_out.write(temp_out);
        }
        if (temp == 0) {
          break;
        }
      }

    } else {
      bitstream_put_byte_out.write(temp);
    }

#ifndef __SYNTHESIS__
  }
#endif
}

// Clock crossing and arbiter blocks between all cabac command writers and actual cabac_write block
void cabac_write_buf(cabac_chan &cabac_write_out, cabac_chan &cabac_write) {
#ifndef __SYNTHESIS__
  while (cabac_write.available(1)) {
#endif

    cabac_write_out.write(cabac_write.read());

#ifndef __SYNTHESIS__
  }
#endif
}

void cabac_write_encode_buf(cabac_chan &cabac_write_out, cabac_chan &cabac_write_encode) {
#ifndef __SYNTHESIS__
  while (cabac_write_encode.available(1)) {
#endif

    cabac_write_out.write(cabac_write_encode.read());

#ifndef __SYNTHESIS__
  }
#endif
}

void cabac_write_coding_tree_buf(cabac_chan &cabac_write_out, cabac_chan &cabac_write_coding_tree, ac_int< 1, false > stall_coding_tree) {
#ifndef __SYNTHESIS__
  while (cabac_write_coding_tree.available(1)) {
#endif

    if (stall_coding_tree == 0) {
      cabac_write_out.write(cabac_write_coding_tree.read());
    }

#ifndef __SYNTHESIS__
  }
#endif
}

void cabac_write_demux(cabac_chan &cabac_write_out, cabac_chan &cabac_write, cabac_chan &cabac_write_encode, cabac_chan &cabac_write_coding_tree) {
  static ac_int< 1, false > p[3];

  p[0] = cabac_write.size() > 0;
  p[1] = cabac_write_encode.size() > 0;
  p[2] = cabac_write_coding_tree.size() > 0;

  if (p[0]) {
    cabac_write_out.write(cabac_write.read());
  } else if (p[1]) {
    cabac_write_out.write(cabac_write_encode.read());
  } else if (p[2]) {
    cabac_write_out.write(cabac_write_coding_tree.read());
  }
}

void cabac_write_dc_buf(cabac_chan &cabac_write_in, cabac_chan &cabac_write_out) {
#ifndef __SYNTHESIS__
  while (cabac_write_in.available(1)) {
#endif
    cabac_write_out.write(cabac_write_in.read());
#ifndef __SYNTHESIS__
  }
#endif
}

void binary_coding(uint_8 cabac_states[256 * LCU_COUNT], ac_int< 32, false > low, ac_int< 16, false > range, ac_int< 8, false > buffered_byte, ac_int< 8, false > num_buffered_bytes,
                   ac_int< 8, false > bits_left, uint_8 stream_data, uint_8 stream_cur_bit, uint_8 stream_zercount, cabac_chan &cabac_write, cabac_chan &cabac_write_encode,
                   cabac_chan &cabac_write_coding_tree, ac_channel< ac_int< 5, false > > &ctu_done, uint_8 bitstream_last_data[KVZ_DATA_CHUNK_SIZE * LCU_COUNT],
                   ac_int< 128, false > cabac_metas[LCU_COUNT * 2], ac_int< 1, false > stall_coding_tree) {
  static ac_channel< ac_int< 16 + 1, false > > bs_data;
  static cabac_chan cabac_write_hier;
  static cabac_chan cabac_write_hier2;
  static cabac_chan cabac_write_hier3;

  static cabac_chan cabac_write_b;
  static cabac_chan cabac_write_encode_b;
  static cabac_chan cabac_write_coding_tree_b;

  static ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > cabac_write_command1;
  static ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > cabac_write_command2;
  static ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > cabac_write_command3;
  static ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > cabac_write_command4;

  static ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > bitstream_put_byte1;
  static ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > bitstream_put_byte2;

  cabac_write_buf(cabac_write_b, cabac_write);
  cabac_write_encode_buf(cabac_write_encode_b, cabac_write_encode);
  cabac_write_coding_tree_buf(cabac_write_coding_tree_b, cabac_write_coding_tree, stall_coding_tree);

  cabac_write_demux(cabac_write_hier, cabac_write_b, cabac_write_encode_b, cabac_write_coding_tree_b);
  cabac_write_dc_buf(cabac_write_hier, cabac_write_hier2);

  cabac_write_main_state(cabac_states, cabac_write_hier2, cabac_write_hier3);
  cabac_write_main_cmd(low, range, buffered_byte, num_buffered_bytes, bits_left, stream_data, stream_cur_bit, stream_zercount, cabac_write_hier3, cabac_write_command1);

  cabac_low_shift_add_buf(cabac_write_command1, cabac_write_command2);

  cabac_bits(cabac_write_command2, cabac_write_command3);
  cabac_low(cabac_write_command3, cabac_write_command4);
  kvz_cabac_write(cabac_write_command4, bitstream_put_byte1);
  kvz_bitstream_put_byte_buf(bitstream_put_byte1, bitstream_put_byte2);
  kvz_bitstream_put_byte(bitstream_put_byte2, bs_data, ctu_done, cabac_metas);

  cabac_write_bitstream(bitstream_last_data, bs_data);
}

void bin_arbiter(cabac_chan &cabac_write, cabac_chan &cabac_write_encode, cabac_chan &cabac_write_coding_tree, cabac_chan &cabac_write_out, ac_int< 1, false > stall_coding_tree) {
  static cabac_chan cabac_write_b;
  static cabac_chan cabac_write_encode_b;
  static cabac_chan cabac_write_coding_tree_b;
  static cabac_chan cabac_write_hier;

  cabac_write_buf(cabac_write_b, cabac_write);
  cabac_write_encode_buf(cabac_write_encode_b, cabac_write_encode);
  cabac_write_coding_tree_buf(cabac_write_coding_tree_b, cabac_write_coding_tree, stall_coding_tree);

  cabac_write_demux(cabac_write_hier, cabac_write_b, cabac_write_encode_b, cabac_write_coding_tree_b);
  cabac_write_dc_buf(cabac_write_hier, cabac_write_out);
}

void bin_coding(uint_8 cabac_states[256 * LCU_COUNT], ac_int< 32, false > low, ac_int< 16, false > range, ac_int< 8, false > buffered_byte, ac_int< 8, false > num_buffered_bytes,
                ac_int< 8, false > bits_left, uint_8 stream_data, uint_8 stream_cur_bit, uint_8 stream_zercount, cabac_chan &cabac_write, ac_channel< ac_int< 5, false > > &ctu_done,
                uint_8 bitstream_last_data[KVZ_DATA_CHUNK_SIZE * LCU_COUNT], ac_int< 128, false > cabac_metas[LCU_COUNT * 2]) {
  static ac_channel< ac_int< 16 + 1, false > > bs_data;
  static ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > cabac_write_command1;
  static ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > cabac_write_command2;
  static ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > cabac_write_command3;
  static ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > cabac_write_command4;
  static ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > bitstream_put_byte1;
  static ac_channel< ac_int< 32 + 16 + 8 + 8 + 8 + 8 + 8 + 5 + 2, false > > bitstream_put_byte2;
  static cabac_chan cabac_write_hier;

  cabac_write_main_state(cabac_states, cabac_write, cabac_write_hier);
  cabac_write_main_cmd(low, range, buffered_byte, num_buffered_bytes, bits_left, stream_data, stream_cur_bit, stream_zercount, cabac_write_hier, cabac_write_command1);

  cabac_low_shift_add_buf(cabac_write_command1, cabac_write_command2);
  cabac_bits(cabac_write_command2, cabac_write_command3);
  cabac_low(cabac_write_command3, cabac_write_command4);
  kvz_cabac_write(cabac_write_command4, bitstream_put_byte1);
  kvz_bitstream_put_byte_buf(bitstream_put_byte1, bitstream_put_byte2);
  kvz_bitstream_put_byte(bitstream_put_byte2, bs_data, ctu_done, cabac_metas);

  cabac_write_bitstream(bitstream_last_data, bs_data);
}
