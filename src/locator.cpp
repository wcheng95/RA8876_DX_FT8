/*
 * locator.c
 *
 *  Created on: Nov 4, 2019
 *      Author: user
 */

#include <Arduino.h>
#include "locator.h"
#include <math.h>
#include "arm_math.h"

const double EARTH_RAD = 6371; // radius in km

void process_locator(const char *locator);
double distance(double lat1, double lon1, double lat2, double lon2);
double bearing(double lat1, double long1, double lat2, double long2);
float Target_Distance(const char *locator);
float Target_Bearing(const char *locator);
float Map_Distance(const char *locator);
float Map_Bearing(const char *locator);

double deg2rad(double deg);
double rad2deg(double rad);

float Latitude, Longitude;
float Station_Latitude, Station_Longitude;
float Map_Latitude, Map_Longitude;
float Target_Latitude, Target_Longitude;

extern const char *map_locator;

void set_Station_Coordinates(const char *station)
{
  process_locator(station);
  Station_Latitude = Latitude;
  Station_Longitude = Longitude;
}

float Target_Bearing(const char *target)
{
  process_locator(target);
  Target_Latitude = Latitude;
  Target_Longitude = Longitude;

  return (float)bearing((double)Station_Latitude, (double)Station_Longitude, (double)Target_Latitude, (double)Target_Longitude);
}

float Target_Distance(const char *target)
{
  process_locator(target);
  Target_Latitude = Latitude;
  Target_Longitude = Longitude;

  return (float)distance((double)Station_Latitude, (double)Station_Longitude, (double)Target_Latitude, (double)Target_Longitude);
}

float Map_Bearing(const char *target)
{
  process_locator(map_locator);

  Map_Latitude = Latitude;
  Map_Longitude = Longitude;

  process_locator(target);
  Target_Latitude = Latitude;
  Target_Longitude = Longitude;

  return (float)bearing((double)Map_Latitude, (double)Map_Longitude, (double)Target_Latitude, (double)Target_Longitude);
}

float Map_Distance(const char *target)
{
  process_locator(map_locator);

  Map_Latitude = Latitude;
  Map_Longitude = Longitude;

  process_locator(target);
  Target_Latitude = Latitude;
  Target_Longitude = Longitude;

  return (float)distance((double)Map_Latitude, (double)Map_Longitude, (double)Target_Latitude, (double)Target_Longitude);
}

void process_locator(const char *locator)
{
  #if 0
    if (locator && strlen(locator) == 4) {
      Latitude = ((locator[1] - 'A') * 10.0f) + (locator[3] - '0') + ((11.0f / 24.0f + 1.0f / 48.0f) - 90.0f);
      Longitude = ((locator[0] - 'A') * 20.0f) + ((locator[2] - '0') * 2.0f) + (11.0f / 12.0f + 1.0f / 24.0f) - 180.0f;
    }
  #endif
}

// distance (km) on earth's surface from point 1 to point 2
double distance(double lat1, double lon1, double lat2, double lon2)
{
  double lat1r = deg2rad(lat1);
  double lon1r = deg2rad(lon1);
  double lat2r = deg2rad(lat2);
  double lon2r = deg2rad(lon2);
  return acos(sin(lat1r) * sin(lat2r) + cos(lat1r) * cos(lat2r) * cos(lon2r - lon1r)) * EARTH_RAD;
}

double bearing(double lat1, double long1, double lat2, double long2)
{
  double dlon = deg2rad(long2 - long1);
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

double rad2deg(double rad)
{
  return (rad * 180) / PI;
}
