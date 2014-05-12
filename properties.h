// properties.h

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <stdint.h>

typedef struct tag_properties_t
{
	float ps;
	float pd;
	float temp;
	float alt_raw;
	float alt;
	float vspeed;
	float ias;
	float tas;
}
properties_t;

extern properties_t data properties;

#endif
