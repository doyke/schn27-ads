// config.c

#include <common.h>
#include <config.h>
#include <F410_FlashPrimitives.h>
#include <crc.h>
#include <maintimer.h>

static CONFIG code configStored _at_ 0x7a00;
CONFIG data config;
static uint8_t modified_timer = 0;
static uint8_t changeCnt = 0;


static void setDefault(void);


// read config from flash
void config_init(void)
{
	uint16_t i = sizeof(config);
	uint8_t *ptr = (uint8_t *)&config;
	uint16_t addr = (uint16_t)&configStored;

	while (i--)
		*ptr++ = FLASH_ByteRead(addr++);

	if (crc8((uint8_t *)&config, sizeof(config) - 1) != config.crc)
		setDefault();
}


// write config to flash
void config_store(void)
{
	uint16_t i = sizeof(CONFIG);
	uint8_t *ptr = (uint8_t *)&config;
	uint16_t addr = (uint16_t)&configStored;

	config.crc = crc8((uint8_t *)&config, sizeof(config) - 1);	// update crc

	mainTimer_enable(0);
	INTERRUPT_DISABLE();

	FLASH_PageErase(addr);

	while (i--)
		FLASH_ByteWrite(addr++, *ptr++);

	INTERRUPT_ENABLE();
	mainTimer_enable(1);
}


// check modification timeout
void config_checkModified(void)
{
	if (modified_timer)
	{
		if (--modified_timer == 0)
			config_store();
	}
}


// lauch modification timeout
void config_setModified(void)
{
	modified_timer = 100;
	++changeCnt;
}


uint8_t config_getChangeCnt(void)
{
	return changeCnt;
}

static void setDefault(void)
{
	config.addr = 255;
	config.ps0 = 101325;
	config.pd0 = 0;
	config.temp0 = 288;

	config.kref = 2.0f;
	config.kps = 2.0059f;
	config.kpd = 2.0058f;

	config.var_a = 0.03f;
	config.var_h = 0.1f;
	config.lpf_alpha = 0.1f;

	config_store();
}

