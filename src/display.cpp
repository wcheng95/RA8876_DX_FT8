#include <RA8876_t3.h>
extern RA8876_t3 tft;

#include "display.h"
#include <TimeLib.h>

#include <SD.h>
#include <SPI.h>
#include "decode_ft8.h"
#include "ADIF.h"
#include "button.h"


File stationData_File;

extern char file_name_string[24];

extern time_t getTeensy3Time();
extern char Station_Call[]; // six character call sign + /0
extern char Locator[];      // four character locator  + /0
extern char Free_Text1[];
extern char Free_Text2[];

extern int left_hand_message;
extern int kMax_message_length;

extern int Band_Minimum;

int max_log_messages = 10;
display_message_details log_messages[10];
char current_message[40];

int old_rtc_hour;

// Helper to set text properties
static void set_tft_style(uint16_t color, uint16_t bg_color, uint8_t size = 2)
{
  tft.textColor(color, bg_color);
  tft.setFontSize(size, true);
}

void display_value(int x, int y, int value)
{
  char string[6]; // Increased size for safety
  snprintf(string, sizeof(string), "%4i", value);

  set_tft_style(YELLOW, BLACK);
  tft.setCursor(x, y);
  tft.print(string);
}

void show_short(uint16_t x, uint16_t y, uint8_t variable)
{
  char string[4]; // %2i -> up to 2 digits for uint8_t + null
  snprintf(string, sizeof(string), "%2u", variable);
  set_tft_style(YELLOW, BLACK);
  tft.setCursor(x, y);
  tft.print(string);
}

void show_wide(uint16_t x, uint16_t y, int variable)
{
  char string[8];
  snprintf(string, sizeof(string), "%6i", variable);
  set_tft_style(YELLOW, BLACK);
  tft.setCursor(x, y);
  tft.print(string);
}

void display_time(int x, int y)
{
  getTeensy3Time();
  char string[12];
  snprintf(string, sizeof(string), "%02i:%02i:%02i", hour(), minute(), second());

  // This handles midnight rollover to create a new log file for the new day.
  if (hour() < old_rtc_hour)
  {
    Init_Log_File();
    display_date(650, 30);
  }
  old_rtc_hour = hour();

  set_tft_style(WHITE, BLACK);
  tft.setCursor(x, y);
  tft.print(string);
}

void display_date(int x, int y)
{
  getTeensy3Time();
  char string[12];
  snprintf(string, sizeof(string), "%02i/%02i/%04i", day(), month(), year());
  set_tft_style(WHITE, BLACK);
  tft.setCursor(x, y);
  tft.print(string);
}

bool load_station_data(void)
{
  if (!SD.begin(BUILTIN_SDCARD))
  {
    set_tft_style(RED, BLACK);
    tft.setCursor(0, 300);
    tft.print("SD Card not found");
    return false;
  }

  File dataFile = SD.open("StationData.txt", FILE_READ);
  if (!dataFile)
  {
    // File not found, can't load data.
    // This might be a normal condition on first run.
    return true;
  }

  char read_buffer[65]; // +1 for null terminator
  size_t bytes_read = dataFile.read(read_buffer, sizeof(read_buffer) - 1);
  read_buffer[bytes_read] = '\0'; // Null-terminate the buffer
  dataFile.close();

  char *token;

  token = strtok(read_buffer, ":");
  if (token == NULL) return true;
  strcpy(Station_Call, token);

  token = strtok(NULL, ":");
  if (token == NULL) return true;
  strcpy(Locator, token);

  token = strtok(NULL, ":");
  if (token == NULL) return true;
  strcpy(Free_Text1, token);

  token = strtok(NULL, ":");
  if (token == NULL) return true;
  strcpy(Free_Text2, token);

  return true;
}

void display_station_data(int x, int y)
{
  char str[14];
  snprintf(str, sizeof(str), "%7s %4s", Station_Call, Locator);

  set_tft_style(YELLOW, BLACK);
  tft.setCursor(x, y);
  tft.print(str);
}

void display_revision_level(void)
{
  set_tft_style(YELLOW, BLACK);

  tft.setCursor(0, 100);
  tft.print("RA8876_DX FT8");

  tft.setCursor(0, 130);
  tft.print("Hardware: V3.0");

  tft.setCursor(0, 160);
  tft.print("Firmware: V1.0");

  tft.setCursor(0, 190);
  tft.print("W5BAA - WB2CBA");

  tft.setCursor(0, 220);

  if (Band_Minimum == _20M)
    tft.print("Five Band Board");
  else
    tft.print("Seven Band Board");

  set_tft_style(GREEN, BLACK);
  tft.setCursor(0, 270);
  tft.print("Please Wait While");
  tft.setCursor(0, 300);
  tft.print("Gears & Pulleys");
  tft.setCursor(0, 330);
  tft.print("Are Aligned");
}

void show_degrees(uint16_t x, uint16_t y, int32_t variable)
{
  char str[10];
  float scaled_variable, remainder;
  int units, fraction;
  scaled_variable = (float)variable / 10000000;
  units = (int)scaled_variable;
  remainder = scaled_variable - units;
  fraction = (int)(remainder * 1000);
  fraction = abs(fraction);
  snprintf(str, sizeof(str), "%3i.%03i", units, fraction);

  set_tft_style(YELLOW, BLACK);
  tft.setCursor(x, y);
  tft.print(str);
}

void show_decimal(uint16_t x, uint16_t y, float variable)
{
  char str[10];
  int units, fraction;
  float remainder;
  units = (int)variable;
  remainder = variable - units;
  fraction = (int)(remainder * 1000);
  fraction = abs(fraction);
  snprintf(str, sizeof(str), "%3i.%03i", units, fraction);

  set_tft_style(YELLOW, BLACK);
  tft.setCursor(x, y);
  tft.print(str);
}

void clear_log_messages(void)
{
  for (int i = 0; i < max_log_messages; i++)
    log_messages[i].message[0] = '\0';
}

void Be_Patient(void)
{
  tft.fillRect(0, 100, 300, 400, BLACK);
  set_tft_style(YELLOW, BLACK);
  tft.setCursor(0, 120);
  tft.print("Patience, Synching");
}

void update_message_log_display(int mode)
{
  // Shift messages up
  for (int i = 0; i < max_log_messages - 1; i++)
  {
    log_messages[i] = log_messages[i + 1];
  }

  // Add new message at the bottom
  strncpy(log_messages[max_log_messages - 1].message, current_message, sizeof(log_messages[0].message) - 1);
  log_messages[max_log_messages - 1].message[sizeof(log_messages[0].message) - 1] = '\0'; // Ensure null termination
  log_messages[max_log_messages - 1].text_color = (mode != 0);

  // Redraw the log area
  tft.fillRect(left_hand_message, 100, 260, 400, BLACK);
  set_tft_style(WHITE, BLACK); // Default, will be overridden in loop

  for (int i = 0; i < max_log_messages; i++)
  {
    if (log_messages[i].text_color)
      set_tft_style(YELLOW, BLACK);
    else
      set_tft_style(RED, BLACK);

    tft.setCursor(left_hand_message, 100 + i * 40);
    tft.print(log_messages[i].message);
  }
}
