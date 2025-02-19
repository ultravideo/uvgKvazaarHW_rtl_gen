#############################################################################
# This file is part of Kvazaar HEVC encoder.
#
# Copyright (C) 2013-2014 Tampere University of Technology and others (see
# COPYING file).
#
# Kvazaar is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as published
# by the Free Software Foundation.
#
# Kvazaar is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Kvazaar.  If not, see <http://www.gnu.org/licenses/>.
#############################################################################

# List of constraints
proc set_analyse_directives {} {
    # Insert the list of analyse directives
    directive set -TRANSACTION_DONE_SIGNAL false
    directive set -RESET_CLEARS_ALL_REGS no
    directive set -CLOCK_OVERHEAD 20.000000
}
proc set_assembly_directives {} {
    # Insert the list of assembly directives
    directive set -CLOCKS {clk {-CLOCK_PERIOD 4.0 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 2.0 -RESET_SYNC_NAME rst -RESET_ASYNC_NAME arst_n -RESET_KIND async -RESET_SYNC_ACTIVE low -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high}}
    directive set -DSP_EXTRACTION yes
}

proc set_extract_directives {} {
    # Insert the list of extract directives
    directive set /main_ip_get_ang_zero/ref:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
    directive set /main_ip_get_ang_zero/core/main -PIPELINE_INIT_INTERVAL 1
    directive set /main_ip_get_ang_zero/core/main -PIPELINE_STALL_MODE flush
    directive set /main_ip_get_ang_zero/ref:rsc -INTERLEAVE 2
    directive set /main_ip_get_ang_zero/conf_in:rsc -MAP_TO_MODULE ccs_ioport.ccs_in_wait
    directive set /main_ip_get_ang_zero/hor_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
    directive set /main_ip_get_ang_zero/ver_out:rsc -MAP_TO_MODULE ccs_ioport.ccs_out_wait
}
