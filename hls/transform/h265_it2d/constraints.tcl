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
    directive set -DESIGN_GOAL latency
    directive set -CLOCK_OVERHEAD 40.000000
    solution design set it2d_transpose_push -block
    solution design set it2d_transpose_pull -block
    solution design set it2d_transpose_pipe -block
    solution design set it2d -top
    directive set -OPT_CONST_MULTS full
}
proc set_assembly_directives {} {
    # Insert the list of assembly directives
    directive set -CLOCKS {clk {-CLOCK_PERIOD 4.0 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 2.0 -RESET_SYNC_NAME rst -RESET_ASYNC_NAME arst_n -RESET_KIND async -RESET_SYNC_ACTIVE low -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high}}
    directive set -DSP_EXTRACTION yes       
}

proc set_extract_directives {} {
    # Insert the list of extract directives    
    directive set /it2d/it2d_idct<1> -DSP_EXTRACTION_OPTIONS {muladd mulacc mul2add mul2acc mul3add mul3acc mul4add mul4acc}
    directive set /it2d/it2d_idct<0> -DSP_EXTRACTION_OPTIONS {muladd mulacc mul2add mul2acc mul3add mul3acc mul4add mul4acc}
    directive set /it2d/it2d_idst<0> -DSP_EXTRACTION_OPTIONS {mul3add mul3acc mul4add mul4acc}
    directive set /it2d/it2d_idst<0>/core/main_idst:for -PIPELINE_INIT_INTERVAL 0
    directive set /it2d/it2d_transpose_push/core/ctl::RotateL<512,16>:r:rsc -MAP_TO_MODULE {[Register]}
    directive set /it2d/it2d_transpose_pull/core/ctl::RotateR<512,16>:r:rsc -MAP_TO_MODULE {[Register]}
    directive set /it2d/it2d_transpose_pipe/trans_push_conf:cns -FIFO_DEPTH 0
    directive set /it2d/it2d_transpose_push/trans_push_conf:rsc -FIFO_DEPTH 0
    directive set /it2d/it2d_transpose_pull/trans_conf:rsc -FIFO_DEPTH 0
    directive set /it2d/u_from_idct_2nd:cns -FIFO_DEPTH 0
    directive set /it2d/from_idct_2nd:cns -FIFO_DEPTH 0
    directive set /it2d/to_idct_2nd:cns -FIFO_DEPTH 0
    directive set /it2d/from_idct_1st:cns -FIFO_DEPTH 0
    directive set /it2d/u_from_idct_1st:cns -FIFO_DEPTH 0
    directive set /it2d/to_idct_trans:cns -FIFO_DEPTH 0
    directive set /it2d/from_idst:cns -FIFO_DEPTH 0
    directive set /it2d/to_idct_1st:cns -FIFO_DEPTH 0
    directive set /it2d/to_idst:cns -FIFO_DEPTH 0
    directive set /it2d/conf_out:cns -FIFO_DEPTH 0
    directive set /it2d/conf_2nd:cns -FIFO_DEPTH 0
    directive set /it2d/trans_conf:cns -FIFO_DEPTH 0
    directive set /it2d/trans_push:rsc -GEN_EXTERNAL_ENABLE true
    directive set /it2d/it2d_transpose_pipe<0>/trans_push:rsc -GEN_EXTERNAL_ENABLE true
    directive set /it2d/it2d_transpose_pipe/trans_push:rsc -GEN_EXTERNAL_ENABLE true
    directive set /it2d/it2d_transpose_push/trans:rsc -GEN_EXTERNAL_ENABLE true
    directive set /it2d/trans_pull:rsc -GEN_EXTERNAL_ENABLE true
    directive set /it2d/it2d_transpose_pipe<0>/trans_pull:rsc -GEN_EXTERNAL_ENABLE true
    directive set /it2d/it2d_transpose_pipe/trans_pull:rsc -GEN_EXTERNAL_ENABLE true
    directive set /it2d/it2d_transpose_pull/trans:rsc -GEN_EXTERNAL_ENABLE true
    directive set /it2d/trans_pull:rsc -INTERLEAVE 32
    directive set /it2d/trans_pull:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /it2d/it2d_transpose_pipe<0>/trans_pull:rsc -INTERLEAVE 32
    directive set /it2d/it2d_transpose_pipe<0>/trans_pull:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /it2d/it2d_transpose_pipe/trans_pull:rsc -INTERLEAVE 32
    directive set /it2d/it2d_transpose_pipe/trans_pull:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /it2d/it2d_transpose_pull/trans:rsc -INTERLEAVE 32
    directive set /it2d/it2d_transpose_pull/trans:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /it2d/trans_push:rsc -INTERLEAVE 32
    directive set /it2d/trans_push:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /it2d/it2d_transpose_pipe<0>/trans_push:rsc -INTERLEAVE 32
    directive set /it2d/it2d_transpose_pipe<0>/trans_push:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /it2d/it2d_transpose_pipe/trans_push:rsc -INTERLEAVE 32
    directive set /it2d/it2d_transpose_pipe/trans_push:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /it2d/it2d_transpose_push/trans:rsc -INTERLEAVE 32
    directive set /it2d/it2d_transpose_push/trans:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /it2d/it2d_idct<1>/core/main_idct:src:rsc -MAP_TO_MODULE {[Register]}
    directive set /it2d/it2d_idct<0>/core/main_idct:src:rsc -MAP_TO_MODULE {[Register]}
    directive set /it2d/it2d_out/t2d_out_conf:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/it2d_out/coeff_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /it2d/it2d_out/idst_in:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/it2d_out/idct_in:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/it2d_idct<1>/input_port:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/it2d_idct<1>/output_1st:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /it2d/it2d_idct<1>/output_2nd:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /it2d/it2d_2nd_pass/t2d_2nd_conf:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/it2d_2nd_pass/from_idct:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/it2d_2nd_pass/to_idct:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /it2d/it2d_transpose_pull/trans_conf:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/it2d_transpose_pull/output_port:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /it2d/it2d_transpose_push/trans_conf:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/it2d_transpose_push/trans_push_conf:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /it2d/it2d_transpose_push/input_port:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/it2d_transpose_pipe/trans_conf:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/it2d_transpose_pipe/input_port:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/it2d_transpose_pipe/output_port:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /it2d/it2d_transpose_pipe<0>/trans_conf:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/it2d_transpose_pipe<0>/input_port:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/it2d_transpose_pipe<0>/output_port:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /it2d/it2d_idct<0>/input_port:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/it2d_idct<0>/output_1st:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /it2d/it2d_idct<0>/output_2nd:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /it2d/it2d_idst<0>/input_port:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/it2d_idst<0>/output_port:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /it2d/it2d_control/coeff_in:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/it2d_control/trans_conf:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /it2d/it2d_control/t2d_2nd_conf:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /it2d/it2d_control/t2d_out_conf:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /it2d/it2d_control/to_idst:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /it2d/it2d_control/to_idct:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /it2d/coeff_in:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /it2d/coeff_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
}
