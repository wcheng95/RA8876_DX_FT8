#pragma once

#include "arm_math.h"

#define FFT_BASE_SIZE 1024
#define FFT_SIZE FFT_BASE_SIZE * 2
#define num_que_blocks 40
#define block_size 128

#define ft8_buffer 348 //arbitrary for 2.175 kc
#define ft8_min_bin 48
#define FFT_Resolution 6.25
#define ft8_msg_samples 91

extern uint8_t export_fft_power[];

void init_DSP(void);
void process_FT8_FFT(void);


