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

#ifndef GLOBAL_H_
#define GLOBAL_H_

#include "ac_assert.h" // Replaced <assert.h> with <ac_assert.h>
#include "ac_channel.h"
#include "ac_fixed.h"
#include "ac_int.h"

#ifndef __SYNTHESIS__
#include <iostream>
using namespace std;
#endif

#define CHECK_INTEGER(macro) assert((macro) == (int)(macro))

#define MEM_WIDTH 128
#define MEM_BYTES (MEM_WIDTH / 8)
#define CTU_IDS 16
#define DATA_IN_ADDR_WIDTH 24
#define CTU_ID_WIDTH 4

#define REF_BYTES 6144
#define TOP_BORDER_BYTES 256
#define LEFT_BORDER_BYTES 256
#define CU_INFO_VER_BYTES 16
#define CU_INFO_HOR_BYTES 16
#define CABAC_BYTES 16
#define CABAC_STATES_BYTES 256
#define CABAC_METAS_BYTES 16

#define REF_SIZE (REF_BYTES / MEM_BYTES)
#define TOP_BORDER_SIZE (TOP_BORDER_BYTES / MEM_BYTES)
#define LEFT_BORDER_SIZE (LEFT_BORDER_BYTES / MEM_BYTES)
#define CU_INFO_VER_SIZE (CU_INFO_VER_BYTES / MEM_BYTES)
#define CU_INFO_HOR_SIZE (CU_INFO_HOR_BYTES / MEM_BYTES)
#define CABAC_SIZE (CABAC_BYTES / MEM_BYTES)
#define CABAC_STATES_SIZE (CABAC_STATES_BYTES / MEM_BYTES)
#define CABAC_METAS_SIZE (CABAC_METAS_BYTES / MEM_BYTES)

#define REF_OFFSET REF_BYTES / MEM_BYTES
#define TOP_BORDER_OFFSET (REF_OFFSET + TOP_BORDER_BYTES / MEM_BYTES)
#define LEFT_BORDER_OFFSET (TOP_BORDER_OFFSET + LEFT_BORDER_BYTES / MEM_BYTES)
#define CU_INFO_VER_OFFSET (LEFT_BORDER_OFFSET + CU_INFO_VER_BYTES / MEM_BYTES)
#define CU_INFO_HOR_OFFSET (CU_INFO_VER_OFFSET + CU_INFO_HOR_BYTES / MEM_BYTES)
#define CU_INFO_HOR_STR_OFFSET (CU_INFO_HOR_OFFSET + CU_INFO_HOR_BYTES / MEM_BYTES)
#define CABAC_OFFSET (CU_INFO_HOR_STR_OFFSET + CABAC_BYTES / MEM_BYTES)
#define CABAC_STATES_OFFSET (CABAC_OFFSET + CABAC_STATES_BYTES / MEM_BYTES)
#define CABAC_METAS_OFFSET (CABAC_STATES_OFFSET + CABAC_METAS_BYTES / MEM_BYTES)

#define REC_BYTES 6144
#define CU_INFO_BYTES 1024
#define CABAC_METAS_BYTES2 32
#define BITSTREAM_BYTES 2048

#define OUTPUT_BYTES (REC_BYTES + CABAC_STATES_BYTES + CU_INFO_BYTES + CABAC_METAS_BYTES2 + BITSTREAM_BYTES)

#define REC_SIZE (REC_BYTES / MEM_BYTES)
#define CU_INFO_SIZE (CU_INFO_BYTES / MEM_BYTES)
#define CABAC_METAS_SIZE2 (CABAC_METAS_BYTES2 / MEM_BYTES)
#define BITSTREAM_SIZE (BITSTREAM_BYTES / MEM_BYTES)

#define REC_OFFSET REC_BYTES / MEM_BYTES
#define CABAC_STATES_OFFSET2 (REC_OFFSET + CABAC_STATES_BYTES / MEM_BYTES)
#define CU_INFO_OFFSET (CABAC_STATES_OFFSET2 + CU_INFO_BYTES / MEM_BYTES)
#define CABAC_METAS_OFFSET2 (CU_INFO_OFFSET + CABAC_METAS_BYTES2 / MEM_BYTES)
#define BITSTREAM_OFFSET (CABAC_METAS_OFFSET2 + BITSTREAM_BYTES / MEM_BYTES)

#ifndef __SYNTHESIS__
extern ac_int< MEM_WIDTH, false > ref_pixels[REF_SIZE * CTU_IDS];
extern ac_int< MEM_WIDTH, false > top_border[TOP_BORDER_SIZE * CTU_IDS];
extern ac_int< MEM_WIDTH, false > left_border[LEFT_BORDER_SIZE * CTU_IDS];
extern ac_int< MEM_WIDTH, false > cu_info_ver[CU_INFO_VER_SIZE * CTU_IDS];
extern ac_int< MEM_WIDTH, false > cu_info_ver_d1[CU_INFO_VER_SIZE * CTU_IDS / 2];
extern ac_int< MEM_WIDTH, false > cu_info_ver_d2[CU_INFO_VER_SIZE * CTU_IDS / 2];
extern ac_int< MEM_WIDTH, false > cu_info_ver_d1_top[CU_INFO_VER_SIZE * CTU_IDS / 2];
extern ac_int< MEM_WIDTH, false > cu_info_ver_d2_top[CU_INFO_VER_SIZE * CTU_IDS / 2];
extern ac_int< MEM_WIDTH, false > cu_info_hor[CU_INFO_HOR_SIZE * CTU_IDS];
extern ac_int< MEM_WIDTH, false > cu_info_hor_d1[CU_INFO_HOR_SIZE * CTU_IDS / 2];
extern ac_int< MEM_WIDTH, false > cu_info_hor_d2[CU_INFO_HOR_SIZE * CTU_IDS / 2];
extern ac_int< MEM_WIDTH, false > cu_info_hor_d1_top[CU_INFO_HOR_SIZE * CTU_IDS / 2];
extern ac_int< MEM_WIDTH, false > cu_info_hor_d2_top[CU_INFO_HOR_SIZE * CTU_IDS / 2];
extern ac_int< MEM_WIDTH, false > cu_info_hor_str[CU_INFO_HOR_SIZE * CTU_IDS];
extern ac_int< MEM_WIDTH, false > cabac[CABAC_SIZE * CTU_IDS];
extern ac_int< MEM_WIDTH, false > cabac_metas[CABAC_METAS_SIZE * CTU_IDS / 2];
extern ac_int< MEM_WIDTH, false > cabac_metas_top[CABAC_METAS_SIZE * CTU_IDS / 2];
#endif

#endif
