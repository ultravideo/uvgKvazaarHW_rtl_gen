//  Catapult Ultra Synthesis 10.5a/871028 (Production Release) Tue Apr 14 07:55:32 PDT 2020
//  
//  Copyright (c) Mentor Graphics Corporation, 1996-2020, All Rights Reserved.
//                        UNPUBLISHED, LICENSED SOFTWARE.
//             CONFIDENTIAL AND PROPRIETARY INFORMATION WHICH IS THE
//                 PROPERTY OF MENTOR GRAPHICS OR ITS LICENSORS
//  

//  
//  Package information: SIFLIBS v23.5_1.0, HLS_PKGS v23.5_1.0, 
//                       SIF_TOOLKITS v23.5_1.0, SIF_XILINX v23.5_1.0, 
//                       SIF_ALTERA v23.5_1.0, CCS_LIBS v23.5_1.0, 
//                       CDS_PPRO v10.3c_2, CDS_DesigChecker v10.5a, 
//                       CDS_OASYS v19.1_3.7, CDS_PSR v19.2_0.9, 
//                       DesignPad v2.78_1.0
//  
solution new -state initial
solution options defaults
solution options set /General/RestoreCWD true
solution options set /Interface/DefaultClockOverhead 40.0
solution options set /Interface/DefaultResetKind async
solution options set /Interface/DefaultSyncResetActive low
solution options set /Interface/DefaultResetClearsAllRegs no
solution options set /Interface/DefaultTransactionDoneSignal false
solution options set /Interface/DefaultChanIO lzvz
solution options set /Architectural/DefaultRegisterThreshold 1024
solution options set /Output/GenerateCycleNetlist false
solution file add tb_cabac_encode_intra_coding_unit_and_transform_coeff.c -type C++ -exclude true
solution file add main.c -type C++
solution file add cabac.h -type CHEADER
directive set -DESIGN_GOAL area
directive set -SPECULATE true
directive set -MERGEABLE true
directive set -MEM_MAP_THRESHOLD 32
directive set -LOGIC_OPT false
directive set -FSM_ENCODING none
directive set -FSM_BINARY_ENCODING_THRESHOLD 64
directive set -REG_MAX_FANOUT 0
directive set -NO_X_ASSIGNMENTS true
directive set -SAFE_FSM false
directive set -REGISTER_SHARING_MAX_WIDTH_DIFFERENCE 8
directive set -REGISTER_SHARING_LIMIT 0
directive set -ASSIGN_OVERHEAD 0
directive set -TIMING_CHECKS true
directive set -MUXPATH true
directive set -REALLOC true
directive set -UNROLL no
directive set -IO_MODE super
directive set -ARRAY_SIZE 1024
directive set -REGISTER_IDLE_SIGNAL false
directive set -IDLE_SIGNAL {}
directive set -STALL_FLAG false
directive set -DONE_FLAG {}
directive set -READY_FLAG {}
directive set -START_FLAG {}
directive set -OPT_CONST_MULTS use_library
directive set -CHARACTERIZE_ROM false
directive set -PROTOTYPE_ROM true
directive set -ROM_THRESHOLD 64
directive set -CLUSTER_ADDTREE_IN_WIDTH_THRESHOLD 0
directive set -CLUSTER_ADDTREE_IN_COUNT_THRESHOLD 0
directive set -CLUSTER_OPT_CONSTANT_INPUTS true
directive set -CLUSTER_RTL_SYN false
directive set -CLUSTER_FAST_MODE false
directive set -CLUSTER_TYPE combinational
directive set -PIPELINE_RAMP_UP true
go new
solution design set encode_intra_coding_unit_and_transform_coeff -block
solution design set encode_intra_coding_unit_and_transform_coeff_hier -top
solution design set cabac_main_config_dc -block
solution design set encode_intra_coding_unit_and_transform_coeff_hier -block
solution design set cabac_main_config_dc -block
solution design set encode_intra_coding_unit_and_transform_coeff -top
solution library add mgc_Altera-Arria-10-2_beh -- -rtlsyntool Quartus -manufacturer Altera -family {Arria 10} -speed 2 -part 10AX115N2F40I2SG
solution library add Altera_ROMS
solution library add Altera_DIST
solution library add Altera_FIFO
solution library add Altera_M20K
solution library add Altera_MLAB
go libraries
directive set -CLOCKS {clk {-CLOCK_PERIOD 6.67 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 3.335 -RESET_SYNC_NAME rst -RESET_ASYNC_NAME arst_n -RESET_KIND async -RESET_SYNC_ACTIVE low -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high} clk_cabac_main {-CLOCK_PERIOD 6.67 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 3.335 -RESET_SYNC_NAME clk_200_rst -RESET_ASYNC_NAME clk_cabac_main_arst_n -RESET_KIND async -RESET_SYNC_ACTIVE low -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high}}
directive set /encode_intra_coding_unit_and_transform_coeff/cabac_main_config_dc/core -CLOCK_NAME clk_cabac_main
directive set /encode_intra_coding_unit_and_transform_coeff/core -CLOCK_NAME clk_cabac_main
go assembly
directive set /encode_intra_coding_unit_and_transform_coeff/encode_intra_coding_unit_and_transform_coeff_hier/core/for#3:else#1:if#1:for -MERGEABLE true
directive set /encode_intra_coding_unit_and_transform_coeff/encode_intra_coding_unit_and_transform_coeff_hier/core/for#3:else#1:if#1:for -PIPELINE_INIT_INTERVAL 1
directive set /encode_intra_coding_unit_and_transform_coeff/encode_intra_coding_unit_and_transform_coeff_hier/kvz_part_mode_offsets.rom:rsc -MAP_TO_MODULE {[Register]}
directive set /encode_intra_coding_unit_and_transform_coeff/encode_intra_coding_unit_and_transform_coeff_hier/core/for#2 -PIPELINE_INIT_INTERVAL 2
directive set /encode_intra_coding_unit_and_transform_coeff/encode_intra_coding_unit_and_transform_coeff_hier/core/for#1 -PIPELINE_INIT_INTERVAL 1
directive set /encode_intra_coding_unit_and_transform_coeff/encode_intra_coding_unit_and_transform_coeff_hier/core/for -PIPELINE_INIT_INTERVAL 3
directive set /encode_intra_coding_unit_and_transform_coeff/lcu:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
directive set /encode_intra_coding_unit_and_transform_coeff/lcu_top:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
directive set /encode_intra_coding_unit_and_transform_coeff/lcu_left:rsc -MAP_TO_MODULE Altera_DIST.DIST_SP_WT
directive set /encode_intra_coding_unit_and_transform_coeff/cabac_main_config_hier:cns -FIFO_DEPTH 0
directive set /encode_intra_coding_unit_and_transform_coeff/cabac_main_config_dc/core/main -PIPELINE_INIT_INTERVAL 1
directive set /encode_intra_coding_unit_and_transform_coeff/cabac_main_config_dc/core/main -PIPELINE_STALL_MODE flush
directive set /encode_intra_coding_unit_and_transform_coeff/cabac_main_config_dc/cabac_main_config_in:rsc -FIFO_DEPTH -1
directive set /encode_intra_coding_unit_and_transform_coeff/encode_intra_coding_unit_and_transform_coeff_hier/cabac_main_config:rsc -FIFO_DEPTH -1
directive set /encode_intra_coding_unit_and_transform_coeff/encode_intra_coding_unit_and_transform_coeff_hier/cbf_masks.rom:rsc -MAP_TO_MODULE {[Register]}
directive set /encode_intra_coding_unit_and_transform_coeff/encode_intra_coding_unit_and_transform_coeff_hier/chroma_pred_modes.rom:rsc -MAP_TO_MODULE {[Register]}
directive set /encode_intra_coding_unit_and_transform_coeff/encode_intra_coding_unit_and_transform_coeff_hier -RESET_CLEARS_ALL_REGS no
directive set -RESET_CLEARS_ALL_REGS no
directive set /encode_intra_coding_unit_and_transform_coeff/encode_intra_coding_unit_and_transform_coeff_hier/lcu_top -WORD_WIDTH 8
directive set /encode_intra_coding_unit_and_transform_coeff/lcu_top -WORD_WIDTH 8
directive set /encode_intra_coding_unit_and_transform_coeff/encode_intra_coding_unit_and_transform_coeff_hier/lcu_left -WORD_WIDTH 8
directive set /encode_intra_coding_unit_and_transform_coeff/lcu_left -WORD_WIDTH 8
directive set /encode_intra_coding_unit_and_transform_coeff/encode_intra_coding_unit_and_transform_coeff_hier/core/for#3 -PIPELINE_INIT_INTERVAL 0
directive set /encode_intra_coding_unit_and_transform_coeff/encode_intra_coding_unit_and_transform_coeff_hier/core/for#3 -MERGEABLE true
go architect
go extract
