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
#include "tables.h"
#include "types.h"
#include <time.h>

#include <mc_scverify.h>

inst_t::ac_t golden_luma_off_frame[] = {
    0x00194000, 0x00068004, 0x00020008, 0x0001000c, 0x0001404c, 0x0001840c, 0x0001c44c, 0x00020009, 0x00038088, 0x0002808c, 0x0002c0cc, 0x0003048c, 0x000344cc, 0x00038089, 0x00050808,
    0x0004080c, 0x0004484c, 0x00048c0c, 0x0004cc4c, 0x00050809, 0x00064888, 0x0005888c, 0x0005c8cc, 0x00060c8c, 0x00064ccc, 0x00068005, 0x000cc104, 0x00084108, 0x0007410c, 0x0007814c,
    0x0007c50c, 0x0008054c, 0x00084109, 0x0009c188, 0x0008c18c, 0x000901cc, 0x0009458c, 0x000985cc, 0x0009c189, 0x000b4908, 0x000a490c, 0x000a894c, 0x000acd0c, 0x000b0d4c, 0x000b4909,
    0x000c8988, 0x000bc98c, 0x000c09cc, 0x000c4d8c, 0x000c8dcc, 0x000cc105, 0x00131004, 0x000e9008, 0x000d900c, 0x000dd04c, 0x000e140c, 0x000e544c, 0x000e9009, 0x00101088, 0x000f108c,
    0x000f50cc, 0x000f948c, 0x000fd4cc, 0x00101089, 0x00119808, 0x0010980c, 0x0010d84c, 0x00111c0c, 0x00115c4c, 0x00119809, 0x0012d888, 0x0012188c, 0x001258cc, 0x00129c8c, 0x0012dccc,
    0x00131005, 0x00191104, 0x0014d108, 0x0013d10c, 0x0014114c, 0x0014550c, 0x0014954c, 0x0014d109, 0x00165188, 0x0015518c, 0x001591cc, 0x0015d58c, 0x001615cc, 0x00165189, 0x0017d908,
    0x0016d90c, 0x0017194c, 0x00175d0c, 0x00179d4c, 0x0017d909, 0x00191988, 0x0018598c, 0x001899cc, 0x0018dd8c, 0x00191dcc, 0x00194001, 0x00000003,
};

void scheduler(ctu_init_t::port_t &conf_in, ac_channel< inst_exec_t > &inst_done, inst_t::ac_t inst[LCU_COUNT][CTU_INST_STACK_SIZE], exec_port_t &inst_out);

