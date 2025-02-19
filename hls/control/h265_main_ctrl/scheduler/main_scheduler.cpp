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
#include "intra.h"
#include "tables.h"
#include "types.h"

#pragma hls_design
void scheduler_conf_in(ctu_init_t::port_t &conf_in, ctu_init_t::port_t &conf_out) {
#ifndef __SYNTHESIS__
  while (conf_in.available(1)) {
#endif
    conf_out.write(conf_in.read());
#ifndef __SYNTHESIS__
  }
#endif
}

#pragma hls_design
void scheduler_inst_done(ac_channel< inst_exec_t > &inst_done, ac_channel< inst_exec_t > &inst_done_out) {
#ifndef __SYNTHESIS__
  while (inst_done.available(1)) {
#endif
    inst_done_out.write(inst_done.read());
#ifndef __SYNTHESIS__
  }
#endif
}

#pragma hls_design
void scheduler_hier(ctu_init_t::port_t &conf_in, ac_channel< inst_exec_t > &inst_done, inst_t::ac_t inst[LCU_COUNT][CTU_INST_STACK_SIZE], exec_port_t &inst_out) {
  static inst_t prev_inst;
  static struct inst_stack_t {
    pc_t pc;
    inst_t inst;
    ctu_init_t conf;
  } stack[LCU_COUNT];

  one_bit conf_valid = conf_in.size() > 0;
  one_bit inst_done_valid = inst_done.size() > 0;

  static struct {
    one_bit valid[LCU_COUNT];
    uint_3 priority[LCU_COUNT];
  } state;

  if (conf_valid) {
    ctu_init_t conf = conf_in.read();
    inst_stack_t &cur_stack = stack[conf.lcu_id];

    state.valid[conf.lcu_id] = 1;

    cur_stack.pc = 0;
    cur_stack.conf = conf;
    cur_stack.inst = inst_t(conf.use_preset ? inst_presets[conf.inst_set][0] : inst[conf.lcu_id][0]);
  } else if (inst_done_valid) {
    inst_exec_t conf = inst_done.read();
    inst_stack_t &cur_stack = stack[conf.lcu_id];

    state.valid[conf.lcu_id] = 1;

    cur_stack.pc = conf.skip ? (pc_t)cur_stack.inst.skip : (pc_t)(cur_stack.pc + 1);
    cur_stack.inst = inst_t(cur_stack.conf.use_preset ? inst_presets[cur_stack.conf.inst_set][cur_stack.pc] : inst[conf.lcu_id][cur_stack.pc]);
  }

set_priority:
#pragma hls_unroll yes
  for (uint_6 i = 0; i < LCU_COUNT; ++i) {
    int_3 distance = stack[i].inst.depth - prev_inst.depth;
    uint_3 priority;

    switch (distance) {
    case 0:
      priority = 7;
      break;
    case 1:
      priority = 6;
      break;
    case 2:
      priority = 5;
      break;
    case 3:
      priority = 4;
      break;
    case -1:
      priority = 3;
      break;
    case -2:
      priority = 2;
      break;
    case -3:
      priority = 1;
      break;
    }

    state.priority[i] = state.valid[i] ? priority : uint_3(0);
  }

  // Find valid instruction with highest priority and send it forward
  {
    uint_4 next_id;
    ctl::Max< LCU_COUNT >(state.priority, next_id);

    if (state.valid[next_id]) {
      state.valid[next_id] = 0;

      inst_t cur_inst = prev_inst = stack[next_id].inst;

      // Write selected instruction
      inst_out.write(cur_inst.toExec(next_id));
    }
  }
}

#pragma hls_design top
void scheduler(ctu_init_t::port_t &conf_in, ac_channel< inst_exec_t > &inst_done, inst_t::ac_t inst[LCU_COUNT][CTU_INST_STACK_SIZE], exec_port_t &inst_out) {
  static ctu_init_t::port_t conf_hier;
  static ac_channel< inst_exec_t > inst_done_hier;

  scheduler_conf_in(conf_in, conf_hier);
  scheduler_inst_done(inst_done, inst_done_hier);
  scheduler_hier(conf_hier, inst_done_hier, inst, inst_out);
}
