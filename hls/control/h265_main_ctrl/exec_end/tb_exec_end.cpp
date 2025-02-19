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
#include "types.h"
#include <cstring>
#include <time.h>

#include <mc_scverify.h>

void exec_end(exec_port_t &inst_input, ac_channel< int_192 > &result_in, ctu_end_conf_t::ac_t ctu_end[LCU_COUNT], cu_info_mem_t cu[LCU_COUNT], ac_int< 128 > cabac[LCU_COUNT],
              ac_channel< ac_int< 17 + 5 + 1, false > > &buffer);

typedef uint_32 acc_cu_info_t;
static uint_8 cu_get_depth(acc_cu_info_t cu) { return ((cu >> 6) & 0x03) + 1; };

two_bit dselect_cu(ctu_stack_t::ac_t *ctu_stack, two_bit str_depth, two_bit end_depth, ctu_stack_t &selected_cu) {
  two_bit selected_depth = str_depth;

dselect_cu:
  for (two_bit i = 0; i < 3; ++i) {
    two_bit cur_depth = str_depth - 1 - i;
    two_bit top_depth = str_depth - 2 - i;

    ctu_stack_t cur_cu(ctu_stack[cur_depth]);
    ctu_stack_t top_cu(ctu_stack[top_depth]);

    // Disable split if smaller CUs were skipped
    bool no_split = !cur_cu.split_valid;

    // Split if split_cost is smaller
    fixed_t diff = cur_cu.cost - cur_cu.split_cost;

    std::cout << "Depth: " << cur_depth << " Cost: " << cur_cu.cost << " Split: " << cur_cu.split_cost << " Diff: " << diff << " InValid: " << no_split << std::endl;

    if (no_split || diff <= 0) {
      selected_cu = cur_cu;
      selected_depth = cur_depth;
    } else {
      top_cu.split_cost -= diff;
    }

    ctu_stack[top_depth] = top_cu.toInt();
    if (cur_depth == end_depth)
      break;
  }

  return selected_depth;
}

void generate_inst_result(int depth, int min_depth, int max_depth, int x, int y, int lcu_id, exec_port_t &inst_input, ac_channel< int_192 > &result_in, bool chroma_recon) {
  exec_t inst;
  inst.lcu_id = lcu_id;
  inst.color = chroma_recon ? COLOR_ALL : COLOR_Y;
  inst.x = x;
  inst.y = y;

  if (depth != max_depth + 1) {
    inst.depth = depth;
    inst.op_code = CMP;

    generate_inst_result(depth + 1, min_depth, max_depth, x, y, lcu_id, inst_input, result_in, chroma_recon);
    if (!chroma_recon) {
      inst_input.write(inst);
    }

    generate_inst_result(depth + 1, min_depth, max_depth, x + (64 >> depth), y, lcu_id, inst_input, result_in, chroma_recon);
    if (!chroma_recon) {
      inst_input.write(inst);
    }

    generate_inst_result(depth + 1, min_depth, max_depth, x, y + (64 >> depth), lcu_id, inst_input, result_in, chroma_recon);
    if (!chroma_recon) {
      inst_input.write(inst);
    }

    generate_inst_result(depth + 1, min_depth, max_depth, x + (64 >> depth), y + (64 >> depth), lcu_id, inst_input, result_in, chroma_recon);
    if (depth == min_depth && !chroma_recon) {
      inst_input.write(inst);
    }
  }

  if (depth == min_depth) {
    return;
  }

  inst.op_code = IP;
  inst.depth = depth - 1;

  inst_input.write(inst);

  int_192 cost_input = 0;
  cost_input.set_slc(8, (uint8)((x + y) * depth) % 35); // intra mode

  cost_input.set_slc(16, (uint_26)((x + depth) % 3)); // ssd
  cost_input.set_slc(42, (uint_26)((y + depth) % 4)); // ssd
  cost_input.set_slc(68, (uint_26)((y + depth) % 5)); // ssd

  cost_input.set_slc(96, (uint_26)((y + depth) % 5));  // coeff_bits
  cost_input.set_slc(128, (uint_26)((x + depth) % 4)); // coeff_bits
  cost_input.set_slc(160, (uint_26)((x + depth) % 3)); // coeff_bits

  if (chroma_recon) {
    result_in.write(cost_input);
  }
  result_in.write(cost_input);
}

