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

#pragma hls_noglobals
#pragma hls_design block
void main_indexer_in(ac_channel< ac_int< MEM_WIDTH + DATA_IN_ADDR_WIDTH + CTU_ID_WIDTH, false > > &data_in, ac_int< MEM_WIDTH + 9, false > mem_out[REF_SIZE * CTU_IDS],
                     ac_int< MEM_WIDTH + 6, false > mem_out_btm_top[REF_SIZE * CTU_IDS], ac_channel< ac_int< MEM_WIDTH + DATA_IN_ADDR_WIDTH + CTU_ID_WIDTH, false > > &cabac_states_push) {
#ifndef __SYNTHESIS__
  while (data_in.available(1)) {
#endif

#ifndef __SYNTHESIS__
    CHECK_INTEGER(REF_OFFSET);
    CHECK_INTEGER(TOP_BORDER_OFFSET);
    CHECK_INTEGER(LEFT_BORDER_OFFSET);
    CHECK_INTEGER(CU_INFO_VER_OFFSET);
    CHECK_INTEGER(CU_INFO_HOR_OFFSET);
    CHECK_INTEGER(CU_INFO_HOR_STR_OFFSET);
    CHECK_INTEGER(CABAC_OFFSET);
    CHECK_INTEGER(CABAC_STATES_OFFSET);
    CHECK_INTEGER(CABAC_METAS_OFFSET);
#endif

    ac_int< MEM_WIDTH + DATA_IN_ADDR_WIDTH + CTU_ID_WIDTH, false > temp = data_in.read();
    ac_int< MEM_WIDTH + 9, false > data = temp.slc< MEM_WIDTH >(0);
    ac_int< DATA_IN_ADDR_WIDTH, false > addr = temp.slc< DATA_IN_ADDR_WIDTH >(MEM_WIDTH) / MEM_BYTES;
    ac_int< CTU_ID_WIDTH, false > ctu_id = temp.slc< CTU_ID_WIDTH >(MEM_WIDTH + DATA_IN_ADDR_WIDTH);
    ac_int< CTU_ID_WIDTH, false > ctu_id_top = ctu_id - 8;

    ac_int< 24, false > addr_out;

    ac_int< MEM_WIDTH + 6, false > data2 = data;
    ac_int< 24, false > addr_out2;
    ac_int< 1, false > top_btm = 0;
    ac_int< 1, false > cabac_push = 0;

    if (addr < REF_OFFSET) {
#ifndef __SYNTHESIS__
      ref_pixels[ctu_id * REF_SIZE + addr] = data;
#endif

      data[MEM_WIDTH] = 1;
      addr_out = ctu_id * REF_SIZE + addr;
    } else if (addr < TOP_BORDER_OFFSET) {
#ifndef __SYNTHESIS__
      top_border[ctu_id * TOP_BORDER_SIZE + (addr - REF_OFFSET)] = data;
#endif

      data[MEM_WIDTH + 1] = 1;
      addr_out = ctu_id * TOP_BORDER_SIZE + (addr - REF_OFFSET);
    } else if (addr < LEFT_BORDER_OFFSET) {
#ifndef __SYNTHESIS__
      left_border[ctu_id * LEFT_BORDER_SIZE + (addr - TOP_BORDER_OFFSET)] = data;
#endif

      data[MEM_WIDTH + 2] = 1;
      addr_out = ctu_id * LEFT_BORDER_SIZE + (addr - TOP_BORDER_OFFSET);
    } else if (addr < CU_INFO_VER_OFFSET) {
#ifndef __SYNTHESIS__
      cu_info_ver[ctu_id * CU_INFO_VER_SIZE + (addr - LEFT_BORDER_OFFSET)] = data;
#endif

      data[MEM_WIDTH + 3] = 1;
      addr_out = ctu_id * CU_INFO_VER_SIZE + (addr - LEFT_BORDER_OFFSET);

      top_btm = 1;
      if (ctu_id < 8) {
#ifndef __SYNTHESIS__
        cu_info_ver_d1[ctu_id * CU_INFO_VER_SIZE + (addr - LEFT_BORDER_OFFSET)] = data;
        cu_info_ver_d2[ctu_id * CU_INFO_VER_SIZE + (addr - LEFT_BORDER_OFFSET)] = data;
#endif

        data2[MEM_WIDTH] = 1;
        addr_out2 = ctu_id * CU_INFO_VER_SIZE + (addr - LEFT_BORDER_OFFSET);
      } else {
#ifndef __SYNTHESIS__
        cu_info_ver_d1_top[ctu_id_top * CU_INFO_VER_SIZE + (addr - LEFT_BORDER_OFFSET)] = data;
        cu_info_ver_d2_top[ctu_id_top * CU_INFO_VER_SIZE + (addr - LEFT_BORDER_OFFSET)] = data;
#endif

        data2[MEM_WIDTH + 1] = 1;
        addr_out2 = ctu_id_top * CU_INFO_VER_SIZE + (addr - LEFT_BORDER_OFFSET);
      }
    } else if (addr < CU_INFO_HOR_OFFSET) {
#ifndef __SYNTHESIS__
      cu_info_hor[ctu_id * CU_INFO_HOR_SIZE + (addr - CU_INFO_VER_OFFSET)] = data;
#endif

      data[MEM_WIDTH + 4] = 1;
      addr_out = ctu_id * CU_INFO_HOR_SIZE + (addr - CU_INFO_VER_OFFSET);

      top_btm = 1;
      if (ctu_id < 8) {
#ifndef __SYNTHESIS__
        cu_info_hor_d1[ctu_id * CU_INFO_HOR_SIZE + (addr - CU_INFO_VER_OFFSET)] = data;
        cu_info_hor_d2[ctu_id * CU_INFO_HOR_SIZE + (addr - CU_INFO_VER_OFFSET)] = data;
#endif

        data2[MEM_WIDTH + 2] = 1;
        addr_out2 = ctu_id * CU_INFO_HOR_SIZE + (addr - CU_INFO_VER_OFFSET);
      } else {
#ifndef __SYNTHESIS__
        cu_info_hor_d1_top[ctu_id_top * CU_INFO_HOR_SIZE + (addr - CU_INFO_VER_OFFSET)] = data;
        cu_info_hor_d2_top[ctu_id_top * CU_INFO_HOR_SIZE + (addr - CU_INFO_VER_OFFSET)] = data;
#endif

        data2[MEM_WIDTH + 3] = 1;
        addr_out2 = ctu_id_top * CU_INFO_HOR_SIZE + (addr - CU_INFO_VER_OFFSET);
      }
    } else if (addr < CU_INFO_HOR_STR_OFFSET) {
#ifndef __SYNTHESIS__
      cu_info_hor_str[ctu_id * CU_INFO_HOR_SIZE + (addr - CU_INFO_HOR_OFFSET)] = data;
#endif

      data[MEM_WIDTH + 5] = 1;
      addr_out = ctu_id * CU_INFO_HOR_SIZE + (addr - CU_INFO_HOR_OFFSET);
    } else if (addr < CABAC_OFFSET) {
#ifndef __SYNTHESIS__
      cabac[ctu_id * CABAC_SIZE + (addr - CU_INFO_HOR_STR_OFFSET)] = data;
#endif

      data[MEM_WIDTH + 6] = 1;
      addr_out = ctu_id * CABAC_SIZE + (addr - CU_INFO_HOR_STR_OFFSET);
    } else if (addr < CABAC_STATES_OFFSET) {
      cabac_push = 1;
    } else if (addr < CABAC_METAS_OFFSET) {
      if (ctu_id < 8) {
#ifndef __SYNTHESIS__
        cabac_metas[ctu_id * CABAC_METAS_SIZE + (addr - CABAC_STATES_OFFSET)] = data;
#endif

        data[MEM_WIDTH + 7] = 1;
        addr_out = ctu_id * CABAC_METAS_SIZE + (addr - CABAC_STATES_OFFSET);
      } else {
#ifndef __SYNTHESIS__
        cabac_metas_top[ctu_id_top * CABAC_METAS_SIZE + (addr - CABAC_STATES_OFFSET)] = data;
#endif

        data[MEM_WIDTH + 8] = 1;
        addr_out = ctu_id_top * CABAC_METAS_SIZE + (addr - CABAC_STATES_OFFSET);
      }
    }

    if (cabac_push) {
      cabac_states_push.write(temp);
    } else {
      mem_out[addr_out] = data;
      if (top_btm) {
        mem_out_btm_top[addr_out2] = data2;
      }
    }

#ifndef __SYNTHESIS__
  }
#endif
}

