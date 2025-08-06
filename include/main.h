
#pragma once

extern int FT_8_counter;
extern int ft8_marker;
extern int WF_counter;
extern int xmit_flag;

extern RA8876_t3 tft;
extern AudioControlSGTL5000 sgtl5000;
extern Si5351 si5351;
extern AudioAmplifier amp1;
extern AudioAmplifier in_left_amp;
extern AudioAmplifier in_right_amp;

extern char Station_Call[11];
extern char Station_Locator[7];
extern char Short_Station_Locator[5];
extern int decode_flag;
extern uint16_t cursor_line;
extern int Tune_On;
extern int master_decoded;
extern uint16_t cursor_freq;
extern int ft8_flag;
extern int ft8_xmit_counter;
extern int slot_state;
extern int target_slot;

extern q15_t __attribute__((aligned(4))) dsp_buffer[];
extern q15_t __attribute__((aligned(4))) dsp_output[];

void display_messages(int decoded_messages);
time_t getTeensy3Time(void);
