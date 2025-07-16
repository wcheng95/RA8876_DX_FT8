#pragma once

// TFT stuff - change these defs for your specific LCD
#include "arm_math.h"
#include "WString.h" //This defines String data type

void display_value(int x, int y, int value);
void display_time(int x, int y);
void display_date(int x, int y);
void display_station_data(int x, int y);
void show_decimal(uint16_t x, uint16_t y, float variable);
void show_wide(uint16_t x, uint16_t y, int variable);
bool open_stationData_file(void);
void update_message_log_display(int mode);
void clear_log_messages(void);
void Be_Patient(void);
void display_revision_level(void);

extern char current_message[];
