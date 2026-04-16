#include "xparameters.h"
#include "xaxidma.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xil_printf.h"
#include "xaxidma_hw.h"
#include "sleep.h"

#define BRAM_ADDR XPAR_AXI_BRAM_CTRL_0_BASEADDR

int main()
{
    print("This is lab for DMA with MicroBlazeV\r\n");

    int input_data [] = {0, 1, 4, 9, 16, 25, 36, 49, 64, 81};
    UINTPTR init_addr = BRAM_ADDR;

    for (int i = 0; i < 10; i++) {
        Xil_Out32(init_addr, input_data[i]);
        init_addr += sizeof(int);
    }

    /* XAxiDma is instance of DMA */
    XAxiDma my_DMA;
    /* XAxiDma_Config store the configuration of DMA engine
     * the information of DMA is provide by BSP
     * so the DMA engine information is READ-ONLY
     */
    XAxiDma_Config *my_DMA_config;
    
    my_DMA_config = XAxiDma_LookupConfig(XPAR_AXI_DMA_0_BASEADDR);

    if (my_DMA_config == NULL) { print("LookupConfig failed\r\n"); return -1; }

    XAxiDma_CfgInitialize(&my_DMA, my_DMA_config);

    /* tell the DMA there has 10 * sizeof(int) data will come
     * ready the buffer and enable S2MM channel
     * when the data coming is determine by the source IP
     * when the data arrive, store at address BRAM_ADDR + 10 * sizeof(int)
     */
    XAxiDma_SimpleTransfer(&my_DMA, BRAM_ADDR + 10 * sizeof(int), 10 * sizeof(int), XAXIDMA_DEVICE_TO_DMA);

    /* tell the DMA to transfer the 10 * sizeof(int) data from BRAM_ADDR
     * to device, the device in this example is the two IP
     * which concat fixed_to_floating and sqrt IP
     * so this channel is MM2S, means DMA read the data
     * from memory and "stream" to the IP
     * the result from IP will "stream" back to S2MM
     */
    XAxiDma_SimpleTransfer(&my_DMA, BRAM_ADDR,  10 * sizeof(int), XAXIDMA_DMA_TO_DEVICE);

    /*                  ┌────────────┐             ┌─────────────┐               
     *                  │            │             │             │               
     *                  │ AXI-Stream │             │     AXI     │               
     *                  │            │             │             │               
     *                  └────────────┘             └─────────────┘               
     *                                    MM2S                                   
     *                                                                     
     *┌─────────────────┐             ┌───────────┐                ┌────────────┐
     *│                ◄┼─────────────┼           ◄────────────────┼            │
     *│  customized IP  │             │    DMA    │                │    BRAM    │
     *│                ─┼─────────────┼►          ┼────────────────►            │
     *└─────────────────┘             └───────────┘                └────────────┘
     *                                                                    
     *                                    S2MM                                   
     */

    while ((XAxiDma_Busy(&my_DMA, XAXIDMA_DEVICE_TO_DMA))
           || (XAxiDma_Busy(&my_DMA, XAXIDMA_DMA_TO_DEVICE))) {
           usleep(1U);
    }
    print("both MM2S and S2MM done\r\n");

    float *float_addr = (float *) ((UINTPTR) BRAM_ADDR + 10 * sizeof(int));
    for (int i = 0; i < 10; i++) {
        float val = float_addr[i];
        int int_part = (int)val;
        int frac_part = (int)((val - int_part) * 1000000);
        xil_printf("%d.%06d\r\n", int_part, frac_part);
    }

    print("Exit lab\r\n");
}