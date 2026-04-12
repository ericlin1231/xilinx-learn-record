#include <stdlib.h>
#include "platform.h"
#include "xparameters.h"
#include "xil_types.h"
#include "xil_printf.h"
#include "xgpio.h"
#include "xuartlite.h"

#define SW          XPAR_AXI_GPIO_0_BASEADDR
#define LED         XPAR_AXI_GPIO_1_BASEADDR
#define NUM1        XPAR_AXI_GPIO_2_BASEADDR
#define NUM2        XPAR_AXI_GPIO_3_BASEADDR
#define ADD_OR_SUB  XPAR_AXI_GPIO_4_BASEADDR
#define ANS         XPAR_AXI_GPIO_5_BASEADDR

XGpio LED_Gpio, SW_Gpio, NUM1_Gpio, NUM2_Gpio, ADD_OR_SUB_Gpio, ANS_Gpio;

int read_number() {
    char buffer[4] = {0};
    int idx = 0;
    char c;
    while (1) {
        c = inbyte();
        xil_printf("%c", c);
        if (c == '\r') break;
        if (idx < 3 && c >= '0' && c <= '9') {
            buffer[idx++] = c;
        }
    }
    print("\r\n");
    buffer[idx] = '\0';
    return atoi(buffer);
}

int main()
{
    init_platform();
    print("Successfully  initialize platform\r\n");

    int LED_Status, SW_Status, NUM1_Status, NUM2_Status, ADD_OR_SUB_Status, ANS_Status;
    u8 sw_data = 0x00;
    u8 num1, num2, result;
    char op;

	/* Initialize the GPIO driver */
	LED_Status = XGpio_Initialize(&LED_Gpio, LED);
	SW_Status = XGpio_Initialize(&SW_Gpio, SW);
    NUM1_Status = XGpio_Initialize(&NUM1_Gpio, NUM1);
    NUM2_Status = XGpio_Initialize(&NUM2_Gpio, NUM2);
    ADD_OR_SUB_Status = XGpio_Initialize(&ADD_OR_SUB_Gpio, ADD_OR_SUB);
    ANS_Status = XGpio_Initialize(&ANS_Gpio, ANS);

    if (LED_Status != XST_SUCCESS || SW_Status != XST_SUCCESS || 
        NUM1_Status != XST_SUCCESS || NUM2_Status != XST_SUCCESS ||
        ADD_OR_SUB_Status != XST_SUCCESS || ANS_Status != XST_SUCCESS) {
        
        print("Gpio Initialization Failed\r\n");
        return XST_FAILURE;
    } else print("Initialize GPIO successfully\r\n");

    XGpio_SetDataDirection(&LED_Gpio,        1, 0x00000000);  
    XGpio_SetDataDirection(&SW_Gpio,         1, 0xFFFFFFFF); 
    XGpio_SetDataDirection(&NUM1_Gpio,       1, 0x00000000); 
    XGpio_SetDataDirection(&NUM2_Gpio,       1, 0x00000000);  
    XGpio_SetDataDirection(&ADD_OR_SUB_Gpio, 1, 0x00000000);  
    XGpio_SetDataDirection(&ANS_Gpio,        1, 0xFFFFFFFF);

    while (1) {
        print("Enter '+' for add, '-' for sub, or 'q' to quit:\r\n");
        op = inbyte();
        xil_printf("%c\r\n", op);

        if (op == 'q') {
            print("Program terminated by user.\r\n");
            break;
        }

        if (op == '+') {
            XGpio_DiscreteWrite(&ADD_OR_SUB_Gpio, 1, 1);
        }else if(op == '-') {
            XGpio_DiscreteWrite(&ADD_OR_SUB_Gpio, 1, 0);
        } else {
            print("Invalid operator. Try again.\r\n");
            continue;
        }

        print("Enter first number (0~255):\r\n");
        num1 = read_number();
        XGpio_DiscreteWrite(&NUM1_Gpio, 1, num1);


        print("Enter second number (0~255):\r\n");
        num2 = read_number();
        XGpio_DiscreteWrite(&NUM2_Gpio, 1, num2);

        result = XGpio_DiscreteRead(&ANS_Gpio, 1);
        xil_printf("Result = %d (0x%02X)\r\n", result, result & 0xFF);

        print("-------Switch and LED-------\r\n");

        sw_data = XGpio_DiscreteRead(&SW_Gpio, 1);
        xil_printf("Switch value: 0x%02X\r\n", sw_data);
        XGpio_DiscreteWrite(&LED_Gpio, 1, sw_data);

        print("----------------------------\r\n");
	}

    cleanup_platform();
    return 0;
}
