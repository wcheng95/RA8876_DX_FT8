
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "maidenhead.h"

static char letterize(int x)
{
    return (char)x + 'A';
}

const char *get_mh(double lat, double lon, int size)
{
    static const double LON_F[] = {20, 2.0, 0.083333, 0.008333, 0.0003472083333333333};
    static const double LAT_F[] = {10, 1.0, 0.0416665, 0.004166, 0.0001735833333333333};

    static char locator[11];
    int i;
    lon += 180;
    lat += 90;

    if (size <= 0 || size > 10)
        size = 6;
    size /= 2;
    size *= 2;

    for (i = 0; i < size / 2; i++)
    {
        if (i % 2 == 1)
        {
            locator[i * 2] = (char)(lon / LON_F[i] + '0');
            locator[i * 2 + 1] = (char)(lat / LAT_F[i] + '0');
        }
        else
        {
            locator[i * 2] = letterize((int)(lon / LON_F[i]));
            locator[i * 2 + 1] = letterize((int)(lat / LAT_F[i]));
        }
        lon = fmod(lon, LON_F[i]);
        lat = fmod(lat, LAT_F[i]);
    }
    locator[i * 2] = 0;
    return locator;
}

const char *complete_mh(const char *locator)
{
    static char locator2[11] = "LL55LL55LL";
    int len = strlen(locator);
    if (len >= 10)
        return locator;

    memcpy(locator2, locator, strlen(locator));
    return locator2;
}