#pragma hls_noglobals
#pragma hls_design block
void main_indexer_out(ac_channel< ac_int< MEM_WIDTH, false > > &data_out, ac_channel< ac_int< CTU_ID_WIDTH, false > > &data_req, ac_channel< ac_int< CTU_ID_WIDTH, false > > &data_req_hier,
                      ac_channel< ac_int< MEM_WIDTH, false > > &cabac_states_pull, ac_int< MEM_WIDTH, false > rec[REC_SIZE * CTU_IDS], ac_int< MEM_WIDTH, false > cu_info[CU_INFO_SIZE * CTU_IDS],
                      ac_int< MEM_WIDTH, false > cabac_metas[CABAC_METAS_SIZE2 * CTU_IDS / 2], ac_int< MEM_WIDTH, false > cabac_metas_top[CABAC_METAS_SIZE2 * CTU_IDS / 2],
                      ac_int< MEM_WIDTH, false > bitstream_last_data[BITSTREAM_SIZE * CTU_IDS / 2], ac_int< MEM_WIDTH, false > bitstream_last_data_top[BITSTREAM_SIZE * CTU_IDS / 2]) {
#ifndef __SYNTHESIS__
  while (data_req.available(1) || cabac_states_pull.available(1)) {
#endif

#ifndef __SYNTHESIS__
    CHECK_INTEGER(REC_OFFSET);
    CHECK_INTEGER(CABAC_STATES_OFFSET2);
    CHECK_INTEGER(CU_INFO_OFFSET);
    CHECK_INTEGER(CABAC_METAS_OFFSET2);
    CHECK_INTEGER(BITSTREAM_OFFSET);
#endif

    static ac_int< CTU_ID_WIDTH, false > ctu_id;

#ifndef __SYNTHESIS__
    static int send_data_req_once = 1;

    if (send_data_req_once) {
#endif
      ctu_id = data_req.read();
      data_req_hier.write(ctu_id);
#ifndef __SYNTHESIS__
      send_data_req_once = 0;
      return;
    }
#endif

    ac_int< CTU_ID_WIDTH, false > ctu_id_top = ctu_id - 8;

    ac_int< MEM_WIDTH, false > temp;

    for (ac_int< ac::nbits< OUTPUT_BYTES / MEM_BYTES >::val, false > a = 0; a < OUTPUT_BYTES / MEM_BYTES; a++) {
      if (a < REC_OFFSET) {
        temp = rec[ctu_id * REC_SIZE + a];
      } else if (a < CABAC_STATES_OFFSET2) {
        temp = cabac_states_pull.read();
      } else if (a < CU_INFO_OFFSET) {
        temp = cu_info[ctu_id * CU_INFO_SIZE + (a - CABAC_STATES_OFFSET2)];
      } else if (a < CABAC_METAS_OFFSET2) {
        if (ctu_id < 8) {
          temp = cabac_metas[ctu_id * CABAC_METAS_SIZE2 + (a - CU_INFO_OFFSET)];
        } else {
          temp = cabac_metas_top[ctu_id_top * CABAC_METAS_SIZE2 + (a - CU_INFO_OFFSET)];
        }
      } else if (a < BITSTREAM_OFFSET) {
        if (ctu_id < 8) {
          temp = bitstream_last_data[ctu_id * BITSTREAM_SIZE + (a - CABAC_METAS_OFFSET2)];
        } else {
          temp = bitstream_last_data_top[ctu_id_top * BITSTREAM_SIZE + (a - CABAC_METAS_OFFSET2)];
        }
      }

      data_out.write(temp);
    }

#ifndef __SYNTHESIS__
  }
#endif
}