// Generated golden output for buffer channel: Depths 0-3, CTU not on edge
int golden_buffer_data[] = {
    0x0600030, 0x0400000, 0x0600230, 0x0400000, 0x0602030, 0x0400000, 0x0602230, 0x0000020, 0x0400020, 0x0600430, 0x0400000, 0x0600630, 0x0400000, 0x0602430, 0x0400000, 0x0602630, 0x0000420,
    0x0400420, 0x0604030, 0x0400000, 0x0604230, 0x0400000, 0x0606030, 0x0400000, 0x0606230, 0x0004020, 0x0404020, 0x0604430, 0x0400000, 0x0604630, 0x0400000, 0x0606430, 0x0400000, 0x0606630,
    0x0004420, 0x0000010, 0x0400010, 0x0600830, 0x0400000, 0x0600a30, 0x0400000, 0x0602830, 0x0400000, 0x0602a30, 0x0000820, 0x0400820, 0x0600c30, 0x0400000, 0x0600e30, 0x0400000, 0x0602c30,
    0x0400000, 0x0602e30, 0x0000c20, 0x0400c20, 0x0604830, 0x0400000, 0x0604a30, 0x0400000, 0x0606830, 0x0400000, 0x0606a30, 0x0004820, 0x0404820, 0x0604c30, 0x0400000, 0x0604e30, 0x0400000,
    0x0606c30, 0x0400000, 0x0606e30, 0x0004c20, 0x0000810, 0x0400810, 0x0608030, 0x0400000, 0x0608230, 0x0400000, 0x060a030, 0x0400000, 0x060a230, 0x0608020, 0x0408020, 0x0608430, 0x0400000,
    0x0608630, 0x0408020, 0x060a430, 0x0408020, 0x060a630, 0x0608420, 0x0408420, 0x060c030, 0x000c030, 0x060c230, 0x0408420, 0x060e030, 0x0408420, 0x060e230, 0x000c020, 0x040c020, 0x060c430,
    0x0400810, 0x060c630, 0x0400810, 0x060e430, 0x0400810, 0x060e630, 0x000c420, 0x0008010, 0x0408010, 0x0608830, 0x0400000, 0x0608a30, 0x0400000, 0x060a830, 0x0400000, 0x060aa30, 0x0608820,
    0x0408820, 0x0608c30, 0x0008c30, 0x0608e30, 0x0408820, 0x060ac30, 0x0408820, 0x060ae30, 0x0608c20, 0x0408c20, 0x060c830, 0x000c830, 0x060ca30, 0x0408c20, 0x060e830, 0x0408c20, 0x060ea30,
    0x000c820, 0x040c820, 0x060cc30, 0x0408010, 0x060ce30, 0x0408010, 0x060ec30, 0x0408010, 0x060ee30, 0x000cc20, 0x0008810, 0x0000000, 0x0400000, 0x0600030, 0x0400000, 0x0600230, 0x0400000,
    0x0602030, 0x0400000, 0x0602230, 0x0000020, 0x0400020, 0x0600430, 0x0400000, 0x0600630, 0x0400000, 0x0602430, 0x0400000, 0x0602630, 0x0000420, 0x0400420, 0x0604030, 0x0400000, 0x0604230,
    0x0400000, 0x0606030, 0x0400000, 0x0606230, 0x0004020, 0x0404020, 0x0604430, 0x0400000, 0x0604630, 0x0400000, 0x0606430, 0x0400000, 0x0606630, 0x0004420, 0x0000010, 0x0400010, 0x0600830,
    0x0400010, 0x0600a30, 0x0400010, 0x0602830, 0x0400010, 0x0602a30, 0x0000820, 0x0400820, 0x0600c30, 0x0000c30, 0x0600e30, 0x0400820, 0x0602c30, 0x0400820, 0x0602e30, 0x0000c20, 0x0400c20,
    0x0604830, 0x0400010, 0x0604a30, 0x0400010, 0x0606830, 0x0400010, 0x0606a30, 0x0004820, 0x0404820, 0x0604c30, 0x0400010, 0x0604e30, 0x0400010, 0x0606c30, 0x0400010, 0x0606e30, 0x0004c20,
    0x0000810, 0x0400810, 0x0608030, 0x0400810, 0x0608230, 0x0400810, 0x060a030, 0x0400810, 0x060a230, 0x0608020, 0x0408020, 0x0608430, 0x0008430, 0x0608630, 0x0408020, 0x060a430, 0x0408020,
    0x060a630, 0x0608420, 0x0408420, 0x060c030, 0x0400810, 0x060c230, 0x0400810, 0x060e030, 0x0400810, 0x060e230, 0x000c020, 0x040c020, 0x060c430, 0x0400810, 0x060c630, 0x0400810, 0x060e430,
    0x0400810, 0x060e630, 0x000c420, 0x0008010, 0x0408010, 0x0608830, 0x0408010, 0x0608a30, 0x0408010, 0x060a830, 0x0408010, 0x060aa30, 0x0608820, 0x0408820, 0x0608c30, 0x0008c30, 0x0608e30,
    0x0408820, 0x060ac30, 0x0408820, 0x060ae30, 0x0608c20, 0x0408c20, 0x060c830, 0x000c830, 0x060ca30, 0x0408c20, 0x060e830, 0x0408c20, 0x060ea30, 0x000c820, 0x040c820, 0x060cc30, 0x0408010,
    0x060ce30, 0x0408010, 0x060ec30, 0x0408010, 0x060ee30, 0x000cc20, 0x0008810, 0x0000000, 0x0400000, 0x0600030, 0x0400000, 0x0600230, 0x0400000, 0x0602030, 0x0400000, 0x0602230, 0x0000020,
    0x0400020, 0x0600430, 0x0400000, 0x0600630, 0x0400000, 0x0602430, 0x0400000, 0x0602630, 0x0000420, 0x0400420, 0x0604030, 0x0400000, 0x0604230, 0x0400000, 0x0606030, 0x0400000, 0x0606230,
    0x0004020, 0x0404020, 0x0604430, 0x0400000, 0x0604630, 0x0400000, 0x0606430, 0x0400000, 0x0606630, 0x0004420, 0x0000010, 0x0400010, 0x0600830, 0x0400010, 0x0600a30, 0x0400010, 0x0602830,
    0x0400010, 0x0602a30, 0x0000820, 0x0400820, 0x0600c30, 0x0000c30, 0x0600e30, 0x0400820, 0x0602c30, 0x0400820, 0x0602e30, 0x0000c20, 0x0400c20, 0x0604830, 0x0004830, 0x0604a30, 0x0400c20,
    0x0606830, 0x0400c20, 0x0606a30, 0x0004820, 0x0404820, 0x0604c30, 0x0400010, 0x0604e30, 0x0400010, 0x0606c30, 0x0400010, 0x0606e30, 0x0004c20, 0x0000810, 0x0400810, 0x0608030, 0x0400810,
    0x0608230, 0x0400810, 0x060a030, 0x0400810, 0x060a230, 0x0008020, 0x0408020, 0x0608430, 0x0008430, 0x0608630, 0x0008630, 0x060a430, 0x0408020, 0x060a630, 0x0008420, 0x0408420, 0x060c030,
    0x000c030, 0x060c230, 0x000c230, 0x060e030, 0x0408420, 0x060e230, 0x060c020, 0x040c020, 0x060c430, 0x0400810, 0x060c630, 0x0400810, 0x060e430, 0x0400810, 0x060e630, 0x060c420, 0x0008010,
    0x0408010, 0x0608830, 0x0408010, 0x0608a30, 0x0408010, 0x060a830, 0x0408010, 0x060aa30, 0x0008820, 0x0408820, 0x0608c30, 0x0008c30, 0x0608e30, 0x0408820, 0x060ac30, 0x0408820, 0x060ae30,
    0x0008c20, 0x0408c20, 0x060c830, 0x0408010, 0x060ca30, 0x0408010, 0x060e830, 0x0408010, 0x060ea30, 0x060c820, 0x040c820, 0x060cc30, 0x0408010, 0x060ce30, 0x0408010, 0x060ec30, 0x0408010,
    0x060ee30, 0x060cc20, 0x0008810, 0x0600000, 0x0400000, 0x0600030, 0x0400000, 0x0600230, 0x0400000, 0x0602030, 0x0400000, 0x0602230, 0x0000020, 0x0400020, 0x0600430, 0x0400000, 0x0600630,
    0x0400000, 0x0602430, 0x0400000, 0x0602630, 0x0000420, 0x0400420, 0x0604030, 0x0400000, 0x0604230, 0x0400000, 0x0606030, 0x0400000, 0x0606230, 0x0004020, 0x0404020, 0x0604430, 0x0400000,
    0x0604630, 0x0400000, 0x0606430, 0x0400000, 0x0606630, 0x0004420, 0x0000010, 0x0400010, 0x0600830, 0x0400010, 0x0600a30, 0x0400010, 0x0602830, 0x0400010, 0x0602a30, 0x0000820, 0x0400820,
    0x0600c30, 0x0000c30, 0x0600e30, 0x0400820, 0x0602c30, 0x0400820, 0x0602e30, 0x0000c20, 0x0400c20, 0x0604830, 0x0004830, 0x0604a30, 0x0400c20, 0x0606830, 0x0400c20, 0x0606a30, 0x0004820,
    0x0404820, 0x0604c30, 0x0400010, 0x0604e30, 0x0400010, 0x0606c30, 0x0400010, 0x0606e30, 0x0004c20, 0x0000810, 0x0400810, 0x0608030, 0x0400810, 0x0608230, 0x0400810, 0x060a030, 0x0400810,
    0x060a230, 0x0008020, 0x0408020, 0x0608430, 0x0008430, 0x0608630, 0x0008630, 0x060a430, 0x0408020, 0x060a630, 0x0008420, 0x0408420, 0x060c030, 0x000c030, 0x060c230, 0x000c230, 0x060e030,
    0x0408420, 0x060e230, 0x060c020, 0x040c020, 0x060c430, 0x0400810, 0x060c630, 0x0400810, 0x060e430, 0x0400810, 0x060e630, 0x060c420, 0x0008010, 0x0408010, 0x0608830, 0x0408010, 0x0608a30,
    0x0408010, 0x060a830, 0x0408010, 0x060aa30, 0x0008820, 0x0408820, 0x0608c30, 0x0008c30, 0x0608e30, 0x0408820, 0x060ac30, 0x0408820, 0x060ae30, 0x0008c20, 0x0408c20, 0x060c830, 0x0408010,
    0x060ca30, 0x0408010, 0x060e830, 0x0408010, 0x060ea30, 0x060c820, 0x040c820, 0x060cc30, 0x0408010, 0x060ce30, 0x0408010, 0x060ec30, 0x0408010, 0x060ee30, 0x060cc20, 0x0008810, 0x0600000,
    0x0400000,
};

