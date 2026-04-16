#include <hls_stream.h>
#include <stdio.h>
#include "find_IP.h"

void find_sw(uint32_t val, uint32_t in_vec_sw[DIM], uint32_t out_vec_sw[DIM]);

int main()
{
    printf("*************************\n");
    printf("enter simulation\n");

    /* C testbench part */
    uint32_t in_vec_sw[DIM];
    uint32_t out_vec_sw[DIM];

    /* HLS IP part */
    axis_data local_read, local_write;
    hls::stream<axis_data> in_vec_hw;
    hls::stream<axis_data> out_vec_hw;

    /* data initialization */
    for (int i = 0; i < DIM; i++) in_vec_sw[i] = (uint32_t) 2 * i;
    for (int i = 0; i < DIM; i++) {
        local_read.data = in_vec_sw[i];
        if (i == DIM - 1) local_read.last = 1;
        else local_read.last = 0;
        in_vec_hw.write(local_read);
    }
    
    find(4, in_vec_hw, out_vec_hw);
    find_sw(4, in_vec_sw, out_vec_sw);

    int error = 0;
    for (int i = 0; i < DIM; i++) 
        if (out_vec_hw.read().data != out_vec_sw[i]) error++;

    if (error) printf("There has %d error\n", error);
    else printf("Behavior correct !\n"); 

    printf("exit simulation\n");
    printf("*************************\n");
}

void find_sw(uint32_t val, uint32_t in_vec_sw[DIM], uint32_t out_vec_sw[DIM])
{
    for (int i = 0; i < DIM; i++) {
        if (in_vec_sw[i] == val) out_vec_sw[i] = 1;
        else out_vec_sw[i] = 0;
    }
}