#pragma hls_noglobals
#pragma hls_design block
void cabac_states_arbiter(ac_channel< ac_int< MEM_WIDTH + DATA_IN_ADDR_WIDTH + CTU_ID_WIDTH, false > > &data_update, ac_channel< ac_int< CTU_ID_WIDTH, false > > &data_req,
                          ac_channel< ac_int< MEM_WIDTH, false > > &data_out, ac_int< MEM_WIDTH, false > cabac_states[CABAC_STATES_SIZE * CTU_IDS / 2],
                          ac_int< MEM_WIDTH, false > cabac_states_top[CABAC_STATES_SIZE * CTU_IDS / 2]) {
#ifndef __SYNTHESIS__
  while (data_update.available(1) || data_req.available(1)) {
#endif

    ac_int< 1, false > push = (data_update.size() > 0);
    ac_int< 1, false > pull = (data_req.size() > 0);

    if (push) {
      for (ac_int< ac::nbits< CABAC_STATES_SIZE >::val, false > a = 0; a < CABAC_STATES_SIZE; a++) {
        ac_int< MEM_WIDTH + DATA_IN_ADDR_WIDTH + CTU_ID_WIDTH, false > data_in = data_update.read();

        ac_int< MEM_WIDTH, false > data = data_in.slc< MEM_WIDTH >(0);
        ac_int< DATA_IN_ADDR_WIDTH, false > addr = data_in.slc< DATA_IN_ADDR_WIDTH >(MEM_WIDTH) / MEM_BYTES;
        ac_int< CTU_ID_WIDTH, false > ctu_id = data_in.slc< CTU_ID_WIDTH >(MEM_WIDTH + DATA_IN_ADDR_WIDTH);
        ac_int< CTU_ID_WIDTH, false > ctu_id_top = ctu_id - 8;

        if (ctu_id < 8) {
          cabac_states[ctu_id * CABAC_STATES_SIZE + (addr - CABAC_OFFSET)] = data;
        } else {
          cabac_states_top[ctu_id_top * CABAC_STATES_SIZE + (addr - CABAC_OFFSET)] = data;
        }
      }
    } else if (pull) {
      ac_int< CTU_ID_WIDTH, false > ctu_id = data_req.read();
      ac_int< CTU_ID_WIDTH, false > ctu_id_top = ctu_id - 8;

      for (ac_int< ac::nbits< CABAC_STATES_SIZE >::val, false > a = 0; a < CABAC_STATES_SIZE; a++) {
        ac_int< MEM_WIDTH, false > data;
        if (ctu_id < 8) {
          data = cabac_states[ctu_id * CABAC_STATES_SIZE + a];
        } else {
          data = cabac_states_top[ctu_id_top * CABAC_STATES_SIZE + a];
        }
        data_out.write(data);
      }
    }

#ifndef __SYNTHESIS__
  }
#endif
}

