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

#include <mc_scverify.h>

void init_ctu(ac_channel< int_32 > &conf_in, ctu_init_t::port_t &scheduler, inst_t::ac_t inst[LCU_COUNT][CTU_INST_STACK_SIZE], ctu_str_conf_t::ac_t ctu_str[LCU_COUNT],
              ctu_end_conf_t::ac_t ctu_end[LCU_COUNT], cu_simple_info_t cu_feedback[LCU_COUNT][256], ac_channel< ac_int< 44 + 4, false > > &init_chroma_in, one_bit conf_in_lz, one_bit init_chroma_lz);

void op_code_to_str(two_bit op) { std::cout << (op == IP ? "IP " : op == CMP ? "CMP" : op == STR ? "STR" : op == END ? "END" : "ERROR"); }

void get_instruction_set(inst_t::ac_t *inst) {
  pc_t pc = 0;
  inst_t cur_inst(inst[pc]);

  for (int i = 0; i < 512; ++i) {
    printf("0x%08x, ", cur_inst.toInt());

    if (cur_inst.op_code == END)
      break;

    if (i % 8 == 7)
      printf("\n");

    cur_inst = inst_t(inst[++pc]);
  }

  std::cout << std::endl;
}

int golden_luma_off_frame[] = {
    0x00194000, 0x00068004, 0x00020008, 0x0001000c, 0x0001404c, 0x0001840c, 0x0001c44c, 0x00020009, 0x00038088, 0x0002808c, 0x0002c0cc, 0x0003048c, 0x000344cc, 0x00038089, 0x00050808,
    0x0004080c, 0x0004484c, 0x00048c0c, 0x0004cc4c, 0x00050809, 0x00064888, 0x0005888c, 0x0005c8cc, 0x00060c8c, 0x00064ccc, 0x00068005, 0x000cc104, 0x00084108, 0x0007410c, 0x0007814c,
    0x0007c50c, 0x0008054c, 0x00084109, 0x0009c188, 0x0008c18c, 0x000901cc, 0x0009458c, 0x000985cc, 0x0009c189, 0x000b4908, 0x000a490c, 0x000a894c, 0x000acd0c, 0x000b0d4c, 0x000b4909,
    0x000c8988, 0x000bc98c, 0x000c09cc, 0x000c4d8c, 0x000c8dcc, 0x000cc105, 0x00131004, 0x000e9008, 0x000d900c, 0x000dd04c, 0x000e140c, 0x000e544c, 0x000e9009, 0x00101088, 0x000f108c,
    0x000f50cc, 0x000f948c, 0x000fd4cc, 0x00101089, 0x00119808, 0x0010980c, 0x0010d84c, 0x00111c0c, 0x00115c4c, 0x00119809, 0x0012d888, 0x0012188c, 0x001258cc, 0x00129c8c, 0x0012dccc,
    0x00131005, 0x00191104, 0x0014d108, 0x0013d10c, 0x0014114c, 0x0014550c, 0x0014954c, 0x0014d109, 0x00165188, 0x0015518c, 0x001591cc, 0x0015d58c, 0x001615cc, 0x00165189, 0x0017d908,
    0x0016d90c, 0x0017194c, 0x00175d0c, 0x00179d4c, 0x0017d909, 0x00191988, 0x0018598c, 0x001899cc, 0x0018dd8c, 0x00191dcc, 0x00194001, 0x00000003,
};

int golden_chroma_recon[] = {
    0x0000403c,
    0x000080b4,
    0x0000c1b4,
    0x000110b4,
    0x000151b4,
    0x000182b4,
    0x0001c3b4,
    0x000212b4,
    0x000253b4,
    0x0002a0b4,
    0x0002e1b4,
    0x000330b4,
    0x000371b4,
    0x0003a2b4,
    0x0003e3b4,
    0x000432b4,
    0x000473b4,
    0x00000003,
};

int golden_chroma_recon2[] = {
    0x0000403c, 0x000080bc, 0x0000c83c, 0x000108bc, 0x0001413c, 0x000181bc, 0x0001c93c, 0x000209bc, 0x0002503c, 0x000290bc, 0x0002d83c, 0x000318bc, 0x0003513c,
    0x000391bc, 0x0003d93c, 0x000419bc, 0x0004423c, 0x000482bc, 0x0004ca3c, 0x00050abc, 0x0005433c, 0x000583bc, 0x0005cb3c, 0x00060bbc, 0x0006523c, 0x000692bc,
    0x0006da3c, 0x00071abc, 0x0007533c, 0x000793bc, 0x0007db3c, 0x00081bbc, 0x0008603c, 0x0008a0bc, 0x0008e83c, 0x000928bc, 0x0009613c, 0x0009a1bc, 0x0009e93c,
    0x000a29bc, 0x000a703c, 0x000ab0bc, 0x000af83c, 0x000b38bc, 0x000b713c, 0x000bb1bc, 0x000bf93c, 0x000c39bc, 0x000c623c, 0x000ca2bc, 0x000cea3c, 0x000d2abc,
    0x000d633c, 0x000da3bc, 0x000deb3c, 0x000e2bbc, 0x000e723c, 0x000eb2bc, 0x000efa3c, 0x000f3abc, 0x000f733c, 0x000fb3bc, 0x000ffb3c, 0x00103bbc, 0x00000003,
};

