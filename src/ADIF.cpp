/*
 * ADIF.c
 *
 *  Created on: Jun 18, 2023
 *      Author: Charley
 */
#include "ADIF.h"
#include "gen_ft8.h"
#include "decode_ft8.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "button.h"
#include "display.h"
#include "RA8876_t3.h"
#include <TimeLib.h>
#include <SD.h>
#include "EM29_4000.h"  //the picture
#include "EM29_8000.h"  //the picture
#include "EM29_16000.h" //the picture
#include "JM29_4000.h"  //the picture
#include "JM29_8000.h"  //the picture
#include "JM29_16000.h" //the picture

extern RA8876_t3 tft;
extern time_t getTeensy3Time();

extern Decode new_decoded[];

Map_Memory stored_log_entries[100];
int number_logged;

char log_rtc_date_string[9];
char log_rtc_time_string[9];

char file_name_string[24];

extern int BandIndex;
extern char Station_Call[];
extern char Locator[];

extern FreqStruct sBand_Data[];
char display_frequency[9];

extern char Target_Call[];    // six character call sign + /0
extern char Target_Locator[]; // four character locator  + /0
extern int Target_RSL;        // four character RSL  + /0
extern int Station_RSL;

uint16_t scaled_distance;

int ADIF_distance;
int ADIF_map_distance;
int ADIF_map_bearing;

extern int16_t map_width;
extern int16_t map_heigth;
extern int16_t map_center_x;
extern int16_t map_center_y;

double deg2rad(double deg);
double rad2deg(double rad);

double deg2rad(double deg);
double rad2deg(double rad);
        
float Latitude, Longitude;
float Station_Latitude, Station_Longitude;
float Map_Latitude, Map_Longitude;
float Target_Latitude, Target_Longitude;

File Log_File;

int MapIndex;

void make_date(void)
{
  getTeensy3Time();
  sprintf(log_rtc_date_string, "%4i%2i%2i", year(), month(), day());
  for (int i = 0; i < 9; i++)
    if (log_rtc_date_string[i] == 32)
      log_rtc_date_string[i] = 48;
}

void make_time(void)
{
  getTeensy3Time();
  sprintf(log_rtc_time_string, "%2i%2i%2i", hour(), minute(), second());
  for (int i = 0; i < 9; i++)
    if (log_rtc_date_string[i] == 32)
      log_rtc_date_string[i] = 48;
}

void make_File_Name(void)
{
  make_date();
  sprintf((char *)file_name_string, "%s.adi", log_rtc_date_string);
  for (int i = 0; i < 24; i++)
    if (file_name_string[i] == 32)
      file_name_string[i] = 48;
}

void write_log_data(char *data)
{
  Log_File = SD.open(file_name_string, FILE_WRITE);
  Log_File.println(data);
  Log_File.close();
}

void Open_Log_File(void)
{
  Log_File = SD.open(file_name_string, FILE_WRITE);

  if (Log_File.size() == 0)
  {
    Log_File.println("ADIF EXPORT");
    Log_File.println("<eoh>");
  }

  Log_File.close();
}

void Init_Log_File(void)
{
  make_File_Name();
  delay(10);
  Open_Log_File();
}

void write_ADIF_Log()
{
  static char log_line[220];

  make_time();
  make_date();
  strcpy(display_frequency, sBand_Data[BandIndex].display);

  sprintf(log_line,
          "<call:7>%7s"
          "<gridsquare:4>%4s"
          "<mode:3>FT8<qso_date:8>%8s "
          "<time_on:6>%6s"
          "<freq:9>%9s"
          "<station_callsign:7>%7s "
          "<my_gridsquare:4>%4s "
          "<rst_sent:3:N>%3i "
          "<rst_rcvd:3:N>%3i "
          "<tx_pwr:4>0.5 <eor>",

          Target_Call,
          Target_Locator,
          log_rtc_date_string,
          log_rtc_time_string,
          display_frequency,
          Station_Call,
          Locator,
          Target_RSL,
          Station_RSL);

  write_log_data(log_line);

  if(validate_locator(Target_Locator))
  ADIF_distance = Target_Distance(Target_Locator);
  else
  ADIF_distance = 0;

  if (ADIF_distance > 0)
  {
    ADIF_map_distance = Map_Distance(Target_Locator);
    ADIF_map_bearing = Map_Bearing(Target_Locator);

    draw_vector((float)ADIF_map_distance, (float)ADIF_map_bearing, 3, 3);
    stored_log_entries[number_logged].ADIF_map_distance = ADIF_map_distance;
    stored_log_entries[number_logged].ADIF_map_bearing = ADIF_map_bearing;
    number_logged++;
  }
}

