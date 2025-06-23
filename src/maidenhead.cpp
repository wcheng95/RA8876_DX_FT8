
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <string> // For std::string

// Named constants for Maidenhead calculations
static const double LON_OFFSET = 180.0;
static const double LAT_OFFSET = 90.0;
static const double LON_FIELD_WIDTH = 20.0;
static const double LAT_FIELD_WIDTH = 10.0;
static const double LON_SQUARE_WIDTH = 2.0;
static const double LAT_SQUARE_WIDTH = 1.0;
static const double LON_SUBSQUARE_WIDTH = 1.0 / 12.0; // 2.5 degrees / 24 = 0.083333
static const double LAT_SUBSQUARE_WIDTH = 1.0 / 24.0; // 1.25 degrees / 24 = 0.0416665
static const double LON_EXTSQUARE_WIDTH = 1.0 / 120.0; // 0.008333
static const double LAT_EXTSQUARE_WIDTH = 1.0 / 240.0; // 0.004166

#include "maidenhead.h"

static char letterize(int x)
{
    return (char)x + 'A';
}

std::string get_mh(double lat, double lon, int size)
{
    // These factors are derived from the widths of fields, squares, etc.
    // LON_F: 20 deg/field, 2 deg/square, 1/12 deg/subsquare, 1/120 deg/extsquare, 1/2880 deg/precsquare
    // LAT_F: 10 deg/field, 1 deg/square, 1/24 deg/subsquare, 1/240 deg/extsquare, 1/5760 deg/precsquare
    static const double LON_F[] = {LON_FIELD_WIDTH, LON_SQUARE_WIDTH, LON_SUBSQUARE_WIDTH * 12, LON_EXTSQUARE_WIDTH * 10, LON_EXTSQUARE_WIDTH / 24}; // Adjusted for direct use
    static const double LAT_F[] = {LAT_FIELD_WIDTH, LAT_SQUARE_WIDTH, LAT_SUBSQUARE_WIDTH * 24, LAT_EXTSQUARE_WIDTH * 10, LAT_EXTSQUARE_WIDTH / 24}; // Adjusted for direct use

    std::string locator_str(10, '\0'); // Max 10 chars for locator + null terminator
    int i;
    lon += LON_OFFSET;
    lat += LAT_OFFSET;

    if (size <= 0 || size > 10)
        size = 6;
    size /= 2;
    size *= 2;

    for (i = 0; i < size / 2; ++i)
    {
        if (i % 2 == 1)
        {
            locator_str[i * 2] = (char)(static_cast<int>(lon / LON_F[i]) + '0');
            locator_str[i * 2 + 1] = (char)(static_cast<int>(lat / LAT_F[i]) + '0');
        }
        else
        {
            locator_str[i * 2] = letterize(static_cast<int>(lon / LON_F[i]));
            locator_str[i * 2 + 1] = letterize(static_cast<int>(lat / LAT_F[i]));
        }
        lon = fmod(lon, LON_F[i]);
        lat = fmod(lat, LAT_F[i]);
    }
    locator_str.resize(i * 2); // Trim to actual size
    return locator_str;
}

std::string complete_mh(const std::string& locator)
{
    if (locator.length() >= 10) {
        return locator; // Already complete or too long
    }
    std::string completed_locator = locator;
    // Fill with default values for a 10-character locator
    // This assumes a standard pattern for completion
    const std::string default_fill = "LL55LL55LL";
    for (size_t i = locator.length(); i < 10; ++i) {
        completed_locator += default_fill[i];
    }
    return completed_locator;
}

double mh2lon(const std::string& locator_str)
{
    std::string locator = locator_str;
    if (locator.length() > 10) return 0.0;
    if (locator.length() < 10) locator = complete_mh(locator);

    double field_val = (locator[0] - 'A') * LON_FIELD_WIDTH;
    double square_val = (locator[2] - '0') * LON_SQUARE_WIDTH;
    double subsquare_val = (locator[4] - 'A') * LON_SUBSQUARE_WIDTH;
    double extsquare_val = (locator[6] - '0') * (LON_SUBSQUARE_WIDTH / 10.0); // 1/120
    double precsquare_val = (locator[8] - 'A') * (LON_SUBSQUARE_WIDTH / 240.0); // 1/2880

    return field_val + square_val + subsquare_val + extsquare_val + precsquare_val - LON_OFFSET;
}

double mh2lat(const std::string& locator_str)
{
    std::string locator = locator_str;
    if (locator.length() > 10) return 0.0;
    if (locator.length() < 10) locator = complete_mh(locator);

    double field_val = (locator[1] - 'A') * LAT_FIELD_WIDTH;
    double square_val = (locator[3] - '0') * LAT_SQUARE_WIDTH;
    double subsquare_val = (locator[5] - 'A') * LAT_SUBSQUARE_WIDTH;
    double extsquare_val = (locator[7] - '0') * (LAT_SUBSQUARE_WIDTH / 10.0); // 1/240
    double precsquare_val = (locator[9] - 'A') * (LAT_SUBSQUARE_WIDTH / 240.0); // 1/5760

    return field_val + square_val + subsquare_val + extsquare_val + precsquare_val - LAT_OFFSET;
}
