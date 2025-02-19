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

#ifndef INTRA_H_
#define INTRA_H_

#include "global.h"

static void kvz_intra_get_dir_luma_predictor(uint6 *intra_preds, const cu_simple_info_t *const left_pu, const cu_simple_info_t *const above_pu) {
  // The default mode if block is not coded yet is INTRA_DC.
  uint6 left_intra_dir = 1;
  if (left_pu) {
    left_intra_dir = left_pu->intra_mode();
  }

  uint6 above_intra_dir = 1;
  if (above_pu) {
    above_intra_dir = above_pu->intra_mode();
  }

  // If the predictions are the same, add new predictions
  if (left_intra_dir == above_intra_dir) {
    if (left_intra_dir > 1) { // angular modes
      intra_preds[0] = left_intra_dir;
      intra_preds[1] = ((left_intra_dir + 29) % 32) + 2;
      intra_preds[2] = ((left_intra_dir - 1) % 32) + 2;
    } else {               // non-angular
      intra_preds[0] = 0;  // PLANAR_IDX;
      intra_preds[1] = 1;  // DC_IDX;
      intra_preds[2] = 26; // VER_IDX;
    }
  } else { // If we have two distinct predictions
    intra_preds[0] = left_intra_dir;
    intra_preds[1] = above_intra_dir;

    // add planar mode if it's not yet present
    if (left_intra_dir && above_intra_dir) {
      intra_preds[2] = 0; // PLANAR_IDX;
    } else {              // Add DC mode if it's not present, otherwise 26.
      intra_preds[2] = (left_intra_dir + above_intra_dir) < 2 ? 26 : 1;
    }
  }
}

#endif
