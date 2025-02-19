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
    solution design set bin_coding -top
    solution design set cabac_write_main_state -block
    solution design set cabac_write_main_cmd -block
    solution design set cabac_bits -block
    solution design set cabac_low -block
    solution design set kvz_cabac_write -block
    solution design set kvz_bitstream_put_byte -block
    solution design set cabac_write_bitstream -block
    solution design set cabac_low_shift_add_buf -block
    solution design set kvz_bitstream_put_byte_buf -block
    solution design set cabac_write_buf -block
}
proc set_assembly_directives {} {
    # Insert the list of assembly directives
    directive set -CLOCKS {clk {-CLOCK_PERIOD 5.1 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 2.55 -RESET_SYNC_NAME rst -RESET_ASYNC_NAME arst_n -RESET_KIND async -RESET_SYNC_ACTIVE low -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high}}
}

proc set_extract_directives {} {
    # Insert the list of extract directives
    directive set /bin_coding/kvz_bitstream_put_byte_buf/core/main -PIPELINE_INIT_INTERVAL 1
    directive set /bin_coding/cabac_low_shift_add_buf/core/main -PIPELINE_INIT_INTERVAL 1
    directive set /bin_coding/bitstream_last_data:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /bin_coding/cabac_write_bitstream/bitstream_last_data:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /bin_coding/cabac_write_main_state/core/main -PIPELINE_STALL_MODE flush
    directive set /bin_coding/cabac_write_main_state/cabac_indexes.rom:rsc -MAP_TO_MODULE {[Register]}
    directive set /bin_coding/cabac_write_main_state/kvz_g_auc_next_state_lps.rom:rsc -MAP_TO_MODULE {[Register]}
    directive set /bin_coding/cabac_write_main_state/kvz_g_auc_next_state_mps.rom:rsc -MAP_TO_MODULE {[Register]}
    directive set /bin_coding/cabac_write_main_cmd/core/main -PIPELINE_STALL_MODE flush
    directive set /bin_coding/cabac_write_main_cmd/kvz_g_auc_renorm_table.rom:rsc -MAP_TO_MODULE {[Register]}
    directive set /bin_coding/cabac_write_main_cmd/kvz_g_auc_lpst_table.rom:rsc -MAP_TO_MODULE {[Register]}
    directive set /bin_coding/cabac_low_shift_add_buf/core/main -PIPELINE_STALL_MODE flush
    directive set /bin_coding/cabac_bits/core/main -PIPELINE_STALL_MODE flush
    directive set /bin_coding/cabac_low/core/main -PIPELINE_STALL_MODE flush
    directive set /bin_coding/kvz_cabac_write/core/main -PIPELINE_STALL_MODE flush
    directive set /bin_coding/kvz_bitstream_put_byte_buf/core/main -PIPELINE_STALL_MODE flush
    directive set /bin_coding/kvz_bitstream_put_byte/core/main -PIPELINE_STALL_MODE flush
    directive set /bin_coding/cabac_write_bitstream/core/main -PIPELINE_INIT_INTERVAL 2
    directive set /bin_coding/cabac_write_bitstream/core/main -PIPELINE_STALL_MODE flush
    directive set /bin_coding/cabac_states:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /bin_coding/cabac_write_main_state/core/main -PIPELINE_INIT_INTERVAL 2
    directive set /bin_coding/cabac_write_main_state/core -CLOCK_OVERHEAD 10.000000
    directive set /bin_coding/cabac_write_hier:cns -FIFO_DEPTH 0
    directive set /bin_coding/cabac_write_command1:cns -FIFO_DEPTH 0
    directive set /bin_coding/cabac_write_command2:cns -FIFO_DEPTH 0
    directive set /bin_coding/cabac_write_command3:cns -FIFO_DEPTH 0
    directive set /bin_coding/cabac_write_command4:cns -FIFO_DEPTH 0
    directive set /bin_coding/bitstream_put_byte1:cns -FIFO_DEPTH 0
    directive set /bin_coding/bitstream_put_byte2:cns -FIFO_DEPTH 0
    directive set /bin_coding/bs_data:cns -FIFO_DEPTH 0
    directive set /bin_coding/cabac_write_main_cmd/core -CLOCK_OVERHEAD 40.000000
    directive set /bin_coding/cabac_low/core -CLOCK_OVERHEAD 40.000000
    directive set /bin_coding/cabac_write_main_cmd/core/main -PIPELINE_INIT_INTERVAL 2
    directive set /bin_coding/cabac_bits/core/main -PIPELINE_INIT_INTERVAL 2
    directive set /bin_coding/cabac_low/core/main -PIPELINE_INIT_INTERVAL 2
    directive set /bin_coding/kvz_cabac_write/core/main -PIPELINE_INIT_INTERVAL 2
    directive set /bin_coding/kvz_bitstream_put_byte/core/main -PIPELINE_INIT_INTERVAL 2
    directive set /bin_coding/kvz_bitstream_put_byte/cabac_metas:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /bin_coding/kvz_bitstream_put_byte/cabac_metas:rsc -PACKING_MODE absolute
    directive set /bin_coding/cabac_metas:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /bin_coding/cabac_metas:rsc -PACKING_MODE absolute
    directive set /bin_coding/kvz_bitstream_put_byte/cabac_metas -WORD_WIDTH 128
    directive set /bin_coding/cabac_metas -WORD_WIDTH 128
    directive set /bin_coding/cabac_write_main_state/cabac_write:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /bin_coding/kvz_bitstream_put_byte/ctu_done:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /bin_coding/cabac_write:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /bin_coding/ctu_done:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /bin_coding/cabac_write_hier:cns -MAP_TO_MODULE ccs_ioport.ccs_pipe
    directive set /bin_coding/cabac_write_command1:cns -MAP_TO_MODULE ccs_ioport.ccs_pipe
    directive set /bin_coding/cabac_write_command2:cns -MAP_TO_MODULE ccs_ioport.ccs_pipe
    directive set /bin_coding/cabac_write_command3:cns -MAP_TO_MODULE ccs_ioport.ccs_pipe
    directive set /bin_coding/cabac_write_command4:cns -MAP_TO_MODULE ccs_ioport.ccs_pipe
    directive set /bin_coding/bitstream_put_byte1:cns -MAP_TO_MODULE ccs_ioport.ccs_pipe
    directive set /bin_coding/bitstream_put_byte2:cns -MAP_TO_MODULE ccs_ioport.ccs_pipe
    directive set /bin_coding/bs_data:cns -MAP_TO_MODULE ccs_ioport.ccs_pipe
}
