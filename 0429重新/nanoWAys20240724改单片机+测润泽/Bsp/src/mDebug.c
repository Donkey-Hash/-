#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "cmsis_os.h"
#include "stdio.h"

/* Private typedef -----------------------------------------------------------*/
enum { r0, r1, r2, r3, r12, lr, pc, psr};
 
 /* Private define ------------------------------------------------------------*/
 /* Private macro -------------------------------------------------------------*/
 /* Private variables ---------------------------------------------------------*/

 uint32_t Index = 0;
 
 /* Private function prototypes -----------------------------------------------*/
 void Hard_Fault_Handler(uint32_t stack[]);
 
 /* Private functions ---------------------------------------------------------*/
 
 static void printUsageErrorMsg(uint32_t CFSRValue)
 {
     printf("Usage fault: \r\n");
     CFSRValue >>= 16; // right shift to lsb
 
     if((CFSRValue & (1<<9)) != 0) {
         printf("Divide by zero \r\n");
     }
     if((CFSRValue & (1<<8)) != 0) {
         printf("Unaligned access \r\n");
     }
 }
 
 static void printBusFaultErrorMsg(uint32_t CFSRValue)
 {
     printf("Bus fault: \r\n");
     CFSRValue = ((CFSRValue & 0x0000FF00) >> 8); // mask and right shift to lsb
 }
 
 static void printMemoryManagementErrorMsg(uint32_t CFSRValue)
 {
     printf("Memory Management fault: \r\n");
     CFSRValue &= 0x000000FF; // mask just mem faults
 }
 
 static void stackDump(uint32_t stack[])
 {
			static char msg[80];
			sprintf(msg, "R0 = 0x%08x\r\n", stack[r0]); printf(msg);
			sprintf(msg, "R1 = 0x%08x\r\n", stack[r1]); printf(msg);
			sprintf(msg, "R2 = 0x%08x\r\n", stack[r2]); printf(msg);
			sprintf(msg, "R3 = 0x%08x\r\n", stack[r3]); printf(msg);
 			sprintf(msg, "R12 = 0x%08x\r\n", stack[r12]); printf(msg);
			sprintf(msg, "LR = 0x%08x\r\n", stack[lr]); printf(msg);
			sprintf(msg, "PC = 0x%08x\r\n", stack[pc]); printf(msg);
			sprintf(msg, "PSR = 0x%08x\r\n", stack[psr]); printf(msg);
 }
 
 void Hard_Fault_Handler(uint32_t stack[])
 {
			static char msg[80];
			//if((CoreDebug->DHCSR & 0x01) != 0) {
					printf("\r\nIn Hard Fault Handler\r\n");
					sprintf(msg, "SCB->CPUID = 0x%08x\r\n", SCB->CPUID);printf(msg);
					sprintf(msg, "SCB->SHCSR = 0x%08x\r\n", SCB->SHCSR);printf(msg);
					sprintf(msg, "SCB->MMFAR = 0x%08x\r\n", SCB->MMFAR);printf(msg);
					sprintf(msg, "SCB->AFAR = 0x%08x\r\n", SCB->AFSR);printf(msg);
					sprintf(msg, "SCB->BFAR = 0x%08x\r\n", SCB->BFAR);printf(msg);
					sprintf(msg, "SCB->CFAR = 0x%08x\r\n", SCB->CFSR);	printf(msg);				
					sprintf(msg, "SCB->DFAR = 0x%08x\r\n", SCB->DFSR);printf(msg);
					sprintf(msg, "SCB->SCR = 0x%08x\r\n", SCB->SCR);printf(msg);
					sprintf(msg, "SCB->CCR = 0x%08x\r\n", SCB->CCR);printf(msg);
					sprintf(msg, "SCB->HFSR = 0x%08x\r\n", SCB->HFSR);printf(msg);
					if ((SCB->HFSR & (1 << 30)) != 0) {
							printf("Forced Hard Fault\r\n");
							sprintf(msg, "SCB->CFSR = 0x%08x\r\n", SCB->CFSR );
							printf(msg);
							if((SCB->CFSR & 0xFFFF0000) != 0) {
								printUsageErrorMsg(SCB->CFSR);
							}
							if((SCB->CFSR & 0xFF00) != 0) {
									printBusFaultErrorMsg(SCB->CFSR);
							}
							if((SCB->CFSR & 0xFF) != 0) {
									printMemoryManagementErrorMsg(SCB->CFSR);
						}
					}
					stackDump(stack);
					printf("Restart system£¡\r\n");
//					__disable_irq();
//					NVIC_SystemReset();
//				__ASM volatile("BKPT #01");
			//}
		while(1);
 }
 
 
 __ASM	void HardFault_Handler_a(void)
 {
			IMPORT Hard_Fault_Handler
 
			TST lr, #4
			ITE EQ
			MRSEQ r0, MSP
			MRSNE r0, PSP
			B Hard_Fault_Handler
 }