CCS_MAIN(int argc, char *argv[]) {
  srand(time(NULL));

  ctu_init_t::port_t conf_in;
  ac_channel< inst_exec_t > inst_done;
  inst_t::ac_t inst[LCU_COUNT][CTU_INST_STACK_SIZE];
  exec_port_t inst_out;

  int simulation_successful = 1;

  // Testing scheduling of preset 0
  {
    ctu_init_t ctu_conf;

    ctu_conf.lcu_id = rand() % 16;
    ctu_conf.inst_set = 0;
    ctu_conf.use_preset = 1;

    conf_in.write(ctu_conf);

    for (int a = 0; a < 5; a++) {
      if (a != 0) {
        inst_exec_t inst_done_tmp;
        inst_done_tmp.lcu_id = ctu_conf.lcu_id;
        inst_done_tmp.skip = 0;
        inst_done.write(inst_done_tmp);
      }
      CCS_DESIGN(scheduler)(conf_in, inst_done, inst, inst_out);

      exec_t exec = inst_out.read();
      inst_t inst = inst_presets[ctu_conf.inst_set][a];
      exec_t exec_cmp = inst.toExec(ctu_conf.lcu_id);
      if (exec.lcu_id != exec_cmp.lcu_id || exec.depth != exec_cmp.depth || exec.color != exec_cmp.color || exec.x != exec_cmp.x || exec.y != exec_cmp.y || exec.op_code != exec_cmp.op_code) {
        printf("Error: Preset 0, depths 1-1!\n");
        simulation_successful = 0;
        break;
      }
    }
  }

  if (simulation_successful == 0) {
    goto ERR;
  }

  // Testing scheduling of preset 3
  {
    ctu_init_t ctu_conf;

    ctu_conf.lcu_id = rand() % 16;
    ctu_conf.inst_set = 3;
    ctu_conf.use_preset = 1;

    conf_in.write(ctu_conf);

    for (int a = 0; a < 405; a++) {
      if (a != 0) {
        inst_exec_t inst_done_tmp;
        inst_done_tmp.lcu_id = ctu_conf.lcu_id;
        inst_done_tmp.skip = 0;
        inst_done.write(inst_done_tmp);
      }
      CCS_DESIGN(scheduler)(conf_in, inst_done, inst, inst_out);

      exec_t exec = inst_out.read();
      inst_t inst = inst_presets[ctu_conf.inst_set][a];
      exec_t exec_cmp = inst.toExec(ctu_conf.lcu_id);
      if (exec.lcu_id != exec_cmp.lcu_id || exec.depth != exec_cmp.depth || exec.color != exec_cmp.color || exec.x != exec_cmp.x || exec.y != exec_cmp.y || exec.op_code != exec_cmp.op_code) {
        printf("Error: Preset 3, depths 1-4!\n");
        simulation_successful = 0;
        break;
      }
    }
  }

  if (simulation_successful == 0) {
    goto ERR;
  }

  // Testing scheduling of preset 0 with random skipping
  {
    ctu_init_t ctu_conf;

    ctu_conf.lcu_id = rand() % 16;
    ctu_conf.inst_set = 3;
    ctu_conf.use_preset = 1;

    conf_in.write(ctu_conf);

    for (int a = 0; a < 405; a++) {
      if (a != 0) {
        inst_exec_t inst_done_tmp;
        inst_done_tmp.lcu_id = ctu_conf.lcu_id;
        inst_done_tmp.skip = rand() % 2;
        inst_done.write(inst_done_tmp);

        if (inst_done_tmp.skip) {
          inst_t inst = inst_presets[ctu_conf.inst_set][a - 1];
          a = inst.skip;
        }
      }
      CCS_DESIGN(scheduler)(conf_in, inst_done, inst, inst_out);

      exec_t exec = inst_out.read();
      inst_t inst = inst_presets[ctu_conf.inst_set][a];
      exec_t exec_cmp = inst.toExec(ctu_conf.lcu_id);
      if (exec.lcu_id != exec_cmp.lcu_id || exec.depth != exec_cmp.depth || exec.color != exec_cmp.color || exec.x != exec_cmp.x || exec.y != exec_cmp.y || exec.op_code != exec_cmp.op_code) {
        printf("Error: Preset 3, depths 1-4 with random skipping!\n");
        simulation_successful = 0;
        break;
      }
    }
  }

  if (simulation_successful == 0) {
    goto ERR;
  }

  // Testing scheduling from inst memory, i.e. not using a preset
  {
    ctu_init_t ctu_conf;

    ctu_conf.lcu_id = rand() % 16;
    ctu_conf.inst_set = 0;
    ctu_conf.use_preset = 0;

    for (int a = 0; a < sizeof(golden_luma_off_frame) / 4; a++) {
      inst[ctu_conf.lcu_id][a] = golden_luma_off_frame[a];
    }

    conf_in.write(ctu_conf);

    for (int a = 0; a < sizeof(golden_luma_off_frame) / 4; a++) {
      if (a != 0) {
        inst_exec_t inst_done_tmp;
        inst_done_tmp.lcu_id = ctu_conf.lcu_id;
        inst_done_tmp.skip = rand() % 2;
        inst_done.write(inst_done_tmp);

        if (inst_done_tmp.skip) {
          inst_t inst = golden_luma_off_frame[a - 1];
          a = inst.skip;
        }
      }
      CCS_DESIGN(scheduler)(conf_in, inst_done, inst, inst_out);

      exec_t exec = inst_out.read();
      inst_t inst = golden_luma_off_frame[a];
      exec_t exec_cmp = inst.toExec(ctu_conf.lcu_id);
      if (exec.lcu_id != exec_cmp.lcu_id || exec.depth != exec_cmp.depth || exec.color != exec_cmp.color || exec.x != exec_cmp.x || exec.y != exec_cmp.y || exec.op_code != exec_cmp.op_code) {
        printf("Error: Scheduling of generated instructions with random skipping!\n");
        simulation_successful = 0;
        break;
      }
    }
  }

  if (simulation_successful == 0) {
    goto ERR;
  }

ERR:

  if (simulation_successful) {
    cout << "## SIMULATION PASSED ##" << endl;
  } else {
    cout << "## SIMULATION FAILED ##" << endl;
  }

  CCS_RETURN(0);
}
