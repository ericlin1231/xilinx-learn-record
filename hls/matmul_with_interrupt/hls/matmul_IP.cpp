#include "matmul_IP.h"

void matmul_hw(hls::stream<axis_data> &in_mat, hls::stream<axis_data> &out_mat)
{

#pragma HLS INTERFACE s_axilite port=return bundle=control
#pragma HLS INTERFACE axis register both port=in_mat
#pragma HLS INTERFACE axis register both port=out_mat

    Mat_Dtype local_in_mat_A[MAT_L][MAT_L];
    Mat_Dtype local_in_mat_B[MAT_L][MAT_L];
    Mat_Dtype local_out_mat[MAT_L][MAT_L];
    axis_data local_stream;

    init_loop_A_row: for (int a_row = 0; a_row < MAT_L; a_row++) {
        init_loop_A_col: for (int a_col = 0; a_col < MAT_L; a_col++) {
            local_stream = in_mat.read();
            local_in_mat_A[a_row][a_col] = (Mat_Dtype) local_stream.data;
        }
    }

    init_loop_B_row: for (int b_row = 0; b_row < MAT_L; b_row++) {
        init_loop_B_col: for (int b_col = 0; b_col < MAT_L; b_col++) {
            local_stream = in_mat.read();
            local_in_mat_B[b_row][b_col] = (Mat_Dtype) local_stream.data;
        }
    }
    
    loop_A_row: for (int a_row = 0; a_row < MAT_L; a_row++) {
        loop_B_col: for (int b_col = 0; b_col < MAT_L; b_col++) {
            Mat_Dtype sum = 0;
            loop_idx: for (int idx = 0; idx < MAT_L; idx++) {
                sum += local_in_mat_A[a_row][idx] * local_in_mat_B[idx][b_col];
            }
            local_out_mat[a_row][b_col] = sum;
        }
    }

    loop_row: for (int row = 0; row < MAT_L; row++) {
        loop_col: for (int col = 0; col < MAT_L; col++) {
            local_stream.data = local_out_mat[row][col];
            if ((row == MAT_L - 1) && (col == MAT_L - 1)) local_stream.last = 1;
            else local_stream.last = 0;
            out_mat.write(local_stream);
        }
    }
}