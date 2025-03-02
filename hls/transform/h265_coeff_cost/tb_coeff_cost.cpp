/*****************************************************************************
 * This file is part of uvgKvazaarHW.
 *
 * Copyright (c) 2025, Tampere University, ITU/ISO/IEC, project contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * * Neither the name of the Tampere University or ITU/ISO/IEC nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * INCLUDING NEGLIGENCE OR OTHERWISE ARISING IN ANY WAY OUT OF THE USE OF THIS
 ****************************************************************************/

#include "global.h"
#include <cmath>
#include <iostream>
#include <mc_scverify.h>
using namespace std;

typedef int_16 coeff_t;

#define SCAN_DIAG 0
#define SCAN_HOR 1
#define SCAN_VER 2

const int_8 kvz_g_convert_to_bit[LCU_WIDTH + 1] = {-1, -1, -1, -1, 0,  -1, -1, -1, 1,  -1, -1, -1, -1, -1, -1, -1, 2,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3,
                                                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 4};
static const uint_32 g_sig_last_scan_0_0[4] = {0, 2, 1, 3};
static const uint_32 g_sig_last_scan_1_0[4] = {0, 1, 2, 3};
static const uint_32 g_sig_last_scan_2_0[4] = {0, 2, 1, 3};
static const uint_32 g_sig_last_scan_0_1[16] = {0, 4, 1, 8, 5, 2, 12, 9, 6, 3, 13, 10, 7, 14, 11, 15};
static const uint_32 g_sig_last_scan_1_1[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
static const uint_32 g_sig_last_scan_2_1[16] = {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15};
static const uint_32 g_sig_last_scan_0_2[64] = {0, 8,  1, 16, 9,  2, 24, 17, 10, 3, 25, 18, 11, 26, 19, 27, 32, 40, 33, 48, 41, 34, 56, 49, 42, 35, 57, 50, 43, 58, 51, 59,
                                                4, 12, 5, 20, 13, 6, 28, 21, 14, 7, 29, 22, 15, 30, 23, 31, 36, 44, 37, 52, 45, 38, 60, 53, 46, 39, 61, 54, 47, 62, 55, 63};
static const uint_32 g_sig_last_scan_1_2[64] = {0,  1,  2,  3,  8,  9,  10, 11, 16, 17, 18, 19, 24, 25, 26, 27, 4,  5,  6,  7,  12, 13, 14, 15, 20, 21, 22, 23, 28, 29, 30, 31,
                                                32, 33, 34, 35, 40, 41, 42, 43, 48, 49, 50, 51, 56, 57, 58, 59, 36, 37, 38, 39, 44, 45, 46, 47, 52, 53, 54, 55, 60, 61, 62, 63};
static const uint_32 g_sig_last_scan_2_2[64] = {0, 8,  16, 24, 1, 9,  17, 25, 2, 10, 18, 26, 3, 11, 19, 27, 32, 40, 48, 56, 33, 41, 49, 57, 34, 42, 50, 58, 35, 43, 51, 59,
                                                4, 12, 20, 28, 5, 13, 21, 29, 6, 14, 22, 30, 7, 15, 23, 31, 36, 44, 52, 60, 37, 45, 53, 61, 38, 46, 54, 62, 39, 47, 55, 63};
static const uint_32 g_sig_last_scan_0_3[256] = {
    0,   16,  1,   32,  17,  2,   48,  33,  18,  3,   49,  34,  19,  50,  35,  51,  64,  80,  65,  96,  81,  66,  112, 97,  82,  67,  113, 98,  83,  114, 99,  115, 4,   20,  5,   36,  21,
    6,   52,  37,  22,  7,   53,  38,  23,  54,  39,  55,  128, 144, 129, 160, 145, 130, 176, 161, 146, 131, 177, 162, 147, 178, 163, 179, 68,  84,  69,  100, 85,  70,  116, 101, 86,  71,
    117, 102, 87,  118, 103, 119, 8,   24,  9,   40,  25,  10,  56,  41,  26,  11,  57,  42,  27,  58,  43,  59,  192, 208, 193, 224, 209, 194, 240, 225, 210, 195, 241, 226, 211, 242, 227,
    243, 132, 148, 133, 164, 149, 134, 180, 165, 150, 135, 181, 166, 151, 182, 167, 183, 72,  88,  73,  104, 89,  74,  120, 105, 90,  75,  121, 106, 91,  122, 107, 123, 12,  28,  13,  44,
    29,  14,  60,  45,  30,  15,  61,  46,  31,  62,  47,  63,  196, 212, 197, 228, 213, 198, 244, 229, 214, 199, 245, 230, 215, 246, 231, 247, 136, 152, 137, 168, 153, 138, 184, 169, 154,
    139, 185, 170, 155, 186, 171, 187, 76,  92,  77,  108, 93,  78,  124, 109, 94,  79,  125, 110, 95,  126, 111, 127, 200, 216, 201, 232, 217, 202, 248, 233, 218, 203, 249, 234, 219, 250,
    235, 251, 140, 156, 141, 172, 157, 142, 188, 173, 158, 143, 189, 174, 159, 190, 175, 191, 204, 220, 205, 236, 221, 206, 252, 237, 222, 207, 253, 238, 223, 254, 239, 255};
static const uint_32 g_sig_last_scan_1_3[256] = {
    0,   1,   2,   3,   16,  17,  18,  19,  32,  33,  34,  35,  48,  49,  50,  51,  4,   5,   6,   7,   20,  21,  22,  23,  36,  37,  38,  39,  52,  53,  54,  55,  8,   9,   10,  11,  24,
    25,  26,  27,  40,  41,  42,  43,  56,  57,  58,  59,  12,  13,  14,  15,  28,  29,  30,  31,  44,  45,  46,  47,  60,  61,  62,  63,  64,  65,  66,  67,  80,  81,  82,  83,  96,  97,
    98,  99,  112, 113, 114, 115, 68,  69,  70,  71,  84,  85,  86,  87,  100, 101, 102, 103, 116, 117, 118, 119, 72,  73,  74,  75,  88,  89,  90,  91,  104, 105, 106, 107, 120, 121, 122,
    123, 76,  77,  78,  79,  92,  93,  94,  95,  108, 109, 110, 111, 124, 125, 126, 127, 128, 129, 130, 131, 144, 145, 146, 147, 160, 161, 162, 163, 176, 177, 178, 179, 132, 133, 134, 135,
    148, 149, 150, 151, 164, 165, 166, 167, 180, 181, 182, 183, 136, 137, 138, 139, 152, 153, 154, 155, 168, 169, 170, 171, 184, 185, 186, 187, 140, 141, 142, 143, 156, 157, 158, 159, 172,
    173, 174, 175, 188, 189, 190, 191, 192, 193, 194, 195, 208, 209, 210, 211, 224, 225, 226, 227, 240, 241, 242, 243, 196, 197, 198, 199, 212, 213, 214, 215, 228, 229, 230, 231, 244, 245,
    246, 247, 200, 201, 202, 203, 216, 217, 218, 219, 232, 233, 234, 235, 248, 249, 250, 251, 204, 205, 206, 207, 220, 221, 222, 223, 236, 237, 238, 239, 252, 253, 254, 255};
static const uint_32 g_sig_last_scan_2_3[256] = {
    0,   16,  32,  48,  1,   17,  33,  49,  2,   18,  34,  50,  3,   19,  35,  51,  64,  80,  96,  112, 65,  81,  97,  113, 66,  82,  98,  114, 67,  83,  99,  115, 128, 144, 160, 176, 129,
    145, 161, 177, 130, 146, 162, 178, 131, 147, 163, 179, 192, 208, 224, 240, 193, 209, 225, 241, 194, 210, 226, 242, 195, 211, 227, 243, 4,   20,  36,  52,  5,   21,  37,  53,  6,   22,
    38,  54,  7,   23,  39,  55,  68,  84,  100, 116, 69,  85,  101, 117, 70,  86,  102, 118, 71,  87,  103, 119, 132, 148, 164, 180, 133, 149, 165, 181, 134, 150, 166, 182, 135, 151, 167,
    183, 196, 212, 228, 244, 197, 213, 229, 245, 198, 214, 230, 246, 199, 215, 231, 247, 8,   24,  40,  56,  9,   25,  41,  57,  10,  26,  42,  58,  11,  27,  43,  59,  72,  88,  104, 120,
    73,  89,  105, 121, 74,  90,  106, 122, 75,  91,  107, 123, 136, 152, 168, 184, 137, 153, 169, 185, 138, 154, 170, 186, 139, 155, 171, 187, 200, 216, 232, 248, 201, 217, 233, 249, 202,
    218, 234, 250, 203, 219, 235, 251, 12,  28,  44,  60,  13,  29,  45,  61,  14,  30,  46,  62,  15,  31,  47,  63,  76,  92,  108, 124, 77,  93,  109, 125, 78,  94,  110, 126, 79,  95,
    111, 127, 140, 156, 172, 188, 141, 157, 173, 189, 142, 158, 174, 190, 143, 159, 175, 191, 204, 220, 236, 252, 205, 221, 237, 253, 206, 222, 238, 254, 207, 223, 239, 255};
static const uint_32 g_sig_last_scan_0_4[1024] = {
    0,   32,  1,    64,  33,  2,    96,   65,   34,  3,   97,   66,  35,  98,   67,   99,   128, 160,  129, 192,  161, 130, 224, 193, 162, 131, 225, 194, 163, 226, 195, 227, 4,   36,  5,    68,
    37,  6,   100,  69,  38,  7,    101,  70,   39,  102, 71,   103, 256, 288,  257,  320,  289, 258,  352, 321,  290, 259, 353, 322, 291, 354, 323, 355, 132, 164, 133, 196, 165, 134, 228,  197,
    166, 135, 229,  198, 167, 230,  199,  231,  8,   40,  9,    72,  41,  10,   104,  73,   42,  11,   105, 74,   43,  106, 75,  107, 384, 416, 385, 448, 417, 386, 480, 449, 418, 387, 481,  450,
    419, 482, 451,  483, 260, 292,  261,  324,  293, 262, 356,  325, 294, 263,  357,  326,  295, 358,  327, 359,  136, 168, 137, 200, 169, 138, 232, 201, 170, 139, 233, 202, 171, 234, 203,  235,
    12,  44,  13,   76,  45,  14,   108,  77,   46,  15,  109,  78,  47,  110,  79,   111,  512, 544,  513, 576,  545, 514, 608, 577, 546, 515, 609, 578, 547, 610, 579, 611, 388, 420, 389,  452,
    421, 390, 484,  453, 422, 391,  485,  454,  423, 486, 455,  487, 264, 296,  265,  328,  297, 266,  360, 329,  298, 267, 361, 330, 299, 362, 331, 363, 140, 172, 141, 204, 173, 142, 236,  205,
    174, 143, 237,  206, 175, 238,  207,  239,  16,  48,  17,   80,  49,  18,   112,  81,   50,  19,   113, 82,   51,  114, 83,  115, 640, 672, 641, 704, 673, 642, 736, 705, 674, 643, 737,  706,
    675, 738, 707,  739, 516, 548,  517,  580,  549, 518, 612,  581, 550, 519,  613,  582,  551, 614,  583, 615,  392, 424, 393, 456, 425, 394, 488, 457, 426, 395, 489, 458, 427, 490, 459,  491,
    268, 300, 269,  332, 301, 270,  364,  333,  302, 271, 365,  334, 303, 366,  335,  367,  144, 176,  145, 208,  177, 146, 240, 209, 178, 147, 241, 210, 179, 242, 211, 243, 20,  52,  21,   84,
    53,  22,  116,  85,  54,  23,   117,  86,   55,  118, 87,   119, 768, 800,  769,  832,  801, 770,  864, 833,  802, 771, 865, 834, 803, 866, 835, 867, 644, 676, 645, 708, 677, 646, 740,  709,
    678, 647, 741,  710, 679, 742,  711,  743,  520, 552, 521,  584, 553, 522,  616,  585,  554, 523,  617, 586,  555, 618, 587, 619, 396, 428, 397, 460, 429, 398, 492, 461, 430, 399, 493,  462,
    431, 494, 463,  495, 272, 304,  273,  336,  305, 274, 368,  337, 306, 275,  369,  338,  307, 370,  339, 371,  148, 180, 149, 212, 181, 150, 244, 213, 182, 151, 245, 214, 183, 246, 215,  247,
    24,  56,  25,   88,  57,  26,   120,  89,   58,  27,  121,  90,  59,  122,  91,   123,  896, 928,  897, 960,  929, 898, 992, 961, 930, 899, 993, 962, 931, 994, 963, 995, 772, 804, 773,  836,
    805, 774, 868,  837, 806, 775,  869,  838,  807, 870, 839,  871, 648, 680,  649,  712,  681, 650,  744, 713,  682, 651, 745, 714, 683, 746, 715, 747, 524, 556, 525, 588, 557, 526, 620,  589,
    558, 527, 621,  590, 559, 622,  591,  623,  400, 432, 401,  464, 433, 402,  496,  465,  434, 403,  497, 466,  435, 498, 467, 499, 276, 308, 277, 340, 309, 278, 372, 341, 310, 279, 373,  342,
    311, 374, 343,  375, 152, 184,  153,  216,  185, 154, 248,  217, 186, 155,  249,  218,  187, 250,  219, 251,  28,  60,  29,  92,  61,  30,  124, 93,  62,  31,  125, 94,  63,  126, 95,   127,
    900, 932, 901,  964, 933, 902,  996,  965,  934, 903, 997,  966, 935, 998,  967,  999,  776, 808,  777, 840,  809, 778, 872, 841, 810, 779, 873, 842, 811, 874, 843, 875, 652, 684, 653,  716,
    685, 654, 748,  717, 686, 655,  749,  718,  687, 750, 719,  751, 528, 560,  529,  592,  561, 530,  624, 593,  562, 531, 625, 594, 563, 626, 595, 627, 404, 436, 405, 468, 437, 406, 500,  469,
    438, 407, 501,  470, 439, 502,  471,  503,  280, 312, 281,  344, 313, 282,  376,  345,  314, 283,  377, 346,  315, 378, 347, 379, 156, 188, 157, 220, 189, 158, 252, 221, 190, 159, 253,  222,
    191, 254, 223,  255, 904, 936,  905,  968,  937, 906, 1000, 969, 938, 907,  1001, 970,  939, 1002, 971, 1003, 780, 812, 781, 844, 813, 782, 876, 845, 814, 783, 877, 846, 815, 878, 847,  879,
    656, 688, 657,  720, 689, 658,  752,  721,  690, 659, 753,  722, 691, 754,  723,  755,  532, 564,  533, 596,  565, 534, 628, 597, 566, 535, 629, 598, 567, 630, 599, 631, 408, 440, 409,  472,
    441, 410, 504,  473, 442, 411,  505,  474,  443, 506, 475,  507, 284, 316,  285,  348,  317, 286,  380, 349,  318, 287, 381, 350, 319, 382, 351, 383, 908, 940, 909, 972, 941, 910, 1004, 973,
    942, 911, 1005, 974, 943, 1006, 975,  1007, 784, 816, 785,  848, 817, 786,  880,  849,  818, 787,  881, 850,  819, 882, 851, 883, 660, 692, 661, 724, 693, 662, 756, 725, 694, 663, 757,  726,
    695, 758, 727,  759, 536, 568,  537,  600,  569, 538, 632,  601, 570, 539,  633,  602,  571, 634,  603, 635,  412, 444, 413, 476, 445, 414, 508, 477, 446, 415, 509, 478, 447, 510, 479,  511,
    912, 944, 913,  976, 945, 914,  1008, 977,  946, 915, 1009, 978, 947, 1010, 979,  1011, 788, 820,  789, 852,  821, 790, 884, 853, 822, 791, 885, 854, 823, 886, 855, 887, 664, 696, 665,  728,
    697, 666, 760,  729, 698, 667,  761,  730,  699, 762, 731,  763, 540, 572,  541,  604,  573, 542,  636, 605,  574, 543, 637, 606, 575, 638, 607, 639, 916, 948, 917, 980, 949, 918, 1012, 981,
    950, 919, 1013, 982, 951, 1014, 983,  1015, 792, 824, 793,  856, 825, 794,  888,  857,  826, 795,  889, 858,  827, 890, 859, 891, 668, 700, 669, 732, 701, 670, 764, 733, 702, 671, 765,  734,
    703, 766, 735,  767, 920, 952,  921,  984,  953, 922, 1016, 985, 954, 923,  1017, 986,  955, 1018, 987, 1019, 796, 828, 797, 860, 829, 798, 892, 861, 830, 799, 893, 862, 831, 894, 863,  895,
    924, 956, 925,  988, 957, 926,  1020, 989,  958, 927, 1021, 990, 959, 1022, 991,  1023};
static const uint_32 g_sig_last_scan_1_4[1024] = {
    0,   1,   2,   3,   32,  33,   34,   35,   64,   65,   66,   67,   96,   97,   98,   99,  4,   5,   6,   7,   36,  37,  38,  39,  68,   69,   70,   71,   100,  101,  102,  103,  8,    9,    10,
    11,  40,  41,  42,  43,  72,   73,   74,   75,   104,  105,  106,  107,  12,   13,   14,  15,  44,  45,  46,  47,  76,  77,  78,  79,   108,  109,  110,  111,  16,   17,   18,   19,   48,   49,
    50,  51,  80,  81,  82,  83,   112,  113,  114,  115,  20,   21,   22,   23,   52,   53,  54,  55,  84,  85,  86,  87,  116, 117, 118,  119,  24,   25,   26,   27,   56,   57,   58,   59,   88,
    89,  90,  91,  120, 121, 122,  123,  28,   29,   30,   31,   60,   61,   62,   63,   92,  93,  94,  95,  124, 125, 126, 127, 128, 129,  130,  131,  160,  161,  162,  163,  192,  193,  194,  195,
    224, 225, 226, 227, 132, 133,  134,  135,  164,  165,  166,  167,  196,  197,  198,  199, 228, 229, 230, 231, 136, 137, 138, 139, 168,  169,  170,  171,  200,  201,  202,  203,  232,  233,  234,
    235, 140, 141, 142, 143, 172,  173,  174,  175,  204,  205,  206,  207,  236,  237,  238, 239, 144, 145, 146, 147, 176, 177, 178, 179,  208,  209,  210,  211,  240,  241,  242,  243,  148,  149,
    150, 151, 180, 181, 182, 183,  212,  213,  214,  215,  244,  245,  246,  247,  152,  153, 154, 155, 184, 185, 186, 187, 216, 217, 218,  219,  248,  249,  250,  251,  156,  157,  158,  159,  188,
    189, 190, 191, 220, 221, 222,  223,  252,  253,  254,  255,  256,  257,  258,  259,  288, 289, 290, 291, 320, 321, 322, 323, 352, 353,  354,  355,  260,  261,  262,  263,  292,  293,  294,  295,
    324, 325, 326, 327, 356, 357,  358,  359,  264,  265,  266,  267,  296,  297,  298,  299, 328, 329, 330, 331, 360, 361, 362, 363, 268,  269,  270,  271,  300,  301,  302,  303,  332,  333,  334,
    335, 364, 365, 366, 367, 272,  273,  274,  275,  304,  305,  306,  307,  336,  337,  338, 339, 368, 369, 370, 371, 276, 277, 278, 279,  308,  309,  310,  311,  340,  341,  342,  343,  372,  373,
    374, 375, 280, 281, 282, 283,  312,  313,  314,  315,  344,  345,  346,  347,  376,  377, 378, 379, 284, 285, 286, 287, 316, 317, 318,  319,  348,  349,  350,  351,  380,  381,  382,  383,  384,
    385, 386, 387, 416, 417, 418,  419,  448,  449,  450,  451,  480,  481,  482,  483,  388, 389, 390, 391, 420, 421, 422, 423, 452, 453,  454,  455,  484,  485,  486,  487,  392,  393,  394,  395,
    424, 425, 426, 427, 456, 457,  458,  459,  488,  489,  490,  491,  396,  397,  398,  399, 428, 429, 430, 431, 460, 461, 462, 463, 492,  493,  494,  495,  400,  401,  402,  403,  432,  433,  434,
    435, 464, 465, 466, 467, 496,  497,  498,  499,  404,  405,  406,  407,  436,  437,  438, 439, 468, 469, 470, 471, 500, 501, 502, 503,  408,  409,  410,  411,  440,  441,  442,  443,  472,  473,
    474, 475, 504, 505, 506, 507,  412,  413,  414,  415,  444,  445,  446,  447,  476,  477, 478, 479, 508, 509, 510, 511, 512, 513, 514,  515,  544,  545,  546,  547,  576,  577,  578,  579,  608,
    609, 610, 611, 516, 517, 518,  519,  548,  549,  550,  551,  580,  581,  582,  583,  612, 613, 614, 615, 520, 521, 522, 523, 552, 553,  554,  555,  584,  585,  586,  587,  616,  617,  618,  619,
    524, 525, 526, 527, 556, 557,  558,  559,  588,  589,  590,  591,  620,  621,  622,  623, 528, 529, 530, 531, 560, 561, 562, 563, 592,  593,  594,  595,  624,  625,  626,  627,  532,  533,  534,
    535, 564, 565, 566, 567, 596,  597,  598,  599,  628,  629,  630,  631,  536,  537,  538, 539, 568, 569, 570, 571, 600, 601, 602, 603,  632,  633,  634,  635,  540,  541,  542,  543,  572,  573,
    574, 575, 604, 605, 606, 607,  636,  637,  638,  639,  640,  641,  642,  643,  672,  673, 674, 675, 704, 705, 706, 707, 736, 737, 738,  739,  644,  645,  646,  647,  676,  677,  678,  679,  708,
    709, 710, 711, 740, 741, 742,  743,  648,  649,  650,  651,  680,  681,  682,  683,  712, 713, 714, 715, 744, 745, 746, 747, 652, 653,  654,  655,  684,  685,  686,  687,  716,  717,  718,  719,
    748, 749, 750, 751, 656, 657,  658,  659,  688,  689,  690,  691,  720,  721,  722,  723, 752, 753, 754, 755, 660, 661, 662, 663, 692,  693,  694,  695,  724,  725,  726,  727,  756,  757,  758,
    759, 664, 665, 666, 667, 696,  697,  698,  699,  728,  729,  730,  731,  760,  761,  762, 763, 668, 669, 670, 671, 700, 701, 702, 703,  732,  733,  734,  735,  764,  765,  766,  767,  768,  769,
    770, 771, 800, 801, 802, 803,  832,  833,  834,  835,  864,  865,  866,  867,  772,  773, 774, 775, 804, 805, 806, 807, 836, 837, 838,  839,  868,  869,  870,  871,  776,  777,  778,  779,  808,
    809, 810, 811, 840, 841, 842,  843,  872,  873,  874,  875,  780,  781,  782,  783,  812, 813, 814, 815, 844, 845, 846, 847, 876, 877,  878,  879,  784,  785,  786,  787,  816,  817,  818,  819,
    848, 849, 850, 851, 880, 881,  882,  883,  788,  789,  790,  791,  820,  821,  822,  823, 852, 853, 854, 855, 884, 885, 886, 887, 792,  793,  794,  795,  824,  825,  826,  827,  856,  857,  858,
    859, 888, 889, 890, 891, 796,  797,  798,  799,  828,  829,  830,  831,  860,  861,  862, 863, 892, 893, 894, 895, 896, 897, 898, 899,  928,  929,  930,  931,  960,  961,  962,  963,  992,  993,
    994, 995, 900, 901, 902, 903,  932,  933,  934,  935,  964,  965,  966,  967,  996,  997, 998, 999, 904, 905, 906, 907, 936, 937, 938,  939,  968,  969,  970,  971,  1000, 1001, 1002, 1003, 908,
    909, 910, 911, 940, 941, 942,  943,  972,  973,  974,  975,  1004, 1005, 1006, 1007, 912, 913, 914, 915, 944, 945, 946, 947, 976, 977,  978,  979,  1008, 1009, 1010, 1011, 916,  917,  918,  919,
    948, 949, 950, 951, 980, 981,  982,  983,  1012, 1013, 1014, 1015, 920,  921,  922,  923, 952, 953, 954, 955, 984, 985, 986, 987, 1016, 1017, 1018, 1019, 924,  925,  926,  927,  956,  957,  958,
    959, 988, 989, 990, 991, 1020, 1021, 1022, 1023};
static const uint_32 g_sig_last_scan_2_4[1024] = {
    0,   32,  64,  96,   1,   33,  65,  97,   2,   34,  66,  98,   3,   35,  67,  99,   128, 160, 192, 224,  129, 161, 193, 225,  130, 162, 194, 226,  131, 163, 195, 227,  256, 288, 320, 352,
    257, 289, 321, 353,  258, 290, 322, 354,  259, 291, 323, 355,  384, 416, 448, 480,  385, 417, 449, 481,  386, 418, 450, 482,  387, 419, 451, 483,  512, 544, 576, 608,  513, 545, 577, 609,
    514, 546, 578, 610,  515, 547, 579, 611,  640, 672, 704, 736,  641, 673, 705, 737,  642, 674, 706, 738,  643, 675, 707, 739,  768, 800, 832, 864,  769, 801, 833, 865,  770, 802, 834, 866,
    771, 803, 835, 867,  896, 928, 960, 992,  897, 929, 961, 993,  898, 930, 962, 994,  899, 931, 963, 995,  4,   36,  68,  100,  5,   37,  69,  101,  6,   38,  70,  102,  7,   39,  71,  103,
    132, 164, 196, 228,  133, 165, 197, 229,  134, 166, 198, 230,  135, 167, 199, 231,  260, 292, 324, 356,  261, 293, 325, 357,  262, 294, 326, 358,  263, 295, 327, 359,  388, 420, 452, 484,
    389, 421, 453, 485,  390, 422, 454, 486,  391, 423, 455, 487,  516, 548, 580, 612,  517, 549, 581, 613,  518, 550, 582, 614,  519, 551, 583, 615,  644, 676, 708, 740,  645, 677, 709, 741,
    646, 678, 710, 742,  647, 679, 711, 743,  772, 804, 836, 868,  773, 805, 837, 869,  774, 806, 838, 870,  775, 807, 839, 871,  900, 932, 964, 996,  901, 933, 965, 997,  902, 934, 966, 998,
    903, 935, 967, 999,  8,   40,  72,  104,  9,   41,  73,  105,  10,  42,  74,  106,  11,  43,  75,  107,  136, 168, 200, 232,  137, 169, 201, 233,  138, 170, 202, 234,  139, 171, 203, 235,
    264, 296, 328, 360,  265, 297, 329, 361,  266, 298, 330, 362,  267, 299, 331, 363,  392, 424, 456, 488,  393, 425, 457, 489,  394, 426, 458, 490,  395, 427, 459, 491,  520, 552, 584, 616,
    521, 553, 585, 617,  522, 554, 586, 618,  523, 555, 587, 619,  648, 680, 712, 744,  649, 681, 713, 745,  650, 682, 714, 746,  651, 683, 715, 747,  776, 808, 840, 872,  777, 809, 841, 873,
    778, 810, 842, 874,  779, 811, 843, 875,  904, 936, 968, 1000, 905, 937, 969, 1001, 906, 938, 970, 1002, 907, 939, 971, 1003, 12,  44,  76,  108,  13,  45,  77,  109,  14,  46,  78,  110,
    15,  47,  79,  111,  140, 172, 204, 236,  141, 173, 205, 237,  142, 174, 206, 238,  143, 175, 207, 239,  268, 300, 332, 364,  269, 301, 333, 365,  270, 302, 334, 366,  271, 303, 335, 367,
    396, 428, 460, 492,  397, 429, 461, 493,  398, 430, 462, 494,  399, 431, 463, 495,  524, 556, 588, 620,  525, 557, 589, 621,  526, 558, 590, 622,  527, 559, 591, 623,  652, 684, 716, 748,
    653, 685, 717, 749,  654, 686, 718, 750,  655, 687, 719, 751,  780, 812, 844, 876,  781, 813, 845, 877,  782, 814, 846, 878,  783, 815, 847, 879,  908, 940, 972, 1004, 909, 941, 973, 1005,
    910, 942, 974, 1006, 911, 943, 975, 1007, 16,  48,  80,  112,  17,  49,  81,  113,  18,  50,  82,  114,  19,  51,  83,  115,  144, 176, 208, 240,  145, 177, 209, 241,  146, 178, 210, 242,
    147, 179, 211, 243,  272, 304, 336, 368,  273, 305, 337, 369,  274, 306, 338, 370,  275, 307, 339, 371,  400, 432, 464, 496,  401, 433, 465, 497,  402, 434, 466, 498,  403, 435, 467, 499,
    528, 560, 592, 624,  529, 561, 593, 625,  530, 562, 594, 626,  531, 563, 595, 627,  656, 688, 720, 752,  657, 689, 721, 753,  658, 690, 722, 754,  659, 691, 723, 755,  784, 816, 848, 880,
    785, 817, 849, 881,  786, 818, 850, 882,  787, 819, 851, 883,  912, 944, 976, 1008, 913, 945, 977, 1009, 914, 946, 978, 1010, 915, 947, 979, 1011, 20,  52,  84,  116,  21,  53,  85,  117,
    22,  54,  86,  118,  23,  55,  87,  119,  148, 180, 212, 244,  149, 181, 213, 245,  150, 182, 214, 246,  151, 183, 215, 247,  276, 308, 340, 372,  277, 309, 341, 373,  278, 310, 342, 374,
    279, 311, 343, 375,  404, 436, 468, 500,  405, 437, 469, 501,  406, 438, 470, 502,  407, 439, 471, 503,  532, 564, 596, 628,  533, 565, 597, 629,  534, 566, 598, 630,  535, 567, 599, 631,
    660, 692, 724, 756,  661, 693, 725, 757,  662, 694, 726, 758,  663, 695, 727, 759,  788, 820, 852, 884,  789, 821, 853, 885,  790, 822, 854, 886,  791, 823, 855, 887,  916, 948, 980, 1012,
    917, 949, 981, 1013, 918, 950, 982, 1014, 919, 951, 983, 1015, 24,  56,  88,  120,  25,  57,  89,  121,  26,  58,  90,  122,  27,  59,  91,  123,  152, 184, 216, 248,  153, 185, 217, 249,
    154, 186, 218, 250,  155, 187, 219, 251,  280, 312, 344, 376,  281, 313, 345, 377,  282, 314, 346, 378,  283, 315, 347, 379,  408, 440, 472, 504,  409, 441, 473, 505,  410, 442, 474, 506,
    411, 443, 475, 507,  536, 568, 600, 632,  537, 569, 601, 633,  538, 570, 602, 634,  539, 571, 603, 635,  664, 696, 728, 760,  665, 697, 729, 761,  666, 698, 730, 762,  667, 699, 731, 763,
    792, 824, 856, 888,  793, 825, 857, 889,  794, 826, 858, 890,  795, 827, 859, 891,  920, 952, 984, 1016, 921, 953, 985, 1017, 922, 954, 986, 1018, 923, 955, 987, 1019, 28,  60,  92,  124,
    29,  61,  93,  125,  30,  62,  94,  126,  31,  63,  95,  127,  156, 188, 220, 252,  157, 189, 221, 253,  158, 190, 222, 254,  159, 191, 223, 255,  284, 316, 348, 380,  285, 317, 349, 381,
    286, 318, 350, 382,  287, 319, 351, 383,  412, 444, 476, 508,  413, 445, 477, 509,  414, 446, 478, 510,  415, 447, 479, 511,  540, 572, 604, 636,  541, 573, 605, 637,  542, 574, 606, 638,
    543, 575, 607, 639,  668, 700, 732, 764,  669, 701, 733, 765,  670, 702, 734, 766,  671, 703, 735, 767,  796, 828, 860, 892,  797, 829, 861, 893,  798, 830, 862, 894,  799, 831, 863, 895,
    924, 956, 988, 1020, 925, 957, 989, 1021, 926, 958, 990, 1022, 927, 959, 991, 1023};
const uint_32 *const kvz_g_sig_last_scan[3][5] = {{g_sig_last_scan_0_0, g_sig_last_scan_0_1, g_sig_last_scan_0_2, g_sig_last_scan_0_3, g_sig_last_scan_0_4},
                                                  {g_sig_last_scan_1_0, g_sig_last_scan_1_1, g_sig_last_scan_1_2, g_sig_last_scan_1_3, g_sig_last_scan_1_4},
                                                  {g_sig_last_scan_2_0, g_sig_last_scan_2_1, g_sig_last_scan_2_2, g_sig_last_scan_2_3, g_sig_last_scan_2_4}};

static int_8 get_scan_order(const uint_6 intra_mode, const two_bit depth) {
  // Scan mode is diagonal, except for 4x4+8x8 luma and 4x4 chroma, where:
  // - angular 6-14 = vertical
  // - angular 22-30 = horizontal
  if (depth >= 2) {
    if (intra_mode >= 6 && intra_mode <= 14) {
      return SCAN_VER;
    } else if (intra_mode >= 22 && intra_mode <= 30) {
      return SCAN_HOR;
    }
  }

  return SCAN_DIAG;
}

uint_32 get_coeff_cost_int(coeff_t *coeff, int_32 width, int_8 scan_mode) {
  // Coefficients for the linear models, from 4x4 to 32x32.
  // intercept, coeff_sum, zero_coeffs, one_coeffs, nonzero_cg, scan_i

  /*const static double fits[4][6] = {
      // Trained with decimated samples of Kimono+Basket, rd1+nordoq+wpp
      { 57.0892748, 0.4917267, -3.3880683, -2.2381652, 0.0, 1.0528123 }, // stde=2.756
      { 275.6710972, 0.3146363, -4.3033721, -1.7292601, 2.4446289, 0.2775454 }, // stde=4.348
      { 1098.9114759, 0.2115117, -4.2989654, -1.7549422, 5.9533124, 0.0847259 }, // stde=7.655
      { 4452.0086813, 0.1706842, -4.3502404, -2.1188156, 7.7211520, 0.0370893 }, // stde=18.46
  };*/

  /* 15 bit accuracy, rounded down
  const static int_32 ffits[4][6] = {
      { 1870701, 16112, -111021, -73341, 0, 34498 },
      { 9033190, 10310, -141013, -56665, 80105, 9094 },
      { 36009131, 6930, -140869, -57506, 195078, 2776 },
      { 145883420, 5592, -142549, -69430, 253006, 1215 }
  };*/

  /* 14 bit accuracy, rounded down
  const static int_32 hfits[4][6] = {
      { 0, 8056, -55511, -36671, 0, 17249 },
      { 0, 5155, -70507, -28333, 40052, 4547 },
      { 0, 3465, -70435, -28753, 97539, 1388 },
      { 0, 2796, -71275, -34715, 126503, 607 }
  };*/

  // Combined:
  //  First collumn: full 15 bits decimal accuracy
  //  Rest: 14 bit decimal accuracy, shifted to 15 bits
  const static int_32 fits[4][6] = {
      {1870701, 16112, -111022, -73342, 0, 34498}, {9033190, 10310, -141014, -56666, 80104, 9094}, {36009131, 6930, -140870, -57506, 195078, 2776}, {145883420, 5592, -142550, -69430, 253006, 1214}};

  const uint_8 bits = kvz_g_convert_to_bit[width];
  const uint_32 *scan = kvz_g_sig_last_scan[scan_mode][bits + 1];

  // Parameters for linear model.
  uint_16 scan_i_last = 0;
  uint_32 coeff_sum = 0;
  uint_16 zero_coeff_count = 0;
  uint_16 one_coeff_count = 0;
  uint_16 nonzero_cg_count = 0;

  // Iterate over the residual in scan order in order to find the position
  // of the last significant coeff and to count the number of nonzero coeff
  // groups.
  uint_8 last_nonzero_cg = 63;

  for (int i = 0; i < width * width; ++i) {
    coeff_t coeff_abs = abs((int)coeff[scan[i]]);

    if (coeff_abs) {
      if (coeff_abs == 1) {
        one_coeff_count += 1;
      }
      scan_i_last = i;
      coeff_sum += coeff_abs;

      // Avoid counting same coeff group more than once.
      // The coeff group changes every 16 (4x4) coeffs in scan order.
      uint_8 cur_cg = i >> 4;
      if (cur_cg != last_nonzero_cg) {
        last_nonzero_cg = cur_cg;
        ++nonzero_cg_count;
      }
    } else {
      zero_coeff_count += 1;
    }
  }

  if (coeff_sum == 0) {
    return 0;
  }

  // std::cout << "Ref  : " << coeff_sum << " " << zero_coeff_count << " " << one_coeff_count << " " << nonzero_cg_count << " " << scan_i_last << std::endl;

  uint_32 result_bits =
      (fits[bits][0] + fits[bits][1] * coeff_sum + fits[bits][2] * zero_coeff_count + fits[bits][3] * one_coeff_count + fits[bits][4] * nonzero_cg_count + fits[bits][5] * scan_i_last);

  return result_bits;
}

class Input {
public:
  Input() { reset(); }

  void operator<<(const int_16 &rhs) {
    src[w_id].set_slc(slc * 16, rhs);

    slc++;
    if (!slc)
      w_id++;
  }

  ac_int< 16 * 32, false > get() { return src[r_id++]; }

  bool empty() { return r_id == w_id; }
  void reset() { slc = r_id = w_id = 0; }

private:
  ac_int< 5, false > slc;
  ac_int< 6, false > r_id;
  ac_int< 6, false > w_id;
  ac_int< 16 * 32, false > src[64];
};

class Output {
public:
  Output() { reset(); }

  void operator<<(const ac_int< 16 * 32, false > &rhs) { src[w_id++] = rhs; }

  int_16 get() {
    int_16 s = src[r_id].slc< 16 >(16 * slc++);
    if (!slc)
      r_id++;

    return s;
  }

  bool empty() { return r_id == w_id; }
  void reset() { slc = r_id = w_id = 0; }

private:
  ac_int< 5, false > slc;
  ac_int< 6, false > r_id;
  ac_int< 6, false > w_id;
  ac_int< 16 * 32, false > src[64];
};

void main_coeff_cost(coeff_port_t &coeff_in, coeff_port_t &coeff_out, int_16 trans_push[SIZE_MULT * MAX_WIDTH][MAX_WIDTH], int_16 trans_pull[SIZE_MULT * MAX_WIDTH][MAX_WIDTH]);

CCS_MAIN(int argc, char *argv[]) {
  static coeff_port_t coeff_in;
  static coeff_port_t coeff_out;

  static int_16 trans[SIZE_MULT * MAX_WIDTH][MAX_WIDTH];

  coeff_t src[32 * 32];
  coeff_t dst[32 * 32];
  one_bit simulation_successful = 1;

  Input slice_i;
  Output slice_o;

  one_bit print = 0;

  for (int i = 0; i < 128; i++) {
    srand(i);

    for (int y = 0; y < 32; ++y) {
      for (int x = 0; x < 32; ++x) {
        src[y * 32 + x] = 15; // int_16(rand());
      }
    }

    uint_4 depth;
    for (depth = 0; depth < 4; ++depth) {
      uint_4 color;
      for (color = 0; color < 3; ++color) {

        const uint_6 intra_mode = rand() % 35;

        conf_t duv_conf;
        duv_conf.depth = depth;
        duv_conf.color = color;
        duv_conf.intra_mode = intra_mode;

        bool tr_skip = !color && depth == 3;

        two_bit size = duv_conf.size();
        uint_6 width = duv_conf.width();
        cout << "Depth: " << depth << " Color: " << color << " Width: " << width << " Intra Mode: " << intra_mode << endl;

        for (int y = 0; y < width; ++y) {
          for (int x = 0; x < width; ++x) {
            slice_i << src[x * width + y];

            if (print)
              std::cout << src[x * width + y] << " ";
          }
          if (print)
            std::cout << "\n";
        }

        if (size == 3) { // Add second 4x4
          for (int y = 16; y < 32; ++y) {
            slice_i << src[y];
          }
        }

        uint_32 ref_cost_1st, ref_cost_2nd;
        ref_cost_1st = get_coeff_cost_int(src, width, get_scan_order(intra_mode, depth));
        ref_cost_2nd = get_coeff_cost_int(&src[16], width, get_scan_order(intra_mode, depth));

        coeff_in.write(duv_conf.toInt());
        while (!slice_i.empty()) {
          coeff_in.write(slice_i.get());
        }

        CCS_DESIGN(main_coeff_cost)
        (coeff_in, coeff_out, trans, trans);

        if (print)
          std::cout << "DUV Output:\n";

        // Read all but last
        while (coeff_out.available(2)) {
          slice_o << coeff_out.read();
        }

        for (int y = 0; y < width; ++y) {
          for (int x = 0; x < width; ++x) {
            int_16 coeff = slice_o.get();

            if (coeff != src[y * width + x])
              simulation_successful = 0;

            if (print)
              std::cout << coeff << " ";
          }
          if (print)
            std::cout << std::endl;
        }

        if (size == 3) {
          for (uint_6 x = 0; x < 16; x++)
            slice_o.get();
        }

        coeff_slice_t output = coeff_out.read();
        uint_32 cost_full = output.slc< 32 >(0);
        uint_32 cost_1st = output.slc< 32 >(32);
        uint_32 cost_2nd = output.slc< 32 >(64);

        if (size != 3) {
          if (cost_full != ref_cost_1st) {
            simulation_successful = 0;
            std::cout << "ERROR " << cost_full << " " << ref_cost_1st << std::endl;
          }
        } else {
          if (cost_1st != ref_cost_1st) {
            simulation_successful = 0;
            std::cout << "ERROR Dual 1st: " << cost_1st << " " << ref_cost_1st << std::endl;
          }
          if (cost_2nd != ref_cost_2nd) {
            simulation_successful = 0;
            std::cout << "ERROR Dual 2nd: " << cost_2nd << " " << ref_cost_2nd << std::endl;
          }
        }

        if (!slice_o.empty()) {
          simulation_successful = 0;
          std::cout << "ERROR Output channel not empty" << std::endl;
        }

        if (!simulation_successful)
          break;
      }
      if (!simulation_successful)
        break;
    }
    if (!simulation_successful)
      break;
  }

  if (simulation_successful == 1) {
    cout << "## SIMULATION PASSED ##" << endl;
  } else {
    cout << "## SIMULATION FAILED ##" << endl;
  }

  CCS_RETURN(0);
}