int16_t start_x;
int16_t start_y;
int16_t center_x;
int16_t center_y;

int16_t map_width;
int16_t map_heigth;
int16_t map_center_x;
int16_t map_center_y;

char map_locator[] = "      ";

int map_key_index;

Map_Parameters MapFiles[] = {

    {0,
     "EM29",
     431,
     428,
     216,
     212,
     20.0,
     4000.0},

    {1,
     "EM29",
     428,
     432,
     216,
     216,
     40.0,
     8000.0},

    {2,
     "EM29",
     432,
     427,
     218,
     213,
     80.0,
     16000.0},

    {3,
     "JM29",
     431,
     432,
     217,
     215,
     20.0,
     4000.0},

    {4,
     "JM29",
     431,
     429,
     216,
     215,
     40.0,
     8000.0},

    {5,
     "JM29",
     430,
     431,
     213,
     217,
     80.0,
     16000.0}

};

void draw_map(int16_t index)
{

  map_width = MapFiles[index].map_width;
  map_heigth = MapFiles[index].map_heigth;
  map_center_x = MapFiles[index].map_center_x;
  map_center_y = MapFiles[index].map_center_y;

  switch (index)
  {

  case 0:

    drawImage(map_width, map_heigth, map_center_x, map_center_y, (uint16_t *)EM29_4000);

    break;

  case 1:

    drawImage(map_width, map_heigth, map_center_x, map_center_y, (uint16_t *)EM29_8000);

    break;

  case 2:

    drawImage(map_width, map_heigth, map_center_x, map_center_y, (uint16_t *)EM29_16000);

    break;

  case 3:

    drawImage(map_width, map_heigth, map_center_x, map_center_y, (uint16_t *)JM29_4000);

    break;

  case 4:

    drawImage(map_width, map_heigth, map_center_x, map_center_y, (uint16_t *)JM29_8000);

    break;

  case 5:

    drawImage(map_width, map_heigth, map_center_x, map_center_y, (uint16_t *)JM29_16000);

    break;
  }

  strcpy(map_locator, MapFiles[index].map_locator);
  map_key_index = index;
  draw_QTH();
  drawButton(14);
  drawButton(15);
  draw_stored_entries();
}

void draw_stored_entries(void)
{

  for (int j = 0; j < number_logged; j++)
    draw_vector(stored_log_entries[j].ADIF_map_distance, stored_log_entries[j].ADIF_map_bearing, 3, 3);
}

void drawImage(uint16_t image_width, uint16_t image_height, uint16_t image_x, uint16_t image_y, const uint16_t *image)
{

  start_x = (1023 - image_width);
  start_y = (100);
  center_x = (start_x + image_x);
  center_y = (100 + image_y);

  tft.fillRect(588, 100, 435, 435, BLACK);

  tft.writeRect(start_x, start_y, image_width, image_height, image);

  tft.drawLine(center_x - 10, center_y, center_x + 10, center_y, RED); // This puts cross hair on map at map center
  tft.drawLine(center_x, center_y - 10, center_x, center_y + 10, RED);
}

void draw_vector(float distance, float bearing, int size, int color)
{

  float vector_scale = MapFiles[map_key_index].map_scale;
  float max_distance = MapFiles[map_key_index].max_distance;

  float vector_magnitude;
  int plot_X, plot_Y;

  if (distance <= max_distance)
    vector_magnitude = distance / vector_scale;
  else
    vector_magnitude = 199;

  plot_X = (center_x) + (int)(sin(bearing * PI / 180) * vector_magnitude);
  plot_Y = (center_y) - (int)(cos(bearing * PI / 180) * vector_magnitude);

  if (color == 0)
    tft.drawCircleFill(plot_X, plot_Y, size, YELLOW);
  if (color == 1)
    tft.drawCircleFill(plot_X, plot_Y, size, WHITE);
  if (color == 2)
    tft.drawCircleFill(plot_X, plot_Y, size, 0x0400); // Dark Green
  if (color == 3)
    tft.drawCircleFill(plot_X, plot_Y, size, RED);
}

void draw_QTH(void)
{

  float QTH_Distance;
  float QTH_Bearing;

  QTH_Distance = Map_Distance(Locator); // Locator is Station Lacator which is changed when GPS coordinates indcate you are in differnt maidehead area
  QTH_Bearing = Map_Bearing(Locator);
  // Locator is Station Lacator which is changed when GPS coordinates indcate you are in differnt maidehead area

  draw_vector(QTH_Distance, QTH_Bearing, 3, 2);
}

