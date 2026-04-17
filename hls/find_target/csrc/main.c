#include "xparameters.h"
#include "xaxidma.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xil_printf.h"
#include "xaxidma_hw.h"
#include "sleep.h"

/* include find IP parameters */
#include "xfind.h"
#include "xfind_hw.h"
#include <stdint.h>

#define BRAM_ADDR XPAR_AXI_BRAM_CTRL_0_BASEADDR

/* define find IP control register base address */
#define FIND_IP_CTRL_REG_ADDR XPAR_FIND_0_BASEADDR

/* number of data that find IP in one operation */
#define DIM 64
/* define the value that find IP should found */
#define TARGET 8

int main()
{
    print("This is lab for DMA with MicroBlazeV\r\n");
    print("will DMA will stream data to find IP\r\n");
    print("then write back the output vec to memory\r\n");

    u32 in_vec[DIM] = {};
    for (int i = 0; i < DIM; i++) in_vec[i] = (u32) i;
    
    UINTPTR init_addr = BRAM_ADDR;

    for (int i = 0; i < DIM; i++) {
        Xil_Out32(init_addr, in_vec[i]);
        init_addr += sizeof(u32);
    }

    Xil_DCacheFlushRange((UINTPTR) BRAM_ADDR, DIM * sizeof(u32));
    
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

    /* initialization find IP */
    XFind my_find;
    XFind_Config *my_find_config;
    my_find_config = XFind_LookupConfig(XPAR_FIND_0_BASEADDR);
    if (my_find_config == NULL) {
        print("Find IP LookupConfig failed\r\n");
        return -1;
    }
    if (XFind_CfgInitialize(&my_find, my_find_config) != XST_SUCCESS) {
        print("Initialize Find IP failed\r\n");
    }
    
    /* setup DMA ready to receive data from device (find IP)*/
    XAxiDma_SimpleTransfer(&my_DMA, BRAM_ADDR + DIM * sizeof(u32), DIM * sizeof(u32), XAXIDMA_DEVICE_TO_DMA);
    /* setup find IP val value
     * via AXI-Lite
     * set find IP to find the value 8
     */
    XFind_Set_val_r(&my_find, TARGET);
    /* start find IP */
    XFind_Start(&my_find);
    /* start DMA to transfer data */
    XAxiDma_SimpleTransfer(&my_DMA, BRAM_ADDR,  DIM * sizeof(u32), XAXIDMA_DMA_TO_DEVICE);

    while ((XAxiDma_Busy(&my_DMA, XAXIDMA_DEVICE_TO_DMA))
           || (XAxiDma_Busy(&my_DMA, XAXIDMA_DMA_TO_DEVICE))) {
           usleep(1U);
    }
    print("both MM2S and S2MM done\r\n");

    Xil_DCacheInvalidateRange((UINTPTR) (BRAM_ADDR + DIM * sizeof(u32)), DIM * sizeof(u32));

    u32 idx = UINT32_MAX;
    u32 *out_vec = (u32 *) ((UINTPTR) BRAM_ADDR + DIM * sizeof(u32));
    for (int i = 0; i < DIM; i++) if (out_vec[i] == 1) idx = (u32) i;

    print("in_vec\r\n");
    for (int i = 0; i < DIM; i++) xil_printf("%u\r\n", in_vec[i]);
    print("out_vec\r\n");
    for (int i = 0; i < DIM; i++) xil_printf("%u\r\n", out_vec[i]);

    if (idx == UINT32_MAX) xil_printf("value %u not found\r\n", TARGET);
    else xil_printf("find value %u at index %u\r\n", TARGET, idx);

    print("Exit lab\r\n");
}