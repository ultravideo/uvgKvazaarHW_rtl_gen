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

#ifndef TYPES_H_
#define TYPES_H_

#include "global.h"

#define SIZE_PC ac::log2_ceil< CTU_INST_STACK_SIZE >::val

typedef ac_int< SIZE_PC, false > pc_t;

class ctu_init_t {
public:
  typedef ac_channel< ctu_init_t > port_t;

  uint_4 lcu_id;
  uint_4 inst_set;
  one_bit use_preset;

  ctu_init_t() : lcu_id(0), inst_set(0), use_preset(0) {}
  ctu_init_t(uint_4 id) : lcu_id(id), inst_set(0), use_preset(0) {}
};

struct inst_exec_t {
  uint_4 lcu_id;
  one_bit skip;

  inst_exec_t() {}
  inst_exec_t(uint_4 id, one_bit s) : lcu_id(id), skip(s) {}
};

struct exec_t {
  uint_4 lcu_id;
  two_bit depth;
  two_bit color;

  uint_6 x, y;
  two_bit op_code;
};

class ip_conf_t {
public:
  typedef ac_int< 48, false > ac_t;

  uint_4 lcu_id;
  two_bit depth;
  two_bit color;

  uint_6 x_pos;
  uint_6 y_pos;

  uint_8 labda;
  uint_6 qp_scaled;

  uint_7 x_limit;
  uint_7 y_limit;
  one_bit x_zero;
  one_bit y_zero;

  ip_conf_t() { ip_conf_t(0); }
  ip_conf_t(const exec_t &inst) {
    lcu_id = inst.lcu_id;
    depth = inst.depth;
    color = inst.color;
    x_pos = inst.x;
    y_pos = inst.y;
  }

  ip_conf_t(const ac_t &ac) {
    lcu_id = ac.slc< 4 >(0);
    depth = ac.slc< 2 >(4);
    color = ac.slc< 2 >(6);

    x_pos = uint_6(ac.slc< 4 >(8)) << 2;
    y_pos = uint_6(ac.slc< 4 >(12)) << 2;

    labda = ac.slc< 6 >(16);
    qp_scaled = ac.slc< 6 >(24);

    x_limit = ac.slc< 7 >(32);
    x_zero = ac.slc< 1 >(39);
    y_limit = ac.slc< 7 >(40);
    y_zero = ac.slc< 1 >(47);
  }

  ac_t toInt() const {
    ac_t output = 0;
    output.set_slc(0, lcu_id);
    output.set_slc(4, depth);
    output.set_slc(6, color);

    output.set_slc(8, uint_4(x_pos >> 2));
    output.set_slc(12, uint_4(y_pos >> 2));

    output.set_slc(16, labda);
    output.set_slc(24, qp_scaled);

    output.set_slc(32, x_limit);
    output.set_slc(39, x_zero);
    output.set_slc(40, y_limit);
    output.set_slc(47, y_zero);

    return output;
  }
};

class inst_t {
public:
  typedef ac_int< 14 + SIZE_PC, false > ac_t;

  two_bit op_code;

  two_bit depth;
  two_bit color;

  // 4bits each
  uint_6 x_base;
  uint_6 y_base;

  pc_t skip; // Program counter skip location

  inst_t() { inst_t(0); }
  inst_t(two_bit op, two_bit d, two_bit c, uint_6 x, uint_6 y, pc_t s) : op_code(op), depth(d), color(c), x_base(x), y_base(y), skip(s) {}

  inst_t(const ac_t &ac) {
    op_code = ac.slc< 2 >(0);
    depth = ac.slc< 2 >(2);
    color = ac.slc< 2 >(4);
    x_base = uint_6(ac.slc< 4 >(6)) << 2;
    y_base = uint_6(ac.slc< 4 >(10)) << 2;
    skip = pc_t(ac >> 14);
  }

  ac_t toInt() const {
    ac_t output;
    output.set_slc(0, op_code);
    output.set_slc(2, depth);
    output.set_slc(4, color);
    output.set_slc(6, uint_4(x_base >> 2));
    output.set_slc(10, uint_4(y_base >> 2));
    output.set_slc(14, skip);
    return output;
  }

  exec_t toExec(uint_4 lcu_id) const {
    exec_t output;
    output.lcu_id = lcu_id;
    output.depth = depth;
    output.color = color;

    output.x = x_base;
    output.y = y_base;

    output.op_code = op_code;
    return output;
  }
};

