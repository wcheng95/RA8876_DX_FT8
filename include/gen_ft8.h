/*
 * gen_ft8.h
 *
 *  Created on: Oct 30, 2019
 *      Author: user
 */

#ifndef GEN_FT8_H_
#define GEN_FT8_H_

#define MESSAGE_SIZE 28

enum ReplyID
{
    Reply_RSL,
    Reply_R_RSL,
    Reply_Beacon_73,
    Reply_QSO_73
};

enum QueID
{
    Que_Locator,
    Que_RSL,
    Que_73,
    Que_Size
};

void clear_reply_message_box(void);
void erase_CQ(void);
void clear_xmit_messages(void);

// Generate FT8 tone sequence from payload data
// [IN] payload - 9 byte array consisting of 72 bit payload
// [OUT] itone  - array of NN (79) bytes to store the generated tones (encoded as 0..7)
void genft8(const uint8_t *payload, uint8_t *itone);

extern int Station_RSL;
extern char Target_Call[11];    // six character call sign + /0
extern int Target_RSL;
extern char Target_Locator[7]; // four character locator  + /0
extern char Free_Text1[MESSAGE_SIZE];
extern char Free_Text2[MESSAGE_SIZE];
extern int left_hand_message;

void queue_custom_text(const char *tx_msg);

#endif /* GEN_FT8_H_ */
