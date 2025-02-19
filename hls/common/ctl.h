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

#ifndef CTL_H_
#define CTL_H_

#include "ac_int.h"

// -------------------------------------------------------- LOCAL FUNCTIONS -------------------------------------------------------- \\

namespace {

// MAX
template < unsigned N > struct MAX {
  template < typename T, typename I > static T max(T *a, I i, I &index) {
    I i0, i1;
    T m0 = MAX< N / 2 >::template max< T, I >(a, i, i0);
    T m1 = MAX< N - N / 2 >::template max< T, I >(a, i + N / 2, i1);

    if (m0 >= m1) {
      index = i0;
      return m0;
    } else {
      index = i1;
      return m1;
    }
  }
};

template <> struct MAX< 1U > {
  template < typename T, typename I > static T max(T *a, I i, I &index) {
    index = i;
    return a[i];
  }
};

// MIN
template < unsigned N > struct MIN {
  template < typename T, typename I > static T min(T *a, I i, I &index) {
    I i0, i1;
    T m0 = MIN< N / 2 >::template min< T, I >(a, i, i0);
    T m1 = MIN< N - N / 2 >::template min< T, I >(a, i + N / 2, i1);

    if (m0 <= m1) {
      index = i0;
      return m0;
    } else {
      index = i1;
      return m1;
    }
  }
};

template <> struct MIN< 1U > {
  template < typename T, typename I > static T min(T *a, I i, I &index) {
    index = i;
    return a[i];
  }
};

// SUM
template < unsigned N > struct SUM {
  template < unsigned width, bool sign, typename I > static ac_int< width + ac::log2_ceil< N >::val, sign > sum(ac_int< width, sign > *a, I i) {
    ac_int< width + ac::log2_ceil< N / 2 >::val, sign > m0, m1;

    m0 = SUM< N / 2 >::template sum< width, sign, I >(a, i);
    m1 = SUM< N - N / 2 >::template sum< width, sign, I >(a, i + N / 2);

    return m0 + m1;
  }
};

template <> struct SUM< 1U > {
  template < unsigned width, bool sign, typename I > static ac_int< width, sign > sum(ac_int< width, sign > *a, I i) { return a[i]; }
};
} // namespace

// -------------------------------------------------------- PUBLIC FUNCTIONS -------------------------------------------------------- \\

