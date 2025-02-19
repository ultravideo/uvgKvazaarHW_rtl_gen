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
}

proc set_extract_directives {} {
    # Insert the list of extract directives
    directive set /main_indexer_top/cabac_states_arbiter/core/if:for -PIPELINE_INIT_INTERVAL 1
    directive set /main_indexer_top/cabac_states_arbiter/core/main -PIPELINE_INIT_INTERVAL 0
    directive set /main_indexer_top/mem_out_btm_top:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_indexer_top/main_indexer_in/mem_out_btm_top:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_indexer_top/mem_out:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_indexer_top/main_indexer_in/mem_out:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_indexer_top/bitstream_last_data_top:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_indexer_top/cabac_metas2_top -WORD_WIDTH 128
    directive set /main_indexer_top/cabac_metas2_top:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_indexer_top/cabac_states_top:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_indexer_top/main_indexer_out/bitstream_last_data_top:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_indexer_top/main_indexer_out/cabac_metas_top -WORD_WIDTH 128
    directive set /main_indexer_top/main_indexer_out/cabac_metas_top:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_indexer_top/cabac_states_arbiter/cabac_states_top:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_indexer_top/rec:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_indexer_top/cu_info:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_indexer_top/cabac_metas2:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_indexer_top/bitstream_last_data:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_indexer_top/cabac_states_arbiter/cabac_states:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_indexer_top/cabac_states:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_indexer_top/cabac_states_arbiter/core/main -PIPELINE_STALL_MODE flush
    directive set /main_indexer_top/main_indexer_out/core/main -PIPELINE_INIT_INTERVAL 1
    directive set /main_indexer_top/main_indexer_out/core/main -PIPELINE_STALL_MODE flush
    directive set /main_indexer_top/main_indexer_in/core/main -PIPELINE_INIT_INTERVAL 1
    directive set /main_indexer_top/main_indexer_in/core/main -PIPELINE_STALL_MODE flush
    directive set /main_indexer_top/main_indexer_out/data_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_indexer_top/main_indexer_out/data_req:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_indexer_top/main_indexer_in/data_in:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_indexer_top/data_in:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_indexer_top/data_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_indexer_top/data_req:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_indexer_top/main_indexer_out/data_req_hier:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_indexer_top/cabac_states_arbiter/core -CLOCK_OVERHEAD 40.000000
    directive set /main_indexer_top/main_indexer_out/core -CLOCK_OVERHEAD 40.000000
    directive set /main_indexer_top/main_indexer_in/core -CLOCK_OVERHEAD 40.000000
    directive set /main_indexer_top/main_indexer_out/data_req:rsc -FIFO_DEPTH 16
    directive set /main_indexer_top/cabac_states_arbiter/data_update:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait_szchan
    directive set /main_indexer_top/cabac_states_arbiter/data_req:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait_szchan
    directive set /main_indexer_top/cabac_states_arbiter/data_req:rsc -FIFO_DEPTH 1
    directive set /main_indexer_top/main_indexer_out/data_req_hier:rsc -FIFO_DEPTH 1
    directive set /main_indexer_top/cabac_states_arbiter/core/else:if:for -PIPELINE_INIT_INTERVAL 1
    directive set /main_indexer_top/cabac_states_arbiter/data_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_buf_wait
    directive set /main_indexer_top/main_indexer_out/cabac_states_pull:rsc -MAP_TO_MODULE ccs_ioport.ccs_ctrl_in_buf_wait
    directive set /main_indexer_top/cabac_states_arbiter/data_out:rsc -FIFO_DEPTH 0
    directive set /main_indexer_top/main_indexer_out/cabac_states_pull:rsc -FIFO_DEPTH 0
    directive set /main_indexer_top/cabac_states_pull:cns -FIFO_DEPTH 0
    directive set /main_indexer_top/main_indexer_in/cabac_states_push:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_buf_wait
    directive set /main_indexer_top/cabac_states_push:cns -FIFO_DEPTH 2
    directive set /main_indexer_top/cabac_states_arbiter/data_update:rsc -FIFO_DEPTH 2
    directive set /main_indexer_top/main_indexer_in/cabac_states_push:rsc -FIFO_DEPTH 2
}
