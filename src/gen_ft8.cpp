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

char Target_Call[11];   // six character call sign + /0
char Target_Locator[7]; // six character locator  + /0
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

char ft8_time_string[9] = "15:44:15";
int left_hand_message = 300;
char xmit_messages[3][MESSAGE_SIZE];


void clear_reply_message_box(void)
{
  tft.fillRect(left_hand_message, 100, 290, 420, BLACK);
}

char Free_Text1[MESSAGE_SIZE] = "FreeText 1   ";
char Free_Text2[MESSAGE_SIZE] = "FreeText 2   ";

void erase_CQ(void)
{
  char CQ_message[] = "                  ";
  tft.setFontSize(2, true);
  tft.textColor(BLACK, BLACK);
  tft.setCursor(left_hand_message, 520);
  tft.write(CQ_message, 18);
}


// Needed by autoseq_engine
void queue_custom_text(const char *tx_msg)
{
	uint8_t packed[K_BYTES];

	pack77(tx_msg, packed);
	genft8(packed, tones);

}