// Generated golden output for buffer channel: Depths 2-3, CTU on left edge
int golden_buffer_data2[] = {
    0x0600030, 0x0400000, 0x0600230, 0x0400000, 0x0602030, 0x0400000, 0x0602230, 0x0000020, 0x0400020, 0x0600430, 0x0400000, 0x0600630,
    0x0400000, 0x0602430, 0x0400000, 0x0602630, 0x0000420, 0x0400420, 0x0604030, 0x0400000, 0x0604230, 0x0400000, 0x0606030, 0x0400000,
    0x0606230, 0x0004020, 0x0404020, 0x0604430, 0x0400000, 0x0604630, 0x0400000, 0x0606430, 0x0400000, 0x0606630, 0x0004420, 0x0404420,
};

// Generated golden output for buffer channel: Depths 0-1, CTU on left and top edge
int golden_buffer_data3[] = {
    0x0520019, 0x0520029, 0x0520819, 0x0520029, 0x0528019, 0x0520029, 0x0528819, 0x0120009, 0x0520009, 0x0520019, 0x0520029, 0x0520819,
    0x0520029, 0x0528019, 0x0520029, 0x0528819, 0x0120009, 0x0520009, 0x0520019, 0x0520029, 0x0520819, 0x0520029, 0x0528019, 0x0520029,
    0x0528819, 0x0720009, 0x0520009, 0x0520019, 0x0520029, 0x0520819, 0x0520029, 0x0528019, 0x0520029, 0x0528819, 0x0720009, 0x0520009,
};

