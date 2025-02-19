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
    directive set -CLOCKS {clk {-CLOCK_PERIOD 4.55 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 2.275 -RESET_SYNC_NAME rst -RESET_ASYNC_NAME arst_n -RESET_KIND async -RESET_SYNC_ACTIVE low -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high}}
    directive set -DSP_EXTRACTION yes
}

proc set_extract_directives {} {
    # Insert the list of extract directives
    directive set /main_sad_parallel/main_sad_parallel_hier/lcu.ref:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_sad_parallel/lcu.ref:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_sad_parallel/pre_calc/core/main -PIPELINE_INIT_INTERVAL 1
    directive set /main_sad_parallel/pre_calc/core/main -PIPELINE_STALL_MODE flush
    directive set /main_sad_parallel/main_sad_parallel_hier/core/main -PIPELINE_INIT_INTERVAL 1
    directive set /main_sad_parallel/main_sad_parallel_hier/core/costs:rsc -MAP_TO_MODULE {[Register]}
    directive set /main_sad_parallel/main_sad_parallel_hier/core/costs:rsc -GEN_EXTERNAL_ENABLE false
    directive set /main_sad_parallel/main_sad_parallel_hier/inputs.rom:rsc -MAP_TO_MODULE {[Register]}
    directive set /main_sad_parallel/main_sad_parallel_hier/core/main -PIPELINE_STALL_MODE flush
    directive set /main_sad_parallel/pre_calc/conf_in:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/conf_in:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/pred_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_sad_parallel/conf_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_sad_parallel/intra_mode_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_sad_parallel/in0:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in1:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in2:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in3:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in4:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in5:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in6:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in7:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in8:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in9:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in10:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in11:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in12:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in13:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in14:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in15:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in16:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in17:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in18:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in19:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in20:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in21:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in22:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in23:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in24:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in25:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in26:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in27:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in28:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in29:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in30:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in31:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in32:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in33:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/in34:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/main_sad_parallel_hier/cost_init:rsc -FIFO_DEPTH 1
    directive set /main_sad_parallel/main_sad_parallel_hier/cost_init:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_sad_parallel/pre_calc/cost_init:rsc -FIFO_DEPTH 1
    directive set /main_sad_parallel/pre_calc/cost_init:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_sad_parallel/cost_init:cns -FIFO_DEPTH 1
    directive set /main_sad_parallel/cost_init:cns -MAP_TO_MODULE ccs_ioport.ccs_pipe
}
