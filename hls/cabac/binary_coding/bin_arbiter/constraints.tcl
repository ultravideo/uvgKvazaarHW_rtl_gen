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
    directive set -CHAN_IO_PROTOCOL highfreq
    solution design set bin_arbiter -top
    solution design set cabac_write_buf -block
    solution design set cabac_write_encode_buf -block
    solution design set cabac_write_coding_tree_buf -block
    solution design set cabac_write_demux -block
    solution design set cabac_write_dc_buf -block
}
proc set_assembly_directives {} {
    # Insert the list of assembly directives
    directive set /bin_arbiter/cabac_write_buf/core -CLOCK_NAME clk_cabac_main
    directive set /bin_arbiter/core -CLOCK_NAME clk
    directive set /bin_arbiter/cabac_write_encode_buf/core -CLOCK_NAME clk
    directive set /bin_arbiter/cabac_write_dc_buf/core -CLOCK_NAME clk_cabac_state
    directive set /bin_arbiter/cabac_write_demux/core -CLOCK_NAME clk_cabac_main
    directive set -CLOCKS {clk {-CLOCK_PERIOD 6.67 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 3.335 -RESET_SYNC_NAME rst -RESET_ASYNC_NAME arst_n -RESET_KIND async -RESET_SYNC_ACTIVE low -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high} clk_cabac_state {-CLOCK_PERIOD 5.0 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 2.5 -RESET_SYNC_NAME clk_200_rst -RESET_ASYNC_NAME clk_cabac_state_arst_n -RESET_KIND async -RESET_SYNC_ACTIVE low -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high} clk_cabac_main {-CLOCK_PERIOD 3.33 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 1.665 -RESET_SYNC_NAME clk_cabac_main_rst -RESET_ASYNC_NAME clk_cabac_main_arst_n -RESET_KIND async -RESET_SYNC_ACTIVE low -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high}}
}

proc set_extract_directives {} {
    # Insert the list of extract directives
    directive set /bin_arbiter/cabac_write_encode_b:cns -FIFO_DEPTH 0
    directive set /bin_arbiter/cabac_write_buf/core/main -PIPELINE_INIT_INTERVAL 1
    directive set /bin_arbiter/cabac_write_buf/core/main -PIPELINE_STALL_MODE flush
    directive set /bin_arbiter/cabac_write_encode_buf/core/main -PIPELINE_STALL_MODE flush
    directive set /bin_arbiter/cabac_write_coding_tree_buf/core/main -PIPELINE_STALL_MODE flush
    directive set /bin_arbiter/cabac_write_demux/core/main -PIPELINE_INIT_INTERVAL 1
    directive set /bin_arbiter/cabac_write_demux/core/main -PIPELINE_STALL_MODE flush
    directive set /bin_arbiter/cabac_write_dc_buf/core/main -PIPELINE_INIT_INTERVAL 1
    directive set /bin_arbiter/cabac_write_dc_buf/core/main -PIPELINE_STALL_MODE flush
    directive set /bin_arbiter/cabac_write_coding_tree_buf/core/main -PIPELINE_INIT_INTERVAL 0
    directive set /bin_arbiter/cabac_write_b:cns -FIFO_DEPTH 1
    directive set /bin_arbiter/cabac_write_buf/cabac_write_out:rsc -FIFO_DEPTH -1
    directive set /bin_arbiter/cabac_write_demux/cabac_write:rsc -FIFO_DEPTH -1
    directive set /bin_arbiter/cabac_write_b:cns -MAP_TO_MODULE mgc_ioport.mgc_pipe
    directive set /bin_arbiter/cabac_write_encode_buf/core/main -PIPELINE_INIT_INTERVAL 1
    directive set /bin_arbiter/cabac_write_encode_buf/cabac_write_encode:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /bin_arbiter/cabac_write_buf/cabac_write:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /bin_arbiter/cabac_write_dc_buf/cabac_write_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /bin_arbiter/cabac_write_coding_tree_buf/cabac_write_coding_tree:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /bin_arbiter/cabac_write_coding_tree_buf/stall_coding_tree:rsc -MAP_TO_MODULE ccs_ioport.ccs_in
    directive set /bin_arbiter/cabac_write:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /bin_arbiter/cabac_write_encode:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /bin_arbiter/cabac_write_coding_tree:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /bin_arbiter/cabac_write_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /bin_arbiter/stall_coding_tree:rsc -MAP_TO_MODULE ccs_ioport.ccs_in
    directive set /bin_arbiter/cabac_write_encode_buf/cabac_write_out:rsc -MAP_TO_MODULE mgc_ioport.mgc_out_stdreg_wait
    directive set /bin_arbiter/cabac_write_demux/cabac_write_encode:rsc -MAP_TO_MODULE mgc_ioport.mgc_chan_in
    directive set /bin_arbiter/cabac_write_demux/cabac_write_coding_tree:rsc -MAP_TO_MODULE mgc_ioport.mgc_chan_in
    directive set /bin_arbiter/cabac_write_coding_tree_buf/cabac_write_out:rsc -MAP_TO_MODULE mgc_ioport.mgc_out_stdreg_wait
    directive set /bin_arbiter/cabac_write_encode_b:cns -MAP_TO_MODULE mgc_ioport.mgc_pipe_regfile_cdc
    directive set /bin_arbiter/cabac_write_coding_tree_b:cns -MAP_TO_MODULE mgc_ioport.mgc_pipe_regfile_cdc
    directive set /bin_arbiter/cabac_write_demux/cabac_write_coding_tree:rsc -FIFO_DEPTH 0
    directive set /bin_arbiter/cabac_write_coding_tree_buf/cabac_write_out:rsc -FIFO_DEPTH 0
    directive set /bin_arbiter/cabac_write_coding_tree_b:cns -FIFO_DEPTH 0
    directive set /bin_arbiter/cabac_write_dc_buf/cabac_write_in:rsc -MAP_TO_MODULE mgc_ioport.mgc_chan_in
    directive set /bin_arbiter/cabac_write_demux/cabac_write_out:rsc -MAP_TO_MODULE mgc_ioport.mgc_out_stdreg_wait
    directive set /bin_arbiter/cabac_write_demux/cabac_write:rsc -MAP_TO_MODULE mgc_ioport.mgc_chan_in
    directive set /bin_arbiter/cabac_write_dc_buf/cabac_write_in:rsc -FIFO_DEPTH 16
    directive set /bin_arbiter/cabac_write_demux/cabac_write_out:rsc -FIFO_DEPTH 16
    directive set /bin_arbiter/cabac_write_hier:cns -FIFO_DEPTH 16
    directive set /bin_arbiter/cabac_write_hier:cns -MAP_TO_MODULE mgc_ioport.mgc_pipe_regfile_cdc
    directive set /bin_arbiter/cabac_write_buf/cabac_write_out:rsc -MAP_TO_MODULE mgc_ioport.mgc_out_stdreg_wait
}
