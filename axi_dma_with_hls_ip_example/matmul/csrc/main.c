#include "xparameters.h"
#include "xaxidma.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xil_printf.h"
#include "xaxidma_hw.h"
#include "sleep.h"

#include <stdint.h>

#define BRAM_ADDR XPAR_AXI_BRAM_CTRL_0_BASEADDR

#define MAT_SIZE 5
#define MAT_N (MAT_SIZE * MAT_SIZE)
#define OFFSET(row, col) (row * MAT_SIZE + col)

int main()
{
    print("enter lab ==============\r\n");
    /* initialization of DMA */
    XAxiDma my_DMA;
    XAxiDma_Config *my_DMA_config;
    my_DMA_config = XAxiDma_LookupConfig(XPAR_AXI_DMA_0_BASEADDR);
    if (my_DMA_config == NULL) {
        print("DMA LookupConfig failed\r\n");
        return -1;
    }
    if (XAxiDma_CfgInitialize(&my_DMA, my_DMA_config) != XST_SUCCESS) {
        print("Initialize DMA failed\r\n");
        return -1;
    }

    uint32_t *ptr = (uint32_t *) BRAM_ADDR;
    
    print("initialize matrix A\r\n");
    for (int row = 0; row < MAT_SIZE; row++) {
        for (int col = 0; col < MAT_SIZE; col++) {
            ptr[OFFSET(row, col)] = row * MAT_SIZE + col;
            xil_printf("%4u ", ptr[OFFSET(row, col)]);
        }
        print("\r\n");
    }
    print("\r\n");
    ptr += MAT_N;
    print("initialize matrix B\r\n");
    for (int row = 0; row < MAT_SIZE; row++) {
        for (int col = 0; col < MAT_SIZE; col++) {
            // ptr[OFFSET(row, col)] = row * MAT_SIZE + col;
            ptr[OFFSET(row, col)] = 1;
            xil_printf("%4u ", ptr[OFFSET(row, col)]);
        }
        print("\r\n");
    }
    print("\r\n");

    /* setup DMA ready to receive data from device */
    XAxiDma_SimpleTransfer(&my_DMA, BRAM_ADDR + (2 * MAT_N) * sizeof(uint32_t), MAT_N * sizeof(uint32_t), XAXIDMA_DEVICE_TO_DMA);
    /* setup find IP val value
     * via AXI-Lite
     * set find IP to find the value 8
     */
    XAxiDma_SimpleTransfer(&my_DMA, BRAM_ADDR,  (2 * MAT_N) * sizeof(uint32_t), XAXIDMA_DMA_TO_DEVICE);

    while ((XAxiDma_Busy(&my_DMA, XAXIDMA_DEVICE_TO_DMA))
           || (XAxiDma_Busy(&my_DMA, XAXIDMA_DMA_TO_DEVICE))) {
           usleep(1U);
    }
    print("both MM2S and S2MM done\r\n");

    ptr = (uint32_t *) BRAM_ADDR + (2 * MAT_N);
    print("output matrix\r\n");
    for (int row = 0; row < MAT_SIZE; row++) {
        for (int col = 0; col < MAT_SIZE; col++) {
            xil_printf("%4u ", ptr[OFFSET(row, col)]);
        }
        print("\r\n");
    }

    print("exit lab ===============\r\n");
}