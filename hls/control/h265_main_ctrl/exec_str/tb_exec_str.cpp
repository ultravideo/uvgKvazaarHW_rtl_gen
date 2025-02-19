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
#include <time.h>

#include <mc_scverify.h>

void exec_str(exec_port_t &inst_input, exec_port_t &inst_out, const ctu_str_conf_t::ac_t ctu_str[LCU_COUNT], const cu_info_str_mem_t cu[LCU_COUNT], ac_channel< int_48 > &ip_ctrl, irq_t *irq,
              ac_channel< ac_int< 44 + 4, false > > &init_chroma);

int_48 golden_data[] = {0x40401f080049, 0x00000901a040, 0x40401f080089, 0x00000901a040, 0xc03c20080116, 0x00000901a040, 0x40381f08026e, 0x00000909a040, 0x40381f0802ae, 0x00000909a040,
                        0xc03020080430, 0x000009000043, 0x40201f08085b, 0x000009200047, 0x40201f08089b, 0x000009200047, 0x3c402008101b, 0x00000901a001, 0x3c3c1f081149, 0x000009440050,
                        0x3c3c1f081189, 0x000009440050, 0x3c3820081233, 0x000009000091, 0x3c301f081465, 0x000009500113, 0x3c301f0814a5, 0x000009500113, 0x3c202008181a, 0x000009000217,
                        0x38c01f082050, 0x000009800401, 0x38c01f082090, 0x000009800401, 0x383c20082139, 0x000009000460, 0x38381f08224c, 0x0000098804a1, 0x38381f08228c, 0x0000098804a1,
                        0x38302008241b, 0x000009001500, 0x38201f082863, 0x000009140604, 0x38201f0828a3, 0x000009140604, 0x30c020084039, 0x000009000341};

CCS_MAIN(int argc, char *argv[]) {
  srand(time(NULL));

  exec_port_t inst_input;
  exec_port_t inst_out;
  ctu_str_conf_t::ac_t ctu_str[LCU_COUNT];
  cu_info_str_mem_t cu[LCU_COUNT];
  ac_channel< int_48 > ip_ctrl;
  irq_t irq;
  ac_channel< ac_int< 44 + 4, false > > init_chroma;

  int simulation_successful = 1;

  {
    for (int a = 0; a < 16; a++) {
      for (int b = 0; b < 256; b++) {
        cu_info_t temp_info;
        temp_info.set_cu_info(b % 35, b % 35, (b + 3) % 4, 0, 0);
        cu_simple_info_t temp(temp_info);
        cu[a].cu[b] = temp;
        if (b < 16) {
          cu[a].cu_hor[b] = temp;
        }
      }
    }
  }

  int offset[] = {0, 4, 8, 16, 32};

  // Test out different configs and sending configurations for intra prediction
  int golden_data_rd = 0;
  for (int a = 0; a < 16; a++) {
    int lcu_id = a;

    {
      ctu_str_conf_t conf;
      conf.min_depth = (a / 4) % 4;
      conf.max_depth = MAX(conf.min_depth, (ac_int< 2, false >)(a % 4));

      conf.x_zero = (a / 2) % 2;
      conf.x_limit = 64;
      conf.y_zero = a % 2;
      conf.y_limit = 64;

      conf.qp = 32;
      conf.cabac = 9;
      conf.lambda_sqrt = 8;

      ctu_str[lcu_id] = conf.toInt();
    }

    exec_t inst;
    inst.lcu_id = lcu_id;
    // Luma or both chromas
    inst.color = a % 2 ? 0 : 3;
    inst.depth = a % 4;
    if (inst.color != 0) {
      inst.depth = a % 3;
    }
    inst.op_code = IP;
    inst.x = offset[a % 5];
    inst.y = offset[(a / 5) % 5];

    inst_input.write(inst);

    CCS_DESIGN(exec_str)(inst_input, inst_out, ctu_str, cu, ip_ctrl, &irq, init_chroma);
    int_48 data = ip_ctrl.read();

    // Update random LCU id
    golden_data[golden_data_rd].set_slc(0, inst.lcu_id);
    if (data != golden_data[golden_data_rd++]) {
      simulation_successful = 0;
      if (inst.color == 3) {
        printf("Error: Intra prediction chroma U config 2 loop #%d\n", a);
      } else {
        printf("Error: Intra prediction luma config 1 loop #%d\n", a);
      }
      break;
    }

    data = ip_ctrl.read();

    if (data != golden_data[golden_data_rd++]) {
      simulation_successful = 0;
      if (inst.color == 3) {
        printf("Error Intra prediction chroma U config 2 loop #%d\n", a);
      } else {
        printf("Error Intra prediction luma config 2 loop #%d\n", a);
      }
      break;
    }

    if (inst.color == 3) {
      data = ip_ctrl.read();

      golden_data[golden_data_rd].set_slc(0, inst.lcu_id);
      if (data != golden_data[golden_data_rd++]) {
        simulation_successful = 0;
        printf("Error: Intra prediction chroma V config 1 loop #%d\n", a);
        break;
      }

      data = ip_ctrl.read();

      if (data != golden_data[golden_data_rd++]) {
        simulation_successful = 0;
        printf("Error: Intra prediction chroma V config 2 loop #%d\n", a);
        break;
      }
    }
  }

  if (simulation_successful == 0) {
    goto ERR;
  }

  // Testing chroma init and IRQ
  for (int a = 0; a < 16; a++) {
    int lcu_id = a;

    {
      ctu_str_conf_t conf;
      conf.min_depth = 0;
      conf.max_depth = conf.min_depth;

      conf.x_zero = 0;
      conf.x_limit = 64;
      conf.y_zero = 1;
      conf.y_limit = 64;

      conf.qp = 32;
      conf.cabac = 9;
      conf.lambda_sqrt = 8;

      ctu_str[lcu_id] = conf.toInt();
    }

    exec_t inst;
    inst.lcu_id = lcu_id;
    // Luma or both chromas
    inst.color = 0;
    inst.depth = 0;
    inst.op_code = END;
    inst.x = 0;
    inst.y = 0;

    inst_input.write(inst);

    CCS_DESIGN(exec_str)(inst_input, inst_out, ctu_str, cu, ip_ctrl, &irq, init_chroma);

    int_48 data = init_chroma.read();

    // Check conf in hex
    if (data != 0x0908200c0401 && irq != 0) {
      simulation_successful = 0;
      printf("Error: Chroma init\n");
      goto ERR;
    }

    inst_input.write(inst);

    CCS_DESIGN(exec_str)(inst_input, inst_out, ctu_str, cu, ip_ctrl, &irq, init_chroma);

    if (irq != 1 << lcu_id) {
      simulation_successful = 0;
      printf("Error: IRQ %u\n", irq);
      goto ERR;
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
