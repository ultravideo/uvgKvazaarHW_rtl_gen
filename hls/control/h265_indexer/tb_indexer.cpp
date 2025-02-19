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
#include <mc_scverify.h>
#include <queue>
#include <string.h>
#include <time.h>

using namespace std;

ac_int< MEM_WIDTH, false > ref_pixels[REF_SIZE * CTU_IDS];
ac_int< MEM_WIDTH, false > top_border[TOP_BORDER_SIZE * CTU_IDS];
ac_int< MEM_WIDTH, false > left_border[LEFT_BORDER_SIZE * CTU_IDS];
ac_int< MEM_WIDTH, false > cu_info_ver[CU_INFO_VER_SIZE * CTU_IDS];
ac_int< MEM_WIDTH, false > cu_info_ver_d1[CU_INFO_VER_SIZE * CTU_IDS / 2];
ac_int< MEM_WIDTH, false > cu_info_ver_d2[CU_INFO_VER_SIZE * CTU_IDS / 2];
ac_int< MEM_WIDTH, false > cu_info_ver_d1_top[CU_INFO_VER_SIZE * CTU_IDS / 2];
ac_int< MEM_WIDTH, false > cu_info_ver_d2_top[CU_INFO_VER_SIZE * CTU_IDS / 2];
ac_int< MEM_WIDTH, false > cu_info_hor[CU_INFO_HOR_SIZE * CTU_IDS];
ac_int< MEM_WIDTH, false > cu_info_hor_d1[CU_INFO_HOR_SIZE * CTU_IDS / 2];
ac_int< MEM_WIDTH, false > cu_info_hor_d2[CU_INFO_HOR_SIZE * CTU_IDS / 2];
ac_int< MEM_WIDTH, false > cu_info_hor_d1_top[CU_INFO_HOR_SIZE * CTU_IDS / 2];
ac_int< MEM_WIDTH, false > cu_info_hor_d2_top[CU_INFO_HOR_SIZE * CTU_IDS / 2];
ac_int< MEM_WIDTH, false > cu_info_hor_str[CU_INFO_HOR_SIZE * CTU_IDS];
ac_int< MEM_WIDTH, false > cabac[CABAC_SIZE * CTU_IDS];
ac_int< MEM_WIDTH, false > cabac_metas[CABAC_METAS_SIZE * CTU_IDS / 2];
ac_int< MEM_WIDTH, false > cabac_metas_top[CABAC_METAS_SIZE * CTU_IDS / 2];

void main_indexer_top(ac_channel< ac_int< MEM_WIDTH + DATA_IN_ADDR_WIDTH + CTU_ID_WIDTH, false > > &data_in, ac_int< MEM_WIDTH + 9, false > mem_out[REF_SIZE * CTU_IDS],
                      ac_int< MEM_WIDTH + 6, false > mem_out_btm_top[REF_SIZE * CTU_IDS], ac_int< MEM_WIDTH, false > cabac_states[CABAC_STATES_SIZE * CTU_IDS / 2],
                      ac_int< MEM_WIDTH, false > cabac_states_top[CABAC_STATES_SIZE * CTU_IDS / 2], ac_channel< ac_int< MEM_WIDTH, false > > &data_out,
                      ac_channel< ac_int< CTU_ID_WIDTH, false > > &data_req, ac_int< MEM_WIDTH, false > rec[REC_SIZE * CTU_IDS], ac_int< MEM_WIDTH, false > cu_info[CU_INFO_SIZE * CTU_IDS],
                      ac_int< MEM_WIDTH, false > cabac_metas2[CABAC_METAS_SIZE2 * CTU_IDS / 2], ac_int< MEM_WIDTH, false > cabac_metas2_top[CABAC_METAS_SIZE2 * CTU_IDS / 2],
                      ac_int< MEM_WIDTH, false > bitstream_last_data[BITSTREAM_SIZE * CTU_IDS / 2], ac_int< MEM_WIDTH, false > bitstream_last_data_top[BITSTREAM_SIZE * CTU_IDS / 2]);

