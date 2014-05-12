// config.h

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

typedef struct tag_CONFIG
{
	uint8_t addr;			// rs485 addr
	float ps0;
	float pd0;
	float temp0;
	float kref;
	float kps;
	float kpd;
	float var_h;
	float var_a;
	float lpf_alpha;
	uint8_t crc;
} CONFIG;

extern CONFIG data config;

void config_init(void);
void config_store(void);
void config_checkModified(void);
void config_setModified(void);
uint8_t config_getChangeCnt(void);

#endif