namespace ctl {

template < int width > ac_int< width > Abs(ac_int< width > a) {
  ac_int< width > tmp0 = a, tmp1 = 0;

#pragma hls_unroll yes
  for (int i = 0; i < width; i++)
    tmp1[i] = tmp0[i] ^ tmp0[width - 1];

  return tmp1 + tmp0[width - 1];
}

/*      Max & Min
 *
 *   N       Array length
 *
 *       Optional:
 *   i       index of best value
 */
template < unsigned N, typename T > T Max(T *a) {
  ac_int< ac::log2_ceil< N >::val, false > i;
  return MAX< N >::template max< T, ac_int< ac::log2_ceil< N >::val, false > >(a, 0, i);
}
template < unsigned N, typename T > T Max(T *a, ac_int< ac::log2_ceil< N >::val, false > &i) { return MAX< N >::template max< T, ac_int< ac::log2_ceil< N >::val, false > >(a, 0, i); }
template < unsigned N, typename T > ac_int< ac::log2_ceil< N >::val, false > MaxIndex(T *a) {
  ac_int< ac::log2_ceil< N >::val, false > i;
  MAX< N >::template max< T, ac_int< ac::log2_ceil< N >::val, false > >(a, 0, i);
  return i;
}

template < unsigned N, typename T > T Min(T *a) {
  ac_int< ac::log2_ceil< N >::val, false > i;
  return MIN< N >::template min< T, ac_int< ac::log2_ceil< N >::val, false > >(a, 0, i);
}
template < unsigned N, typename T > T Min(T *a, ac_int< ac::log2_ceil< N >::val, false > &i) { return MIN< N >::template min< T, ac_int< ac::log2_ceil< N >::val, false > >(a, 0, i); }
template < unsigned N, typename T > ac_int< ac::log2_ceil< N >::val, false > MinIndex(T *a) {
  ac_int< ac::log2_ceil< N >::val, false > i;
  MIN< N >::template min< T, ac_int< ac::log2_ceil< N >::val, false > >(a, 0, i);
  return i;
}

/*      Rotate
 *
 *   width   Input width
 *   step    Rotate step
 *
 *   Example:
 *       Step 1: Move one bit
 *       Step 8: Move full byte
 *
 *   Width needs to be divisible by step
 */
template < int width, int step > ac_int< width > RotateL(const ac_int< width > &a, ac_int< ac::log2_ceil< width / step >::val, false > i) {
  ac_int< width > r[width / step];

#pragma hls_unroll yes
  for (int y = 0; y < width / step; ++y) {
#pragma hls_unroll yes
    for (int x = 0; x < width / step; ++x) {
#ifndef __SYNTHESIS__
      r[y].set_slc(step * ac_int< ac::log2_ceil< width / step >::val, false >(x + y), (ac_int< step, false >)(a >> (step * x)));
#else
      r[y].set_slc(step * ac_int< ac::log2_ceil< width / step >::val, false >(x + y), a.slc< step >(step * x));
#endif
    }
  }

  return r[i];
}

template < int width, int step > ac_int< width > RotateR(const ac_int< width > &a, ac_int< ac::log2_ceil< width / step >::val, false > i) {
  ac_int< width > r[width / step];

#pragma hls_unroll yes
  for (int y = 0; y < width / step; ++y) {
#pragma hls_unroll yes
    for (int x = 0; x < width / step; ++x) {
#ifndef __SYNTHESIS__
      r[y].set_slc(step * ac_int< ac::log2_ceil< width / step >::val, false >(x - y), (ac_int< step, false >)(a >> (step * x)));
#else
      r[y].set_slc(step * ac_int< ac::log2_ceil< width / step >::val, false >(x - y), a.slc< step >(step * x));
#endif
    }
  }

  return r[i];
}

/*      Sum
 *
 *   width   Input width
 *   sign    Input sign
 *   N       Array length
 */
template < unsigned width, bool sign, unsigned N > ac_int< width + ac::log2_ceil< N >::val, sign > Sum(ac_int< width, sign > *a) {
  return SUM< N >::template sum< width, sign, ac_int< ac::log2_ceil< N >::val, false > >(a, 0);
}

/*      Ones & Zeros
 *
 *   width   Input width
 *
 *   Calculates number of one or zero bits in input
 */
template < unsigned width > ac_int< ac::nbits< width >::val, false > Ones(ac_int< width > a) {
  ac_int< 1, false > array[width];

#pragma hls_unroll yes
  for (unsigned i = 0; i < width; ++i)
    array[i] = a[i];

  return Sum< 1, false, width >(array);
}

template < unsigned width > ac_int< ac::nbits< width >::val, false > Zeros(ac_int< width > a) {
  ac_int< 1, false > array[width];

#pragma hls_unroll yes
  for (unsigned i = 0; i < width; ++i)
    array[i] = ~a[i];

  return Sum< 1, false, width >(array);
}

/*      AddShiftMultiply
 *
 *   Calculates multiplication by using adds and shifts
 *
 *   !!! Unsigned numbers only !!!
 */
template < unsigned width_a, bool sign_a, unsigned width_b, bool sign_b > ac_int< width_a + width_b, sign_a || sign_b > AddShiftMultiply(ac_int< width_a, sign_a > a, ac_int< width_b, sign_b > b) {
  ac_int< width_a + width_b, sign_a > Mult = a;
  ac_int< width_a + width_b, sign_a || sign_b > Shifts[width_b];

#pragma hls_unroll yes
  for (unsigned i = 0; i < width_b; ++i) {
    Shifts[i] = b[i] ? (Mult << i) : 0;
  }

  return Sum < width_a + width_b, sign_a || sign_b, width_b > (Shifts);
}
} // namespace ctl

#endif // CTL_H_
