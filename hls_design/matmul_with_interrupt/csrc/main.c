#include <xparameters.h>
#include <xstatus.h>
#include <xil_io.h>
#include <xil_types.h>
#include <xil_printf.h>
#include <xil_exception.h>
#include <xinterrupt_wrap.h>
#include <sleep.h>
#include <xintc.h>
#include <xaxidma.h>
#include <xaxidma_hw.h>
#include <xmatmul_hw.h>
#include <xmatmul_hw_hw.h>

#include <stdint.h>

#define BRAM_ADDR XPAR_AXI_BRAM_CTRL_0_BASEADDR

#define MAT_SIZE 5
#define MAT_N (MAT_SIZE * MAT_SIZE)
#define OFFSET(row, col) (row * MAT_SIZE + col)

static XIntc my_intc;
static XMatmul_hw my_matmul;

void matmul_isr(void *CallbackRef)
{
    (void *) CallbackRef;
    XMatmul_hw_InterruptClear(&my_matmul, 0x1);
    print("enter matmul interrupt service routine ==========\r\n");
    uint32_t *ptr_result = (uint32_t *) BRAM_ADDR + (2 * MAT_N);
    print("output matrix\r\n");
    for (int row = 0; row < MAT_SIZE; row++) {
        for (int col = 0; col < MAT_SIZE; col++) {
            xil_printf("%4u ", ptr_result[OFFSET(row, col)]);
        }
        print("\r\n");
    }
    print("exit matmul interrupt service routine ===========\r\n");
}

int main()
{

    int status;
    
    print("enter lab ==============\r\n");
    status = XIntc_Initialize(&my_intc, XPAR_XINTC_0_BASEADDR);
    if (status != XST_SUCCESS) {
        print("initialize interrupt controller failed\r\n");
        return XST_FAILURE;
    }
    print("initialize interrupt controller successfully\r\n");
    
    status = XIntc_Connect(&my_intc,
                           XPAR_FABRIC_MATMUL_HW_0_INTR,
                           (XInterruptHandler) matmul_isr,
                           (void *) 0);
    if (status != XST_SUCCESS) {
        print("register matmul ISR to interrupt controller failed\r\n");
        return XST_FAILURE;
    }
    print("register matmul ISR to interrupt controller successfully\r\n");

    status = XIntc_Start(&my_intc, XIN_REAL_MODE);
    if (status != XST_SUCCESS) {
        print("start interrupt controller failed\r\n");
        return XST_FAILURE;
    }

    XIntc_Enable(&my_intc, XPAR_FABRIC_MATMUL_HW_0_INTR);

    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                                 (Xil_ExceptionHandler) XIntc_InterruptHandler,
                                 &my_intc);
    Xil_ExceptionEnable();

    /* initialization of matmul IP*/
    status = XMatmul_hw_Initialize(&my_matmul, XPAR_MATMUL_HW_0_BASEADDR);
    if (status != XST_SUCCESS) {
        print("initialize matmul IP failed\r\n");
        return XST_FAILURE;
    }
    print("initialize matmul IP successfully\r\n");
    XMatmul_hw_InterruptGlobalEnable(&my_matmul);
    XMatmul_hw_InterruptEnable(&my_matmul, 0x01); /* enable ap_done interrupt at bit 0 */
    
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
            ptr[OFFSET(row, col)] = col;
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
    XMatmul_hw_Start(&my_matmul);

    while ((XAxiDma_Busy(&my_DMA, XAXIDMA_DEVICE_TO_DMA))
           || (XAxiDma_Busy(&my_DMA, XAXIDMA_DMA_TO_DEVICE))) {
           usleep(1U);
    }

    print("exit lab ===============\r\n");
}