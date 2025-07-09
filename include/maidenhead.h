#ifndef MAIDENHEAD_H
#define MAIDENHEAD_H

const char *get_mh(double lat, double lon, int size);
const char *complete_mh(const char *locator);

#endif
