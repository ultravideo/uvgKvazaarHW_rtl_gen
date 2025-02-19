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

#ifndef ANG_NEG_MODE_SETUP_H_
#define ANG_NEG_MODE_SETUP_H_

#include "global.h"

#define MODES 14_22

#define MAKE_FN_NAME(x) main_ip_get_ang_neg_##x
#define MAKE_FN(x) void main_ip_get_ang_neg_##x(input_port_t &conf_in, pixel_t ref1[4][64][PARALLEL_PIX * 2], pixel_t ref2[4][64][PARALLEL_PIX * 2], output_port_t &hor_out, output_port_t &ver_out)

#define MAIN_IP_GET_ANG_NEG(modes) MAKE_FN(modes)
#define MAIN_IP_GET_ANG_NEG_CALL(modes) MAKE_FN_NAME(modes)

const int compiled_modes[2] = {14, 22};

const uint_6 indexes[13] = {2, 5, 7, 10, 12, 15, 17, 20, 22, 25, 27, 30, 32};
const int_7 delta_int_table[32] = {
    -1, -1, -2, -2, -3, -3, -3, -4, -4, -5, -5, -5, -6, -6, -7, -7, -7, -8, -8, -9, -9, -9, -10, -10, -11, -11, -11, -12, -12, -13, -13, -13,
};
const uint_5 delta_fract_table[32] = {
    19, 6, 25, 12, 31, 18, 5, 24, 11, 30, 17, 4, 23, 10, 29, 16, 3, 22, 9, 28, 15, 2, 21, 8, 27, 14, 1, 20, 7, 26, 13, 0,
};

#endif
