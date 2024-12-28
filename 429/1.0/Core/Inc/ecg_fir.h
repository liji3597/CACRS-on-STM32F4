#ifndef _ECG_FIR_H
#define _ECG_FIR_H

#define  __FPU_PRESENT  1U
#include "arm_math.h"
//#include "stm32f4xx_hal.h"

void arm_fir_init(void);
void arm_fir(float32_t *inputF32, float32_t *outputF32);

#endif

