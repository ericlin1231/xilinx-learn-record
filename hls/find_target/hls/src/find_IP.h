#ifndef _FIND_IP_H_
#define _FIND_IP_H_

#include <cstdint>
#include <hls_stream.h>
#include "ap_axi_sdata.h"

#define DIM 64

typedef ap_axis<32, 0, 0, 0> axis_data;

void find(uint32_t val, hls::stream<axis_data> &in_vec_hw, hls::stream<axis_data> &out_vec_hw);

#endif