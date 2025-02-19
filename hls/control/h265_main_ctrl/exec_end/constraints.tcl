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
    directive set -DESIGN_GOAL latency
}
proc set_assembly_directives {} {
    # Insert the list of assembly directives
    directive set -CLOCKS {clk {-CLOCK_PERIOD 4.0 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 2.0 -RESET_SYNC_NAME rst -RESET_ASYNC_NAME arst_n -RESET_KIND async -RESET_SYNC_ACTIVE low -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high}}
}

proc set_extract_directives {} {
    # Insert the list of extract directives
    directive set /exec_end/cu.cu.cu_info:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /exec_end/cu.cu_hor.cu_info:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /exec_end/cu.cu_ver.cu_info:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /exec_end/core/stack:rsc -EXTERNAL_MEMORY true
    directive set /exec_end/cabac:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /exec_end/cabac -WORD_WIDTH 128
    directive set /exec_end/core/case-0:for -PIPELINE_INIT_INTERVAL 2
    directive set /exec_end/core/stack:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /exec_end/ctu_end:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /exec_end/ctu_end:rsc -PACKING_MODE absolute
    directive set /exec_end/ctu_end -WORD_WIDTH 16
    directive set /exec_end/inst_input:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /exec_end/result_in:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /exec_end/buffer:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
}
