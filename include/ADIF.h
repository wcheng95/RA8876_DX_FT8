/*
 * ADIF.h
 *
 *  Created on: Jun 18, 2023
 *      Author: Charley
 */

#ifndef ADIF_H_
#define ADIF_H_

#include <SD.h>

typedef struct
{
    int16_t index;
    const char *map_locator;
    int16_t map_width;
    int16_t map_heigth;
    int16_t map_center_x; // from left edge
    int16_t map_center_y;
    float map_scale;
    float max_distance;
} Map_Parameters;

typedef struct
{
    float ADIF_distance;
    float ADIF_map_distance;
    float ADIF_map_bearing;

} Map_Memory;

void draw_map(int16_t index);

void write_ADIF_Log(void);

void make_date(void);
void make_File_Name(void);
bool open_log_file(void);
void write_log_data(char *data);
void Init_Log_File(void);

void drawImage(uint16_t image_width, uint16_t image_height, uint16_t image_x, uint16_t image_y, const uint16_t *image);
void draw_vector(float distance, float bearing, int size, int color);
void draw_QTH(void);
void draw_stored_entries(void);


void set_Station_Coordinates(char station[]);
void process_locator(char locator[]);
double distance(double lat1, double lon1, double lat2, double lon2);
double bearing (double lat1, double long1, double lat2, double long2);
float Target_Distance(char target[]);
float Target_Bearing(char target[]);

float Map_Distance(char target[]);
float Map_Bearing(char target[]);
        
        
        
        









#endif /* ADIF_H_ */
