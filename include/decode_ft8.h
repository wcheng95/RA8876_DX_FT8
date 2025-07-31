/*
 * decode_ft8.h
 *
 *  Created on: Nov 2, 2019
 *      Author: user
 */

#ifndef DECODE_FT8_H_
#define DECODE_FT8_H_

int ft8_decode(void);

enum Sequence
{
    Seq_RSL = 0,
    Seq_Locator
};

struct Decode
{
    char field1[14];
    char field2[14];
    char field3[7];
    int freq_hz;
    char decode_time[10];
    int sync_score;
    int snr;
    int received_snr;
    char target_locator[7];
    int slot;
    int RR73;
    Sequence sequence;
};

struct display_message_details
{
    char message[40];
    int text_color;
};

struct Calling_Station
{
    int number_times_called;
    char call[14];
    char locator[7];
    int RSL;
    int received_RSL;
    int RR73;
    Sequence sequence;
};

int Check_Calling_Stations(int num_decoded);

void process_selected_Station(int stations_decoded, int TouchIndex);

void clear_decoded_messages(void);
void clear_log_stored_data(void);

int validate_locator(const char *QSO_locator);
int strindex(const char *s, const char *t);

extern int Auto_QSO_State;
extern struct Decode new_decoded[];
extern size_t kMax_message_length;

#endif /* DECODE_FT8_H_ */
