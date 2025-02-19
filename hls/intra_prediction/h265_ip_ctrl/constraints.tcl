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
    directive set -CLOCKS {clk {-CLOCK_PERIOD 4.0 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 2.0 -RESET_SYNC_NAME rst -RESET_ASYNC_NAME arst_n -RESET_KIND async -RESET_SYNC_ACTIVE low -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high}}
    directive set -SCHED_USE_MULTICYCLE true
}

proc set_extract_directives {} {
    # Insert the list of extract directives
    directive set /main_ip_ctrl/ref0:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref1:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref2:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref3:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref4:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref5:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref6:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref7:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref8:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref9:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref10:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref11:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref12:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref13:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref14:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref15:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref16:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref17:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/ref18:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_ctrl/dist_thres.rom:rsc -MAP_TO_MODULE {[Register]}
    directive set /main_ip_ctrl/refs.rom:rsc -MAP_TO_MODULE {[Register]}
    directive set /main_ip_ctrl/core/main -PIPELINE_STALL_MODE flush
    directive set /main_ip_ctrl/conf_in:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_ip_ctrl/sad_config:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/planar_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/dc_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_pos_out2:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_pos_out3:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_pos_out4:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_pos_out5:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_pos_out6:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_pos_out7:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_pos_out8:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_pos_out9:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_zero_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_neg_out11:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_neg_out12:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_neg_out13:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_neg_out14:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_neg_out15:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_neg_out16:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_neg_out17:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/ang_neg_out18:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_ctrl/core/main -PIPELINE_INIT_INTERVAL 0
}
