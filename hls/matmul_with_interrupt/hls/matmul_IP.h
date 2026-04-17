#ifndef _MATMUL_IP_H_
#define _MATMUL_IP_H_

#include <hls_stream.h>
#include <ap_axi_sdata.h>

#define MAT_L 5
#define MAT_N (MAT_L * MAT_L)

#define Mat_Dtype int32_t

typedef ap_axis<32, 0, 0, 0> axis_data;

void matmul_hw(hls::stream<axis_data> &in_mat, hls::stream<axis_data> &out_mat);

#endif