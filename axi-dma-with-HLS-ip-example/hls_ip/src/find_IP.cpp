#include "find_IP.h"

void find(uint32_t val, hls::stream<axis_data> &in_vec_hw, hls::stream<axis_data> &out_vec_hw)
{
#pragma HLS INTERFACE ap_ctrl_hs port=return
#pragma HLS INTERFACE s_axilite port=val
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE axis register both port=in_vec_hw
#pragma HLS INTERFACE axis register both port=out_vec_hw

    axis_data local_read, local_write;
    for (int i = 0; i < DIM; i++) {
        local_read = in_vec_hw.read();
        ap_uint<32> in_val = local_read.data;
        ap_uint<1> in_last = local_read.last;

        if (in_val == val) local_write.data = 1;
        else local_write.data = 0;

        if (i == DIM - 1) local_write.last = 1;
        else local_write.last = 0;
        out_vec_hw.write(local_write);
    }
}