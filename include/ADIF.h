/*
 * ADIF.h
 *
 *  Created on: Jun 18, 2023
 *      Author: Charley
 */

#ifndef ADIF_H_
#define ADIF_H_

#include <SD.h>

struct Map_Memory
{
    float ADIF_distance;
    float ADIF_map_distance;
    float ADIF_map_bearing;
};

void draw_map(int16_t index);
void write_ADIF_Log(void);
void Init_Log_File(void);

void set_Station_Coordinates();

extern int16_t map_width;
extern int16_t map_heigth;
extern int16_t map_center_x;
extern int16_t map_center_y;
extern char file_name_string[24];

#endif /* ADIF_H_ */
