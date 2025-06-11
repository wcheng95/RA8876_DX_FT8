#ifndef MAIDENHEAD_H
#define MAIDENHEAD_H

char *get_mh(double lat, double lon, int size);
char *complete_mh(char *locator);
double mh2lon(char *locator);
double mh2lat(char *locator);

#endif
