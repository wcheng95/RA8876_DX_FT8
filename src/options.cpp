
#include <RA8876_t3.h>
#include <Audio.h>
#include <si5351.h>

#include "display.h"
#include "options.h"
#include "button.h"
#include "stdio.h"

#define sentinel 1948 // 1037, 1945, 1066,

struct  OptionStruct
{
  const char *Name;
  const int16_t Initial;
  const int16_t Minimum;
  const int16_t Maximum;
  const int16_t ChangeUnits;
  int16_t CurrentValue;
};

// Order must match OptionNumber in options.h
OptionStruct s_optionsData[] = {
    {
        /*Name*/ "  Band_Index ", // opt0
        /*Init*/ _20M,
        /*Min */ 0,
        /*Max */ 6,
        /*Rate*/ 1,
        /*Data*/ 0,
    },

    {
        /*Name*/ "  Map_Index ", // opt0
        /*Init*/ 0,
        /*Min */ 0,
        /*Max */ 2,
        /*Rate*/ 1,
        /*Data*/ 0,
    }};

// Work with option data
int16_t Options_GetValue(int optionIdx)
{
  return s_optionsData[optionIdx].CurrentValue;
}

void Options_SetValue(int optionIdx, int16_t newValue)
{
  s_optionsData[optionIdx].CurrentValue = newValue;
}

static void Options_ResetToDefaults(void)
{
  for (int i = 0; i < NUM_OPTIONS; i++)
  {
    Options_SetValue(i, s_optionsData[i].Initial);
  }
}

// Initialization
void Options_Initialize(void)
{
  if (EEPROMReadInt(10) == sentinel)
  {
    s_optionsData[0].CurrentValue = EEPROMReadInt(20);
    s_optionsData[1].CurrentValue = EEPROMReadInt(30);
  }
  else
  {
    EEPROMWriteInt(10, sentinel);
    Options_ResetToDefaults();
    EEPROMWriteInt(20, s_optionsData[0].Initial);
    EEPROMWriteInt(30, s_optionsData[1].Initial);
  }

  BandIndex = Options_GetValue(0);
  SelectFilterBlock();

  start_freq = sBand_Data[BandIndex].Frequency;
  show_wide(680, 0, (int)start_freq);

  Map_Index = Options_GetValue(1);
}

void Options_StoreValue(int optionIdx)
{
  int16_t option_value = Options_GetValue(optionIdx);

  switch (optionIdx)
  {
  case 0:
    EEPROMWriteInt(20, option_value);
    break;
  case 1:
    EEPROMWriteInt(30, option_value);
    break;
  }
}
