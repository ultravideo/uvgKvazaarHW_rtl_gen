#############################################################################
# This file is part of uvgKvazaarHW.
#
# Copyright (c) 2025, Tampere University, ITU/ISO/IEC, project contributors
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
# 
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
# 
# * Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
# 
# * Neither the name of the Tampere University or ITU/ISO/IEC nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# INCLUDING NEGLIGENCE OR OTHERWISE ARISING IN ANY WAY OUT OF THE USE OF THIS
#############################################################################

# List of constraints
proc set_analyse_directives {} {
    # Insert the list of analyse directives
    directive set -TRANSACTION_DONE_SIGNAL false
    directive set -RESET_CLEARS_ALL_REGS no
    directive set -CLOCK_OVERHEAD 40.000000
}
proc set_assembly_directives {} {
    # Insert the list of assembly directives
    directive set /main_coeff_cost/coeff_transpose_pipe/output_port:rsc -FIFO_DEPTH 0
    directive set /main_coeff_cost/get_coeff_cost/coeff_in:rsc -FIFO_DEPTH 0
    directive set /main_coeff_cost/coeff_transpose_pull/output_port:rsc -FIFO_DEPTH 0
    directive set /main_coeff_cost/coeff_trans:cns -FIFO_DEPTH 0
    directive set /main_coeff_cost/coeff_transpose_pipe/output_port:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_coeff_cost/get_coeff_cost/coeff_in:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_coeff_cost/get_coeff_cost/coeff_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_coeff_cost/coeff_transpose_pull/output_port:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_coeff_cost/coeff_transpose_pipe/trans_conf:cns -FIFO_DEPTH 0
    directive set /main_coeff_cost/coeff_transpose_pipe/trans_push:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_coeff_cost/coeff_transpose_push/trans:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_coeff_cost/trans_push:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_coeff_cost/coeff_transpose_pipe/input_port:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_coeff_cost/coeff_transpose_push/input_port:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_coeff_cost/coeff_in:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_coeff_cost/coeff_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_coeff_cost/coeff_transpose_pipe/trans_push:rsc -INTERLEAVE 32
    directive set /main_coeff_cost/coeff_transpose_push/trans:rsc -INTERLEAVE 32
    directive set /main_coeff_cost/trans_push:rsc -INTERLEAVE 32
    directive set /main_coeff_cost/coeff_transpose_pipe/trans_pull:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_coeff_cost/coeff_transpose_pull/trans:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_coeff_cost/trans_pull:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_coeff_cost/coeff_transpose_pipe/trans_pull:rsc -INTERLEAVE 32
    directive set /main_coeff_cost/coeff_transpose_pull/trans:rsc -INTERLEAVE 32
    directive set /main_coeff_cost/trans_pull:rsc -INTERLEAVE 32
    directive set /main_coeff_cost/coeff_transpose_pull/core/ctl::RotateR<512,16>:r:rsc -MAP_TO_MODULE {[Register]}
    directive set /main_coeff_cost/coeff_transpose_push/core/ctl::RotateL<512,16>:r:rsc -MAP_TO_MODULE {[Register]}
    directive set -DSP_EXTRACTION yes
    directive set -CLOCKS {clk {-CLOCK_PERIOD 5.0 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 2.5 -RESET_SYNC_NAME rst -RESET_ASYNC_NAME arst_n -RESET_KIND async -RESET_SYNC_ACTIVE low -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high}}
    }

proc set_extract_directives {} {
    # Insert the list of extract directives
    directive set /main_coeff_cost -DSP_MAPPING systolic
    directive set /main_coeff_cost -DSP_EXTRACTION_TRAV_PREADD_FANOUT false
    directive set /main_coeff_cost -DSP_EXTRACTION_UNFOLD_MAC false
    directive set /main_coeff_cost -DSP_EXTRACTION_COMPLEXMULT use_library
    directive set /main_coeff_cost -DSP_EXTRACTION_OPTIONS {muladd mulacc mul2add mul2acc mul3add mul3acc mul4add mul4acc}
    directive set /main_coeff_cost/get_coeff_cost -DSP_MAPPING systolic
    directive set /main_coeff_cost/get_coeff_cost -DSP_EXTRACTION_TRAV_PREADD_FANOUT true
    directive set /main_coeff_cost/get_coeff_cost -DSP_EXTRACTION_UNFOLD_MAC true
    directive set /main_coeff_cost/get_coeff_cost -DSP_EXTRACTION_COMPLEXMULT yes
    directive set /main_coeff_cost/get_coeff_cost/core/ctl::Ones<64U>:array:rsc -MAP_TO_MODULE {[Register]}
    directive set /main_coeff_cost/get_coeff_cost/core/ctl::Ones<64U>#1:array:rsc -MAP_TO_MODULE {[Register]}
    directive set /main_coeff_cost/get_coeff_cost/core/ctl::Ones<64U>#2:array:rsc -MAP_TO_MODULE {[Register]}
    directive set /main_coeff_cost/get_coeff_cost/core/main -PIPELINE_STALL_MODE flush
    directive set /main_coeff_cost/get_coeff_cost/dfits.rom:rsc -MAP_TO_MODULE {[Register]}
    directive set /main_coeff_cost/get_coeff_cost/scan_i:offset.rom:rsc -MAP_TO_MODULE {[Register]}
    directive set /main_coeff_cost/get_coeff_cost/loops#1.rom:rsc -MAP_TO_MODULE {[Register]}
    directive set /main_coeff_cost/get_coeff_cost/fits.rom:rsc -MAP_TO_MODULE {[Register]}
    directive set /main_coeff_cost/get_coeff_cost/core/main -PIPELINE_INIT_INTERVAL 0
    directive set /main_coeff_cost/get_coeff_cost/core -DESIGN_GOAL latency
    directive set /main_coeff_cost/get_coeff_cost/scan_i:inv_g_sig_last_scan.rom:rsc -MAP_TO_MODULE {[Register]}
    directive set /main_coeff_cost/coeff_transpose_push/trans_conf:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
directive set /main_coeff_cost/coeff_transpose_pull/trans_conf:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
}