// Generated golden output for buffer channel: Depths 0-3, chroma recon
int golden_buffer_data4[] = {
    0x04600f3, 0x04602f3, 0x04620f3, 0x04622f3, 0x04600e3, 0x04604f3, 0x04606f3, 0x04624f3, 0x04626f3, 0x04604e3, 0x04640f3, 0x04642f3, 0x04660f3, 0x04662f3, 0x04640e3, 0x04644f3, 0x04646f3,
    0x04664f3, 0x04666f3, 0x04644e3, 0x04600d3, 0x04608f3, 0x0460af3, 0x04628f3, 0x0462af3, 0x04608e3, 0x0460cf3, 0x0460ef3, 0x0462cf3, 0x0462ef3, 0x0460ce3, 0x04648f3, 0x0464af3, 0x04668f3,
    0x0466af3, 0x04648e3, 0x0464cf3, 0x0464ef3, 0x0466cf3, 0x0466ef3, 0x0464ce3, 0x04608d3, 0x04680f3, 0x04682f3, 0x046a0f3, 0x046a2f3, 0x04680e3, 0x04684f3, 0x04686f3, 0x046a4f3, 0x046a6f3,
    0x04684e3, 0x046c0f3, 0x046c2f3, 0x046e0f3, 0x046e2f3, 0x046c0e3, 0x046c4f3, 0x046c6f3, 0x046e4f3, 0x046e6f3, 0x046c4e3, 0x04680d3, 0x04688f3, 0x0468af3, 0x046a8f3, 0x046aaf3, 0x04688e3,
    0x0468cf3, 0x0468ef3, 0x046acf3, 0x046aef3, 0x0468ce3, 0x046c8f3, 0x046caf3, 0x046e8f3, 0x046eaf3, 0x046c8e3, 0x046ccf3, 0x046cef3, 0x046ecf3, 0x046eef3, 0x046cce3, 0x04688d3, 0x04600c3,
    0x04600f3, 0x04602f3, 0x04620f3, 0x04622f3, 0x04600e3, 0x04604f3, 0x04606f3, 0x04624f3, 0x04626f3, 0x04604e3, 0x04640f3, 0x04642f3, 0x04660f3, 0x04662f3, 0x04640e3, 0x04644f3, 0x04646f3,
    0x04664f3, 0x04666f3, 0x04644e3, 0x04600d3, 0x04608f3, 0x0460af3, 0x04628f3, 0x0462af3, 0x04608e3, 0x0460cf3, 0x0460ef3, 0x0462cf3, 0x0462ef3, 0x0460ce3, 0x04648f3, 0x0464af3, 0x04668f3,
    0x0466af3, 0x04648e3, 0x0464cf3, 0x0464ef3, 0x0466cf3, 0x0466ef3, 0x0464ce3, 0x04608d3, 0x04680f3, 0x04682f3, 0x046a0f3, 0x046a2f3, 0x04680e3, 0x04684f3, 0x04686f3, 0x046a4f3, 0x046a6f3,
    0x04684e3, 0x046c0f3, 0x046c2f3, 0x046e0f3, 0x046e2f3, 0x046c0e3, 0x046c4f3, 0x046c6f3, 0x046e4f3, 0x046e6f3, 0x046c4e3, 0x04680d3, 0x04688f3, 0x0468af3, 0x046a8f3, 0x046aaf3, 0x04688e3,
    0x0468cf3, 0x0468ef3, 0x046acf3, 0x046aef3, 0x0468ce3, 0x046c8f3, 0x046caf3, 0x046e8f3, 0x046eaf3, 0x046c8e3, 0x046ccf3, 0x046cef3, 0x046ecf3, 0x046eef3, 0x046cce3, 0x04688d3, 0x04600c3,
    0x04600f3, 0x04602f3, 0x04620f3, 0x04622f3, 0x04600e3, 0x04604f3, 0x04606f3, 0x04624f3, 0x04626f3, 0x04604e3, 0x04640f3, 0x04642f3, 0x04660f3, 0x04662f3, 0x04640e3, 0x04644f3, 0x04646f3,
    0x04664f3, 0x04666f3, 0x04644e3, 0x04600d3, 0x04608f3, 0x0460af3, 0x04628f3, 0x0462af3, 0x04608e3, 0x0460cf3, 0x0460ef3, 0x0462cf3, 0x0462ef3, 0x0460ce3, 0x04648f3, 0x0464af3, 0x04668f3,
    0x0466af3, 0x04648e3, 0x0464cf3, 0x0464ef3, 0x0466cf3, 0x0466ef3, 0x0464ce3, 0x04608d3, 0x04680f3, 0x04682f3, 0x046a0f3, 0x046a2f3, 0x04680e3, 0x04684f3, 0x04686f3, 0x046a4f3, 0x046a6f3,
    0x04684e3, 0x046c0f3, 0x046c2f3, 0x046e0f3, 0x046e2f3, 0x046c0e3, 0x046c4f3, 0x046c6f3, 0x046e4f3, 0x046e6f3, 0x046c4e3, 0x04680d3, 0x04688f3, 0x0468af3, 0x046a8f3, 0x046aaf3, 0x04688e3,
    0x0468cf3, 0x0468ef3, 0x046acf3, 0x046aef3, 0x0468ce3, 0x046c8f3, 0x046caf3, 0x046e8f3, 0x046eaf3, 0x046c8e3, 0x046ccf3, 0x046cef3, 0x046ecf3, 0x046eef3, 0x046cce3, 0x04688d3, 0x04600c3,
    0x04600f3, 0x04602f3, 0x04620f3, 0x04622f3, 0x04600e3, 0x04604f3, 0x04606f3, 0x04624f3, 0x04626f3, 0x04604e3, 0x04640f3, 0x04642f3, 0x04660f3, 0x04662f3, 0x04640e3, 0x04644f3, 0x04646f3,
    0x04664f3, 0x04666f3, 0x04644e3, 0x04600d3, 0x04608f3, 0x0460af3, 0x04628f3, 0x0462af3, 0x04608e3, 0x0460cf3, 0x0460ef3, 0x0462cf3, 0x0462ef3, 0x0460ce3, 0x04648f3, 0x0464af3, 0x04668f3,
    0x0466af3, 0x04648e3, 0x0464cf3, 0x0464ef3, 0x0466cf3, 0x0466ef3, 0x0464ce3, 0x04608d3, 0x04680f3, 0x04682f3, 0x046a0f3, 0x046a2f3, 0x04680e3, 0x04684f3, 0x04686f3, 0x046a4f3, 0x046a6f3,
    0x04684e3, 0x046c0f3, 0x046c2f3, 0x046e0f3, 0x046e2f3, 0x046c0e3, 0x046c4f3, 0x046c6f3, 0x046e4f3, 0x046e6f3, 0x046c4e3, 0x04680d3, 0x04688f3, 0x0468af3, 0x046a8f3, 0x046aaf3, 0x04688e3,
    0x0468cf3, 0x0468ef3, 0x046acf3, 0x046aef3, 0x0468ce3, 0x046c8f3, 0x046caf3, 0x046e8f3, 0x046eaf3, 0x046c8e3, 0x046ccf3, 0x046cef3, 0x046ecf3, 0x046eef3, 0x046cce3, 0x04688d3, 0x04600c3,
};