CCS_MAIN(int argc, char *argv[]) {
  static ac_channel< int_32 > conf_in;
  static ctu_init_t::port_t scheduler;
  static ac_channel< uint_4 > ref_ctrl;

  static ctu_str_conf_t::ac_t ctu_str[LCU_COUNT];
  static ctu_end_conf_t::ac_t ctu_end[LCU_COUNT];
  static inst_t::ac_t inst[LCU_COUNT][CTU_INST_STACK_SIZE];

  static cu_simple_info_t cu_feedback[LCU_COUNT][256];
  static ac_channel< ac_int< 44 + 4, false > > init_chroma_in;

  {
    int a = 0;
    int b = 0;
    for (a = 0; a < 256; a++) {
      cu_info_t temp_info;
      temp_info.set_cu_info(0, 0, (a + 3) % 4, 0, 0);
      cu_simple_info_t temp(temp_info);
      cu_feedback[0][a] = temp;
    }
  }
  int_8 acc_id = 0;

  uint_32 x = 0, y = 0;

  // Frame width and height are always a multiple of 8, rounded up
  uint_32 frame_width = 416;
  uint_32 frame_height = 240;

  int min_depths[10] = {0, 0, 0, 0, 1, 1, 1, 2, 2, 3};
  int max_depths[10] = {0, 1, 2, 3, 1, 2, 3, 2, 3, 3};

  int simulation_successful = 1;

  // Check that correct preset is chosen
  for (int a = 0; a < 10; a++) {
    int_8 min_depth = min_depths[a];
    int_8 max_depth = max_depths[a];

    one_bit acc_trskip = 0;
    one_bit acc_chroma = 1;

    uint_32 first_conf = acc_id;
    first_conf |= (uint_32)(min_depth) << 4;
    first_conf |= (uint_32)(max_depth) << 6;
    first_conf |= uint_32(acc_trskip) << 8; // TR Skip Enable
    first_conf |= uint_32(acc_chroma) << 9; // Chroma Enable
    first_conf |= (uint_32)(MIN((uint_32)(frame_width - x), (uint_32)96)) << 16;
    first_conf |= (uint_32)(x == 0) << 23;
    first_conf |= (uint_32)(MIN((uint_32)(frame_height - y), (uint_32)96)) << 24;
    first_conf |= (uint_32)(y == 0) << 31;

    uint_32 quant_conf = 0;

    conf_in.write(first_conf);
    conf_in.write(quant_conf);

    CCS_DESIGN(init_ctu)
    (conf_in, scheduler, inst, ctu_str, ctu_end, cu_feedback, init_chroma_in, (one_bit)1, (one_bit)0);

    ctu_init_t ctu_conf = scheduler.read();
    inst_t cur_inst(ctu_conf.use_preset ? inst_presets[ctu_conf.inst_set][0] : inst[ctu_conf.lcu_id][0]);

    if (ctu_conf.inst_set != a || ctu_conf.use_preset != 1) {
      printf("Error: selecting preset\n");
      simulation_successful = 0;
      break;
    }
  }

  if (simulation_successful == 0) {
    goto ERR;
  }

  // Checking instruction generation for luma when partly off frame
  {

    int_8 min_depth = 0;
    int_8 max_depth = 3;

    one_bit acc_trskip = 0;
    one_bit acc_chroma = 1;

    uint_32 first_conf = acc_id;
    first_conf |= (uint_32)(min_depth) << 4;
    first_conf |= (uint_32)(max_depth) << 6;
    first_conf |= uint_32(acc_trskip) << 8; // TR Skip Enable
    first_conf |= uint_32(acc_chroma) << 9; // Chroma Enable
    first_conf |= (uint_32)(MIN((uint_32)(32), (uint_32)96)) << 16;
    first_conf |= (uint_32)(x == 0) << 23;
    first_conf |= (uint_32)(MIN((uint_32)(32), (uint_32)96)) << 24;
    first_conf |= (uint_32)(y == 0) << 31;

    uint_32 quant_conf = 0;

    conf_in.write(first_conf);
    conf_in.write(quant_conf);

    CCS_DESIGN(init_ctu)
    (conf_in, scheduler, inst, ctu_str, ctu_end, cu_feedback, init_chroma_in, (one_bit)1, (one_bit)0);

    ctu_init_t ctu_conf = scheduler.read();
    inst_t cur_inst(ctu_conf.use_preset ? inst_presets[ctu_conf.inst_set][0] : inst[ctu_conf.lcu_id][0]);

    for (int a = 0; a < sizeof(golden_luma_off_frame) / 4; a++) {
      if (golden_luma_off_frame[a] != inst[acc_id][a]) {
        printf("Error: luma partly off frame\n");
        simulation_successful = 0;
        break;
      }
    }
  }

  if (simulation_successful == 0) {
    goto ERR;
  }

  // 1st Checking instruction generation for chroma
  {
    int_8 min_depth = 0;
    int_8 max_depth = 3;

    one_bit acc_trskip = 0;
    one_bit acc_chroma = 1;

    uint_32 first_conf = acc_id;
    first_conf |= (uint_32)(min_depth) << 4;
    first_conf |= (uint_32)(max_depth) << 6;
    first_conf |= uint_32(acc_trskip) << 8; // TR Skip Enable
    first_conf |= uint_32(acc_chroma) << 9; // Chroma Enable
    first_conf |= (uint_32)(MIN((uint_32)(frame_width - x), (uint_32)96)) << 16;
    first_conf |= (uint_32)(x == 0) << 23;
    first_conf |= (uint_32)(MIN((uint_32)(frame_height - y), (uint_32)96)) << 24;
    first_conf |= (uint_32)(y == 0) << 31;

    uint_32 quant_conf = 0;

    conf_in.write(first_conf);
    conf_in.write(quant_conf);

    CCS_DESIGN(init_ctu)
    (conf_in, scheduler, inst, ctu_str, ctu_end, cu_feedback, init_chroma_in, (one_bit)1, (one_bit)0);

    ctu_init_t ctu_conf = scheduler.read();
    inst_t cur_inst(ctu_conf.use_preset ? inst_presets[ctu_conf.inst_set][0] : inst[ctu_conf.lcu_id][0]);

    ac_int< 44 + 4, false > init_chroma_temp = ctu_str[acc_id];
    init_chroma_temp <<= 4;
    init_chroma_temp |= 0;
    init_chroma_in.write(init_chroma_temp);

    CCS_DESIGN(init_ctu)
    (conf_in, scheduler, inst, ctu_str, ctu_end, cu_feedback, init_chroma_in, (one_bit)0, (one_bit)1);

    ctu_conf = scheduler.read();

    for (int a = 0; a < sizeof(golden_chroma_recon) / 4; a++) {
      if (golden_chroma_recon[a] != inst[acc_id][a]) {
        printf("Error: chroma reconstruction\n");
        simulation_successful = 0;
        break;
      }
    }
  }

  // 2nd Checking instruction generation for chroma
  {

    {
      int a = 0;
      int b = 0;
      for (a = 0; a < 256; a++) {
        cu_info_t temp_info;
        temp_info.set_cu_info(0, 0, 3, 0, 0);
        cu_simple_info_t temp(temp_info);
        cu_feedback[0][a] = temp;
      }
    }

    int_8 min_depth = 3;
    int_8 max_depth = 3;

    one_bit acc_trskip = 0;
    one_bit acc_chroma = 1;

    uint_32 first_conf = acc_id;
    first_conf |= (uint_32)(min_depth) << 4;
    first_conf |= (uint_32)(max_depth) << 6;
    first_conf |= uint_32(acc_trskip) << 8; // TR Skip Enable
    first_conf |= uint_32(acc_chroma) << 9; // Chroma Enable
    first_conf |= (uint_32)(MIN((uint_32)(frame_width - x), (uint_32)96)) << 16;
    first_conf |= (uint_32)(x == 0) << 23;
    first_conf |= (uint_32)(MIN((uint_32)(frame_height - y), (uint_32)96)) << 24;
    first_conf |= (uint_32)(y == 0) << 31;

    uint_32 quant_conf = 0;

    conf_in.write(first_conf);
    conf_in.write(quant_conf);

    CCS_DESIGN(init_ctu)
    (conf_in, scheduler, inst, ctu_str, ctu_end, cu_feedback, init_chroma_in, (one_bit)1, (one_bit)0);

    ctu_init_t ctu_conf = scheduler.read();
    inst_t cur_inst(ctu_conf.use_preset ? inst_presets[ctu_conf.inst_set][0] : inst[ctu_conf.lcu_id][0]);

    ac_int< 44 + 4, false > init_chroma_temp = ctu_str[acc_id];
    init_chroma_temp <<= 4;
    init_chroma_temp |= 0;
    init_chroma_in.write(init_chroma_temp);

    CCS_DESIGN(init_ctu)
    (conf_in, scheduler, inst, ctu_str, ctu_end, cu_feedback, init_chroma_in, (one_bit)0, (one_bit)1);

    ctu_conf = scheduler.read();

    for (int a = 0; a < sizeof(golden_chroma_recon) / 4; a++) {
      if (golden_chroma_recon2[a] != inst[acc_id][a]) {
        printf("Error: chroma reconstruction\n");
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
