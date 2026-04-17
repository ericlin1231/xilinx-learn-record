#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "matmul_IP.h"

void matmul_sw(Mat_Dtype in_mat_A[MAT_L][MAT_L], Mat_Dtype in_mat_B[MAT_L][MAT_L], Mat_Dtype out_mat[MAT_L][MAT_L]);

int main()
{
    printf("enter simulation program ===========\n");
    srand(time(NULL));

    Mat_Dtype in_mat_A_sw[MAT_L][MAT_L];
    Mat_Dtype in_mat_B_sw[MAT_L][MAT_L];
    Mat_Dtype out_mat_sw[MAT_L][MAT_L];

    for (int row = 0; row < MAT_L; row++) {
        for (int col = 0; col < MAT_L; col++) {
            in_mat_A_sw[row][col] = rand() % 10;
            in_mat_B_sw[row][col] = rand() % 10;
        }
    }

    hls::stream<axis_data> in_mat_hw;
    hls::stream<axis_data> out_mat_hw;
    axis_data local_stream;

    for (int row = 0; row < MAT_L; row++) {
        for (int col = 0; col < MAT_L; col++) {
            // local_stream.last = (row == MAT_L-1 && col == MAT_L-1) ? 1 : 0;
            local_stream.data = in_mat_A_sw[row][col];
            in_mat_hw.write(local_stream);
        }
    }

    for (int row = 0; row < MAT_L; row++) {
        for (int col = 0; col < MAT_L; col++) {
            local_stream.last = (row == MAT_L-1 && col == MAT_L-1) ? 1 : 0;
            local_stream.data = in_mat_B_sw[row][col];
            in_mat_hw.write(local_stream);
        }
    }

    matmul_sw(in_mat_A_sw, in_mat_B_sw, out_mat_sw);
    matmul_hw(in_mat_hw, out_mat_hw);

    int error = 0;
    for (int row = 0; row < MAT_L; row++) {
        for (int col = 0; col < MAT_L; col++) {
            if (out_mat_hw.read().data != out_mat_sw[row][col]) error++;
        }
    }

    if (!error) printf("Test pass\n");
    else printf("There are %d errors\n", error);
    printf("exit simulation program ============\n");
}

void matmul_sw(Mat_Dtype in_mat_A[MAT_L][MAT_L], Mat_Dtype in_mat_B[MAT_L][MAT_L], Mat_Dtype out_mat[MAT_L][MAT_L])
{    
    loop_A_row: for (int a_row = 0; a_row < MAT_L; a_row++) {
        loop_B_col: for (int b_col = 0; b_col < MAT_L; b_col++) {
            Mat_Dtype sum = 0;
            loop_idx: for (int idx = 0; idx < MAT_L; idx++) {
                sum += in_mat_A[a_row][idx] * in_mat_B[idx][b_col];
            }
            out_mat[a_row][b_col] = sum;
        }
    }
}