void draw_Map_Center(void)
{
  tft.drawLine(map_center_x - 10, map_center_y, map_center_x + 10, map_center_y, RED);
  tft.drawLine(map_center_x, map_center_y - 10, map_center_x, map_center_y + 10, RED);
}



        
        #include <math.h>
        #include "arm_math.h"
        
        const double EARTH_RAD = 6371;  //radius in km

        void set_Station_Coordinates(char station[]){
        
        	process_locator(station);
        	Station_Latitude = Latitude;
        	Station_Longitude = Longitude;
        }
        
        float Target_Bearing(char target[]) {
        
          float Target_Bearing;
          process_locator(target);
          Target_Latitude = Latitude;
          Target_Longitude = Longitude;
        
          Target_Bearing = (float) bearing((double)Station_Latitude,(double)Station_Longitude,(double)Target_Latitude,(double)Target_Longitude);
          return Target_Bearing;
        }
        
        float Target_Distance(char target[]) {
        
        	float Target_Distance;
        	process_locator(target);
        	Target_Latitude = Latitude;
        	Target_Longitude = Longitude;
         
        	Target_Distance = (float) distance((double)Station_Latitude,(double)Station_Longitude,(double)Target_Latitude,(double)Target_Longitude);
        
        	return Target_Distance;
        }
        
          float Map_Bearing(char target[]) {

          process_locator(map_locator);
        
          Map_Latitude = Latitude;
          Map_Longitude = Longitude;
          
          float Map_Bearing;
          process_locator(target);
          Target_Latitude = Latitude;
          Target_Longitude = Longitude;
        
          Map_Bearing = (float) bearing((double)Map_Latitude,(double)Map_Longitude,(double)Target_Latitude,(double)Target_Longitude);
          return Map_Bearing;
          }
        
          float Map_Distance(char target[]) {

          process_locator(map_locator);
        
          Map_Latitude = Latitude;
          Map_Longitude = Longitude;
        
          float Map_Distance;
          process_locator(target);
          Target_Latitude = Latitude;
          Target_Longitude = Longitude;
         
          Map_Distance = (float) distance((double)Map_Latitude,(double)Map_Longitude,(double)Target_Latitude,(double)Target_Longitude);
        
          return Map_Distance;
         }
        
        
           void process_locator(char locator[]) {
        
        	uint8_t A1, A2, N1, N2;
        	uint8_t A1_value, A2_value, N1_value, N2_value;
        	float Latitude_1, Latitude_2, Latitude_3;
        	float Longitude_1, Longitude_2, Longitude_3;
        
        	A1 = locator[0];
        	A2 = locator[1];
        	N1 = locator[2];
        	N2= locator [3];
        
        	A1_value = A1-65;
        	A2_value = A2-65;
        	N1_value = N1- 48;
        	N2_value = N2 - 48;
        
        	Latitude_1 = (float) A2_value * 10;
        	Latitude_2 = (float) N2_value;
        	Latitude_3 = (11.0/24.0 + 1.0/48.0) - 90.0;
        	Latitude = Latitude_1 + Latitude_2 + Latitude_3;
        
        	Longitude_1 = (float)A1_value * 20.0;
        	Longitude_2 = (float)N1_value * 2.0;
        	Longitude_3 = 11.0/12.0 +  1.0/24.0;
        	Longitude =  Longitude_1  +  Longitude_2 + Longitude_3 - 180.0;
        
          }
        
        
        // distance (km) on earth's surface from point 1 to point 2
            double distance(double lat1, double lon1, double lat2, double lon2) {
            double lat1r = deg2rad(lat1);
            double lon1r = deg2rad(lon1);
            double lat2r = deg2rad(lat2);
            double lon2r = deg2rad(lon2);
            return acos(sin(lat1r) * sin(lat2r)+cos(lat1r) * cos(lat2r) * cos(lon2r-lon1r)) * EARTH_RAD;
        }
        
          double bearing (double lat1, double long1, double lat2, double long2)
          {
        
          double dlon = deg2rad(long2-long1);
          lat1 = deg2rad(lat1);
          lat2 = deg2rad(lat2);
          double a1 = sin(dlon) * cos(lat2);
          double a2 = sin(lat1) * cos(lat2) * cos(dlon);
          a2 = cos(lat1) * sin(lat2) - a2;
          a2 = atan2(a1, a2);
          if (a2 < 0.0)
          {
            a2 += (2 * PI);
          }
             return rad2deg(a2);
         }
        
        
        // convert degrees to radians
        double deg2rad(double deg)
        {
            return deg * (PI / 180.0);
        }
        
        double rad2deg(double rad){
          return (rad * 180) / PI;
        }