CCS_MAIN(int argc, char *argv[]) {
  srand(time(NULL));

  static ac_channel< ac_int< MEM_WIDTH + DATA_IN_ADDR_WIDTH + CTU_ID_WIDTH, false > > data_in;

  static ac_int< MEM_WIDTH + 9, false > mem_out[REF_SIZE * CTU_IDS];
  static ac_int< MEM_WIDTH + 6, false > mem_out_btm_top[REF_SIZE * CTU_IDS];

  static ac_int< MEM_WIDTH, false > cabac_states[CABAC_STATES_SIZE * CTU_IDS / 2];
  static ac_int< MEM_WIDTH, false > cabac_states_top[CABAC_STATES_SIZE * CTU_IDS / 2];

  static ac_channel< ac_int< MEM_WIDTH, false > > data_out;
  static ac_channel< ac_int< CTU_ID_WIDTH, false > > data_req;

  static ac_int< MEM_WIDTH, false > rec[REC_SIZE * CTU_IDS];
  static ac_int< MEM_WIDTH, false > cu_info[CU_INFO_SIZE * CTU_IDS];
  static ac_int< MEM_WIDTH, false > cabac_metas2[CABAC_METAS_SIZE2 * CTU_IDS / 2];
  static ac_int< MEM_WIDTH, false > cabac_metas2_top[CABAC_METAS_SIZE2 * CTU_IDS / 2];
  static ac_int< MEM_WIDTH, false > bitstream_last_data[BITSTREAM_SIZE * CTU_IDS / 2];
  static ac_int< MEM_WIDTH, false > bitstream_last_data_top[BITSTREAM_SIZE * CTU_IDS / 2];

  int tests_passed = 1;

std:
  queue< int > golden_data;

  ac_int< MEM_WIDTH, false > data = 0;
  ac_int< DATA_IN_ADDR_WIDTH, false > addr = 0;
  ac_int< CTU_ID_WIDTH, false > ctu_id = rand() % 16;
  printf("CTU ID %d\n", ctu_id.to_int());
  for (int a = 0; a < REF_SIZE + TOP_BORDER_SIZE + LEFT_BORDER_SIZE + CU_INFO_VER_SIZE + CU_INFO_HOR_SIZE + CU_INFO_HOR_SIZE + CABAC_SIZE + CABAC_STATES_SIZE + CABAC_METAS_SIZE; a++) {
    ac_int< MEM_WIDTH + DATA_IN_ADDR_WIDTH + CTU_ID_WIDTH, false > data_send = data;
    data_send.set_slc(MEM_WIDTH, addr);
    data_send.set_slc(MEM_WIDTH + DATA_IN_ADDR_WIDTH, ctu_id);
    data++;
    addr += MEM_BYTES;
    data_in.write(data_send);
  }

  CCS_DESIGN(main_indexer_top)
  (data_in, mem_out, mem_out_btm_top, cabac_states, cabac_states_top, data_out, data_req, rec, cu_info, cabac_metas2, cabac_metas2_top, bitstream_last_data, bitstream_last_data_top);

  data = 0;
  for (int a = 0; a < REF_SIZE; a++) {
    if (ref_pixels[a + REF_SIZE * ctu_id] != data) {
      cout << "Error ref" << endl;
      tests_passed = 0;
      break;
    }
    data++;
  }

  if (!tests_passed) {
    goto error;
  }

  for (int a = 0; a < TOP_BORDER_SIZE; a++) {
    if (top_border[a + TOP_BORDER_SIZE * ctu_id] != data) {
      cout << "Error top_border" << endl;
      tests_passed = 0;
      break;
    }
    data++;
  }

  if (!tests_passed) {
    goto error;
  }

  for (int a = 0; a < LEFT_BORDER_SIZE; a++) {
    if (left_border[a + LEFT_BORDER_SIZE * ctu_id] != data) {
      cout << "Error left_border" << endl;
      tests_passed = 0;
      break;
    }
    data++;
  }

  if (!tests_passed) {
    goto error;
  }

  for (int a = 0; a < CU_INFO_VER_SIZE; a++) {
    ac_int< CTU_ID_WIDTH, false > ctu_id_top = ctu_id - 8;
    if (ctu_id < 8) {
      if (cu_info_ver[a + CU_INFO_VER_SIZE * ctu_id] != data || cu_info_ver_d1[a + CU_INFO_VER_SIZE * ctu_id] != data || cu_info_ver_d2[a + CU_INFO_VER_SIZE * ctu_id] != data) {
        cout << "Error cu_info_ver" << endl;
        tests_passed = 0;
        break;
      }
    } else {
      if (cu_info_ver[a + CU_INFO_VER_SIZE * ctu_id] != data || cu_info_ver_d1_top[a + CU_INFO_VER_SIZE * ctu_id_top] != data || cu_info_ver_d2_top[a + CU_INFO_VER_SIZE * ctu_id_top] != data) {
        cout << "Error cu_info_ver" << endl;
        tests_passed = 0;
        break;
      }
    }
    data++;
  }

  if (!tests_passed) {
    goto error;
  }

  for (int a = 0; a < CU_INFO_HOR_SIZE; a++) {
    ac_int< CTU_ID_WIDTH, false > ctu_id_top = ctu_id - 8;
    if (ctu_id < 8) {
      if (cu_info_hor[a + CU_INFO_HOR_SIZE * ctu_id] != data || cu_info_hor_d1[a + CU_INFO_HOR_SIZE * ctu_id] != data || cu_info_hor_d2[a + CU_INFO_HOR_SIZE * ctu_id] != data) {
        cout << "Error cu_info_hor" << endl;
        tests_passed = 0;
        break;
      }
    } else {
      if (cu_info_hor[a + CU_INFO_HOR_SIZE * ctu_id] != data || cu_info_hor_d1_top[a + CU_INFO_HOR_SIZE * ctu_id_top] != data || cu_info_hor_d2_top[a + CU_INFO_HOR_SIZE * ctu_id_top] != data) {
        cout << "Error cu_info_hor" << endl;
        tests_passed = 0;
        break;
      }
    }
    data++;
  }

  if (!tests_passed) {
    goto error;
  }

  for (int a = 0; a < CU_INFO_HOR_SIZE; a++) {
    if (cu_info_hor_str[a + CU_INFO_HOR_SIZE * ctu_id] != data) {
      cout << "Error cu_info_hor_str" << endl;
      tests_passed = 0;
      break;
    }
    data++;
  }

  if (!tests_passed) {
    goto error;
  }

  for (int a = 0; a < CABAC_SIZE; a++) {
    if (cabac[a + CABAC_SIZE * ctu_id] != data) {
      cout << "Error cabac" << endl;
      tests_passed = 0;
      break;
    }
    data++;
  }

  if (!tests_passed) {
    goto error;
  }

  for (int a = 0; a < CABAC_STATES_SIZE; a++) {
    ac_int< CTU_ID_WIDTH, false > ctu_id_top = ctu_id - 8;
    if (ctu_id < 8) {
      if (cabac_states[a + CABAC_STATES_SIZE * ctu_id] != data) {
        cout << "Error cabac_states" << endl;
        tests_passed = 0;
        break;
      }
    } else {
      if (cabac_states_top[a + CABAC_STATES_SIZE * ctu_id_top] != data) {
        cout << "Error cabac_states" << endl;
        tests_passed = 0;
        break;
      }
    }

    data++;
  }

  if (!tests_passed) {
    goto error;
  }

  for (int a = 0; a < CABAC_METAS_SIZE; a++) {
    ac_int< CTU_ID_WIDTH, false > ctu_id_top = ctu_id - 8;
    if (ctu_id < 8) {
      if (cabac_metas[a + CABAC_METAS_SIZE * ctu_id] != data) {
        cout << "Error cabac_metas" << endl;
        tests_passed = 0;
        break;
      }
    } else {
      if (cabac_metas_top[a + CABAC_METAS_SIZE * ctu_id_top] != data) {
        cout << "Error cabac_metas" << endl;
        tests_passed = 0;
        break;
      }
    }

    data++;
  }

  for (int a = 0; a < REC_SIZE; a++) {
    rec[a + REC_SIZE * ctu_id] = a;
    golden_data.push(a);
  }

  for (int a = 0; a < CABAC_STATES_SIZE; a++) {
    ac_int< CTU_ID_WIDTH, false > ctu_id_top = ctu_id - 8;
    if (ctu_id < 8) {
      cabac_states[a + CABAC_STATES_SIZE * ctu_id] = a;
    } else {
      cabac_states_top[a + CABAC_STATES_SIZE * ctu_id_top] = a;
    }
    golden_data.push(a);
  }

  for (int a = 0; a < CU_INFO_SIZE; a++) {
    cu_info[a + CU_INFO_SIZE * ctu_id] = a;
    golden_data.push(a);
  }

  for (int a = 0; a < CABAC_METAS_SIZE2; a++) {
    ac_int< CTU_ID_WIDTH, false > ctu_id_top = ctu_id - 8;
    if (ctu_id < 8) {
      cabac_metas2[a + CABAC_METAS_SIZE2 * ctu_id] = a;
    } else {
      cabac_metas2_top[a + CABAC_METAS_SIZE2 * ctu_id_top] = a;
    }
    golden_data.push(a);
  }

  for (int a = 0; a < BITSTREAM_SIZE; a++) {
    ac_int< CTU_ID_WIDTH, false > ctu_id_top = ctu_id - 8;
    if (ctu_id < 8) {
      bitstream_last_data[a + BITSTREAM_SIZE * ctu_id] = a;
    } else {
      bitstream_last_data_top[a + BITSTREAM_SIZE * ctu_id_top] = a;
    }
    golden_data.push(a);
  }

  data_req.write(ctu_id);

  CCS_DESIGN(main_indexer_top)
  (data_in, mem_out, mem_out_btm_top, cabac_states, cabac_states_top, data_out, data_req, rec, cu_info, cabac_metas2, cabac_metas2_top, bitstream_last_data, bitstream_last_data_top);

  CCS_DESIGN(main_indexer_top)
  (data_in, mem_out, mem_out_btm_top, cabac_states, cabac_states_top, data_out, data_req, rec, cu_info, cabac_metas2, cabac_metas2_top, bitstream_last_data, bitstream_last_data_top);

  while (data_out.available(1)) {
    ac_int< MEM_WIDTH, false > temp = data_out.read();
    int test = temp.slc< 32 >(0).to_int();
    int golden = golden_data.front();
    golden_data.pop();

    if (golden != test) {
      cout << "Error data_out" << endl;
      tests_passed = 0;
      break;
    }
  }

error:

  if (tests_passed) {
    cout << "## SIMULATION PASSED ##" << endl;
  } else {
    cout << "## SIMULATION FAILED ##" << endl;
  }

  CCS_RETURN(0);
}
