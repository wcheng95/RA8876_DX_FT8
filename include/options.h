/*
 * options.h
 *
 *  Created on: May 25, 2016
 *      Author: user
 */

#ifndef OPTIONS_H_
#define OPTIONS_H_
#include <stdint.h>

typedef enum
{
    OPTION_Band_Index = 0,
    OPTION_Map_Index,
    NUM_OPTIONS
} OptionNumber;

// Work with option data
const char *Options_GetName(int optionIdx);
int16_t Options_GetValue(int optionIdx);
int16_t Options_GetInitValue(int optionIdx);

uint16_t Options_GetMinimum(int optionIdx);
uint16_t Options_GetMaximum(int optionIdx);
uint16_t Options_GetChangeRate(int optionIdx);

void Options_SetSelectedOption(OptionNumber newOption);
void Options_SetValue(int optionIdx, int16_t newValue);
int16_t Options_ReadValue(int optionIdx);

void Options_ResetToDefaults(void);
void Options_Initialize(void);
void Options_StoreValue(int optionIdx);


#endif /* OPTIONS_H_ */