CCS_MAIN(int argc, char *argv[]) {
  static ctu_stack_t::ac_t stack[LCU_COUNT][4];

  // Input channels
  static exec_port_t inst_input;
  static ac_channel< int_192 > result_in;
  // Output channels
  static ac_channel< ac_int< 17 + 5 + 1, false > > buffer;

  // Configs
  static ctu_end_conf_t::ac_t ctu_end[LCU_COUNT];

  // Data
  static cu_info_mem_t cu[LCU_COUNT];
  static ac_int< 128 > cabac_mem[LCU_COUNT];

  int simulation_successful = 1;

  // Initialization of memories
  for (int b = 0; b < 256; b++) {
    cu_info_t temp_info;
    temp_info.set_cu_info(b % 35, b % 35, b % 4, 0, 0);
    cu_simple_info_t temp(temp_info);
    for (int a = 0; a < 16; a++) {
      cu[a].cu[b] = temp_info;
    }
  }

  for (int b = 0; b < 16; b++) {
    cu_info_t temp_info;
    temp_info.set_cu_info((b + 1) % 35, (b + 1) % 35, (b + 1) % 4, 0, 0);
    cu_simple_info_t temp(temp_info);

    cu_info_t temp_info2;
    temp_info2.set_cu_info((b + 2) % 3, (b + 2) % 3, (b + 2) % 4, 0, 0);
    cu_simple_info_t temp2(temp_info2);
    for (int a = 0; a < 16; a++) {
      cu[a].cu_hor[b] = temp;
      cu[a].cu_ver[b] = temp2;
    }
  }

  for (int i = 0; i < 12; ++i) {
    for (int a = 0; a < 16; a++) {
      cabac_mem[a].set_slc(8 * i, (cabac_ctx_t)(i + 1));
    }
  }

  // Testing depths 0-3, CTU not on edge
  {
    int lcu_id = rand() % 16;

    // Config
    ctu_end_conf_t ctu_conf;
    ctu_conf.lambda = 16;
    ctu_conf.max_depth = 3;
    ctu_conf.min_depth = 0;
    ctu_conf.trskip_enable = 0;
    ctu_conf.x_zero = 0;
    ctu_conf.y_zero = 0;

    for (int a = 0; a < 16; a++) {
      ctu_end[a] = ctu_conf.toInt();
    }

    generate_inst_result(ctu_conf.min_depth.to_uint(), ctu_conf.min_depth.to_uint(), ctu_conf.max_depth.to_uint(), 0, 0, lcu_id, inst_input, result_in, false);

    while (inst_input.available(1)) {
      CCS_DESIGN(exec_end)
      (inst_input, result_in, ctu_end, cu, cabac_mem, buffer);
    }

    while (buffer.available(1)) {
      static int counter = 0;
      ac_int< 17 + 5 + 1, false > temp = buffer.read();

      if (0) {
        static int a = 0;
        printf("0x%07x", temp.to_uint());
        if ((a + 1) % 8 == 0) {
          printf(",\n");
        } else {
          printf(", ");
        }
        a++;
      }

      // Insert correct lcu_id in golden data
      if ((((golden_buffer_data[counter++] & 0xffe1fff0) | lcu_id) | lcu_id << 17) != (uint_32)(temp.to_uint())) {
        simulation_successful = 0;
        printf("Error: Depths 0-3, CTU not on edge\n");
        goto ERR;
      }
    }
  }

  // Testing depths 2-3, CTU on left edge
  {
    int lcu_id = rand() % 16;

    // Config
    ctu_end_conf_t ctu_conf;
    ctu_conf.lambda = 41;
    ctu_conf.max_depth = 3;
    ctu_conf.min_depth = 2;
    ctu_conf.trskip_enable = 0;
    ctu_conf.x_zero = 1;
    ctu_conf.y_zero = 0;

    for (int a = 0; a < 16; a++) {
      ctu_end[a] = ctu_conf.toInt();
    }

    generate_inst_result(ctu_conf.min_depth.to_uint(), ctu_conf.min_depth.to_uint(), ctu_conf.max_depth.to_uint(), 0, 0, lcu_id, inst_input, result_in, false);

    while (inst_input.available(1)) {
      CCS_DESIGN(exec_end)
      (inst_input, result_in, ctu_end, cu, cabac_mem, buffer);
    }

    while (buffer.available(1)) {
      static int counter = 0;
      ac_int< 17 + 5 + 1, false > temp = buffer.read();

      if (0) {
        static int a = 0;
        printf("0x%07x", temp.to_uint());
        if ((a + 1) % 8 == 0) {
          printf(",\n");
        } else {
          printf(", ");
        }
        a++;
      }

      // Insert correct lcu_id in golden data
      if ((((golden_buffer_data2[counter++] & 0xffe1fff0) | lcu_id) | lcu_id << 17) != (uint_32)(temp.to_uint())) {
        simulation_successful = 0;
        printf("Error: Depths 2-3, CTU on left edge\n");
        goto ERR;
      }
    }
  }

  // Testing depths 0-1, CTU on left and top edge
  {
    int lcu_id = rand() % 16;

    // Config
    ctu_end_conf_t ctu_conf;
    ctu_conf.lambda = 32;
    ctu_conf.max_depth = 1;
    ctu_conf.min_depth = 0;
    ctu_conf.trskip_enable = 0;
    ctu_conf.x_zero = 1;
    ctu_conf.y_zero = 1;

    for (int a = 0; a < 16; a++) {
      ctu_end[a] = ctu_conf.toInt();
    }

    generate_inst_result(ctu_conf.min_depth.to_uint(), ctu_conf.min_depth.to_uint(), ctu_conf.max_depth.to_uint(), 0, 0, lcu_id, inst_input, result_in, false);

    while (inst_input.available(1)) {
      CCS_DESIGN(exec_end)
      (inst_input, result_in, ctu_end, cu, cabac_mem, buffer);
    }

    while (buffer.available(1)) {
      static int counter = 0;
      ac_int< 17 + 5 + 1, false > temp = buffer.read();

      if (0) {
        static int a = 0;
        printf("0x%07x", temp.to_uint());
        if ((a + 1) % 8 == 0) {
          printf(",\n");
        } else {
          printf(", ");
        }
        a++;
      }

      // Insert correct lcu_id in golden data
      if ((((golden_buffer_data3[counter++] & 0xffe1fff0) | lcu_id) | lcu_id << 17) != (uint_32)(temp.to_uint())) {
        simulation_successful = 0;
        printf("Error: Depths 0-1, CTU on left and top edge\n");
        goto ERR;
      }
    }
  }

  // Testing depths 0-3, chroma recon
  {
    int lcu_id = rand() % 16;

    // Config
    ctu_end_conf_t ctu_conf;
    ctu_conf.lambda = 4;
    ctu_conf.max_depth = 3;
    ctu_conf.min_depth = 0;
    ctu_conf.trskip_enable = 0;
    ctu_conf.x_zero = 0;
    ctu_conf.y_zero = 0;

    for (int a = 0; a < 16; a++) {
      ctu_end[a] = ctu_conf.toInt();
    }

    generate_inst_result(ctu_conf.min_depth.to_uint(), ctu_conf.min_depth.to_uint(), ctu_conf.max_depth.to_uint(), 0, 0, lcu_id, inst_input, result_in, true);

    while (inst_input.available(1)) {
      CCS_DESIGN(exec_end)
      (inst_input, result_in, ctu_end, cu, cabac_mem, buffer);
    }

    while (buffer.available(1)) {
      static int counter = 0;
      ac_int< 17 + 5 + 1, false > temp = buffer.read();

      if (0) {
        static int a = 0;
        printf("0x%07x", temp.to_uint());
        if ((a + 1) % 8 == 0) {
          printf(",\n");
        } else {
          printf(", ");
        }
        a++;
      }

      // Insert correct lcu_id in golden data
      if ((((golden_buffer_data4[counter++] & 0xffe1fff0) | lcu_id) | lcu_id << 17) != (uint_32)(temp.to_uint())) {
        simulation_successful = 0;
        printf("Error: Depths 0-3, chroma recon\n");
        goto ERR;
      }
    }
  }

ERR:

  if (simulation_successful) {
    cout << "## SIMULATION PASSED ##" << endl;
  } else {
    cout << "## SIMULATION FAILED ##" << endl;
  }

  CCS_RETURN(0);
}
