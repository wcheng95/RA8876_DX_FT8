/*
 * gen_ft8.h
 *
 *  Created on: Oct 30, 2019
 *      Author: user
 */

#ifndef GEN_FT8_H_
#define GEN_FT8_H_


#define MESSAGE_SIZE 28

#include "arm_math.h"

typedef enum _ReplyID {
    Reply_RSL,
    Reply_R_RSL,
    Reply_Beacon_73,
    Reply_QSO_73
} ReplyID;

typedef enum _QueID {
    Que_Locator,
    Que_RSL,
    Que_73,
    Que_Size
} QueID;

void set_reply(ReplyID replyId);

void set_cq(void);



void compose_messages(void);
void que_message(int index);
void clear_reply_message_box(void);
void erase_CQ(void);
void clear_qued_message(void);
void clear_xmit_messages(void);

int in_range(int num, int min, int max);

#endif /* GEN_FT8_H_ */
