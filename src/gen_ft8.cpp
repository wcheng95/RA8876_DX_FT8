/*
 * gen_ft8.c
 *
 *  Created on: Oct 24, 2019
 *      Author: user
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string.h>

#include <RA8876_t3.h>
#include <Audio.h>
#include <si5351.h>

#include <TimeLib.h>

#include "arm_math.h"

#include "pack.h"
#include "encode.h"
#include "constants.h"
#include "gen_ft8.h"
#include "decode_ft8.h"
#include "display.h"
#include "traffic_manager.h"
#include "ADIF.h"
#include "main.h"
#include "button.h"

char Target_Call[7];    // six character call sign + /0
char Target_Locator[5]; // four character locator  + /0
int Target_RSL;         // four character RSL  + /0
int Station_RSL;

const char Beacon_73[] = "RR73";
const char QSO_73[] = "73";
char reply_message[MESSAGE_SIZE];

const char CQ[] = "CQ";
const char SOTA[] = "SOTA";
const char POTA[] = "POTA";
const char QRP[] = "QRP";
char display_message[26];

char ft8_time_string[] = "15:44:15";
int left_hand_message = 300;
char xmit_messages[3][19];

static int in_range(int num, int min, int max)
{
  if (num < min)
    return min;
  if (num > max)
    return max;
  return num;
}

void compose_messages(void)
{
  const char seventy_three[] = "RR73";
  const char blank[] = "                  ";

  char RSL[5];
  itoa(in_range(Target_RSL, -999, 9999), RSL, 10);

  strcpy(xmit_messages[0], blank);
  sprintf(xmit_messages[0], "%s %s %s", Target_Call, Station_Call, Locator);
  strcpy(xmit_messages[1], blank);
  sprintf(xmit_messages[1], "%s %s R%s", Target_Call, Station_Call, RSL);
  strcpy(xmit_messages[2], blank);
  sprintf(xmit_messages[2], "%s %s %s", Target_Call, Station_Call, seventy_three);

  tft.fillRect(left_hand_message, 520, 240, 20, BLACK);
  tft.setFontSize(2, true);
  tft.textColor(WHITE, BLACK);

  tft.setCursor(left_hand_message, 520);
  tft.write(xmit_messages[0], 18);
}

void que_message(int index)
{
  uint8_t packed[K_BYTES];

  pack77(xmit_messages[index], packed);
  genft8(packed, tones);

  tft.setFontSize(2, true);
  tft.textColor(WHITE, BLACK);

  tft.setCursor(left_hand_message, 520);
  tft.write(xmit_messages[index], 19);

  strcpy(current_message, xmit_messages[index]);

  if (index == 2 && Station_RSL != 99)
    write_ADIF_Log();
}

void set_reply(ReplyID replyId)
{
  uint8_t packed[K_BYTES];
  char RSL[5];

  switch (replyId)
  {
  case Reply_RSL:
  case Reply_R_RSL:
    // compute the RSL for use by the next 'switch'
    itoa(in_range(Target_RSL, -999, 9999), RSL, 10);
    break;
  case Reply_Beacon_73:
    sprintf(reply_message, "%s %s %s", Target_Call, Station_Call, Beacon_73);
    break;
  case Reply_QSO_73:
    sprintf(reply_message, "%s %s %s", Target_Call, Station_Call, QSO_73);
    break;
  }

  switch (replyId)
  {
  case Reply_RSL:
    sprintf(reply_message, "%s %s %s", Target_Call, Station_Call, RSL);
    break;
  case Reply_R_RSL:
    sprintf(reply_message, "%s %s R%s", Target_Call, Station_Call, RSL);
    break;
  case Reply_Beacon_73:
    break;
  case Reply_QSO_73:
    break;
  }

  strcpy(current_message, reply_message);
  update_message_log_display(1);

  pack77(reply_message, packed);
  genft8(packed, tones);

  clear_xmit_messages();

  tft.setFontSize(2, true);
  tft.textColor(WHITE, BLACK);
  tft.setCursor(left_hand_message, 520);
  tft.write(reply_message, 18);
}

void clear_reply_message_box(void)
{
  tft.fillRect(left_hand_message, 100, 290, 420, BLACK);
}

char Free_Text1[] = "FreeText 1   ";
char Free_Text2[] = "FreeText 2   ";

void set_cq(void)
{
  const char CQ[] = "CQ";
  char CQ_message[20] = "                   ";
  uint8_t packed[K_BYTES];

  if (Free_Index == 0)
  {
    const char *mode = NULL;
    switch (CQ_Mode_Index)
    {
    case 1:
      mode = SOTA;
      break;
    case 2:
      mode = POTA;
      break;
    case 3:
      mode = QRP;
      break;
    }

    if (mode == NULL)
    {
      sprintf(CQ_message, "%s %s %s", CQ, Station_Call, Locator);
    }
    else
    {
      sprintf(CQ_message, "%s %s %s %s", CQ, mode, Station_Call, Locator);
    }
  }
  else
  {
    switch (Free_Index)
    {
    case 1:
      strcpy(CQ_message, Free_Text1);
      break;
    case 2:
      strcpy(CQ_message, Free_Text2);
      break;
    }
  }

  pack77(CQ_message, packed);
  genft8(packed, tones);

  erase_CQ();

  tft.setFontSize(2, true);
  tft.textColor(WHITE, BLACK);
  tft.setCursor(left_hand_message, 520);
  tft.write(CQ_message, 18);
}

void erase_CQ(void)
{
  char CQ_message[] = "                  ";
  tft.setFontSize(2, true);
  tft.textColor(BLACK, BLACK);
  tft.setCursor(left_hand_message, 520);
  tft.write(CQ_message, 18);
}

void clear_xmit_messages(void)
{
  char xmit_message[] = "                  ";

  tft.setFontSize(2, true);
  tft.setCursor(left_hand_message, 520);
  tft.write(xmit_message, 18);
}