#pragma hls_noglobals
#pragma hls_design top
void main_indexer_top(ac_channel< ac_int< MEM_WIDTH + DATA_IN_ADDR_WIDTH + CTU_ID_WIDTH, false > > &data_in, ac_int< MEM_WIDTH + 9, false > mem_out[REF_SIZE * CTU_IDS],
                      ac_int< MEM_WIDTH + 6, false > mem_out_btm_top[REF_SIZE * CTU_IDS], ac_int< MEM_WIDTH, false > cabac_states[CABAC_STATES_SIZE * CTU_IDS / 2],
                      ac_int< MEM_WIDTH, false > cabac_states_top[CABAC_STATES_SIZE * CTU_IDS / 2], ac_channel< ac_int< MEM_WIDTH, false > > &data_out,
                      ac_channel< ac_int< CTU_ID_WIDTH, false > > &data_req, ac_int< MEM_WIDTH, false > rec[REC_SIZE * CTU_IDS], ac_int< MEM_WIDTH, false > cu_info[CU_INFO_SIZE * CTU_IDS],
                      ac_int< MEM_WIDTH, false > cabac_metas2[CABAC_METAS_SIZE2 * CTU_IDS / 2], ac_int< MEM_WIDTH, false > cabac_metas2_top[CABAC_METAS_SIZE2 * CTU_IDS / 2],
                      ac_int< MEM_WIDTH, false > bitstream_last_data[BITSTREAM_SIZE * CTU_IDS / 2], ac_int< MEM_WIDTH, false > bitstream_last_data_top[BITSTREAM_SIZE * CTU_IDS / 2]) {
  static ac_channel< ac_int< MEM_WIDTH + DATA_IN_ADDR_WIDTH + CTU_ID_WIDTH, false > > cabac_states_push;

  static ac_channel< ac_int< CTU_ID_WIDTH, false > > data_req_hier;
  static ac_channel< ac_int< MEM_WIDTH, false > > cabac_states_pull;

  main_indexer_in(data_in, mem_out, mem_out_btm_top, cabac_states_push);
  main_indexer_out(data_out, data_req, data_req_hier, cabac_states_pull, rec, cu_info, cabac_metas2, cabac_metas2_top, bitstream_last_data, bitstream_last_data_top);
  cabac_states_arbiter(cabac_states_push, data_req_hier, cabac_states_pull, cabac_states, cabac_states_top);
}