class ctu_stack_t {
public:
  typedef ac_int< 25 + 2 * FIXED_T_BITS, false > ac_t;

  uint_6 x;
  uint_6 y;

  uint_6 mode;
  uint_6 mode_chroma;

  ufixed_t cost;
  ufixed_t split_cost;

  one_bit split_valid;

  one_bit trskip;
  one_bit coeffs[3];

  ctu_stack_t() { ctu_stack_t(0); }

  ctu_stack_t(const ac_t &ac) {
    x = uint_6(ac.slc< 4 >(0)) << 2;
    y = uint_6(ac.slc< 4 >(4)) << 2;

    mode = ac.slc< 6 >(8);
    mode_chroma = ac.slc< 6 >(14);

    trskip = ac.slc< 1 >(20);
    coeffs[0] = ac.slc< 1 >(21);
    coeffs[1] = ac.slc< 1 >(22);
    coeffs[2] = ac.slc< 1 >(23);

    split_valid = ac.slc< 1 >(24);

    cost.set_slc(0, ac.slc< FIXED_T_BITS >(25));
    split_cost.set_slc(0, ac.slc< FIXED_T_BITS >(25 + FIXED_T_BITS));
  }

  ac_t toInt() const {
    ac_t output;
    output.set_slc(0, uint_4(x >> 2));
    output.set_slc(4, uint_4(y >> 2));

    output.set_slc(8, mode);
    output.set_slc(14, mode_chroma);

    output.set_slc(20, trskip);
    output.set_slc(21, coeffs[0]);
    output.set_slc(22, coeffs[1]);
    output.set_slc(23, coeffs[2]);

    output.set_slc(24, split_valid);

    output.set_slc(25, cost.slc< FIXED_T_BITS >(0));
    output.set_slc(25 + FIXED_T_BITS, split_cost.slc< FIXED_T_BITS >(0));
    return output;
  }
};

class ctu_str_conf_t {
public:
  typedef ac_int< 44, false > ac_t;

  two_bit min_depth;
  two_bit max_depth;

  int_8 qp;
  uint_8 lambda_sqrt;

  uint_8 cabac;

  uint_7 x_limit;
  uint_7 y_limit;
  one_bit x_zero;
  one_bit y_zero;

  ctu_str_conf_t() { ctu_str_conf_t(0); }

  ctu_str_conf_t(const ac_t &ac) {
    x_limit = ac.slc< 7 >(0);
    x_zero = ac.slc< 1 >(7);
    y_limit = ac.slc< 7 >(8);
    y_zero = ac.slc< 1 >(15);

    min_depth = ac.slc< 2 >(16);
    max_depth = ac.slc< 2 >(18);

    qp = ac.slc< 8 >(20);
    lambda_sqrt = ac.slc< 8 >(28);

    cabac = ac.slc< 8 >(36);
  }

  ac_t toInt() const {
    ac_t output;
    output.set_slc(0, x_limit);
    output.set_slc(7, x_zero);
    output.set_slc(8, y_limit);
    output.set_slc(15, y_zero);

    output.set_slc(16, min_depth);
    output.set_slc(18, max_depth);

    output.set_slc(20, qp);
    output.set_slc(28, lambda_sqrt);
    output.set_slc(36, cabac);
    return output;
  }
};

class ctu_end_conf_t {
public:
  typedef ac_int< 16, false > ac_t;

  uint_8 lambda;
  one_bit x_zero;
  one_bit y_zero;

  two_bit min_depth;
  two_bit max_depth;

  one_bit trskip_enable;

  ctu_end_conf_t() { ctu_end_conf_t(0); }

  ctu_end_conf_t(const ac_t &ac) {
    min_depth = ac.slc< 2 >(0);
    max_depth = ac.slc< 2 >(2);

    x_zero = ac.slc< 1 >(4);
    y_zero = ac.slc< 1 >(5);
    trskip_enable = ac.slc< 1 >(6);

    lambda = ac.slc< 8 >(8);
  }

  ac_t toInt() const {
    ac_t output;
    output.set_slc(0, min_depth);
    output.set_slc(2, max_depth);

    output.set_slc(4, x_zero);
    output.set_slc(5, y_zero);
    output.set_slc(6, trskip_enable);

    output.set_slc(8, lambda);
    return output;
  }
};

typedef ac_channel< exec_t > exec_port_t;

int_48 to_ac_int(const ip_conf_t &ip_conf);
ip_conf_t to_struct(const int_48 &ac_int);

#endif
