// interface.c

#include <common.h>
#include <interface.h>
#include <rs485.h>
#include <crc.h>
#include <config.h>
#include <string.h>
#include <properties.h>

#include <arraysize.h>
#include <version.h>


static uint8_t buffer[64];
static uint8_t bytecnt = 0;

void processPacket(void);
void sendResponse(void);
char isValidAddr(void);
int callHandler(uint8_t *buf);

int handler_getver(uint8_t *buf);
int handler_getstate(uint8_t *buf);
int handler_resetH(uint8_t *buf);
int handler_set(uint8_t *buf);
int handler_resetV(uint8_t *buf);
int handler_setconfig(uint8_t *buf);
int handler_getconfig(uint8_t *buf);
int handler_loaderrequest(uint8_t *buf);


void put_uint16(uint8_t *buf, uint16_t value);
uint16_t get_uint16(uint8_t *buf);
void put_uint32(uint8_t *buf, uint32_t value);
uint32_t get_uint32(uint8_t *buf);
void put_float32(uint8_t *buf, float value);
float get_float32(uint8_t *buf);



void interface_init(void)
{
	rs485_init();
}


void interface_update(void)
{
	char data anydata = 0;

	while (rs485_get(buffer + bytecnt))
	{
		++bytecnt;
		anydata = 1;

		if (bytecnt < 3)
			continue;

		if (buffer[2] > ARRAYSIZE(buffer) || buffer[2] < 4)
			bytecnt = 0;	// invalid size
		else if (bytecnt >= buffer[2])
		{
			processPacket();
			bytecnt = 0;
		}
	}

	if (!anydata)
		bytecnt = 0;
}


static void processPacket(void)
{
	int resp = -1;

	if (!isValidAddr())
		return;

	if (crc8(buffer, buffer[2] - 1) != buffer[buffer[2] - 1])
		return;		// invalid CRC

	if ((resp = callHandler(buffer + 3)) < 0)
		return;		// no response

	buffer[0] = config.addr;
	buffer[1] += 1;
	buffer[2] = resp + 4;
	buffer[buffer[2] - 1] = crc8(buffer, buffer[2] - 1);

	sendResponse();
}


static void sendResponse(void)
{
	int i = 0;
	while (i < buffer[2])
		rs485_put(buffer[i++]);
}


static char isValidAddr(void)
{
	return buffer[0] == 0 
		|| buffer[0] == config.addr;
}


static int callHandler(uint8_t *buf)
{
	switch (buffer[1])
	{
	case 0x00: return handler_getver(buf);
	case 0x02: return handler_getstate(buf);
	case 0x10: return handler_resetH(buf);
	case 0x12: return handler_set(buf);
	case 0x14: return handler_resetV(buf);
	case 0x20: return handler_setconfig(buf);
	case 0x22: return handler_getconfig(buf);
	case 0xF0: return handler_loaderrequest(buf);
	default: return -1;
	}
}


static int handler_getver(uint8_t *buf)
{
	const char *ver = DEVICE_STRING;

	uint8_t n = strlen(ver);
	const uint8_t bufsize = ARRAYSIZE(buffer) - 4;

	if (n < bufsize)
		n = bufsize;

	strncpy(buffer + 3, ver, n);
	
	return n;
}


static int handler_getstate(uint8_t *buf)
{
	put_float32(buf + 0, properties.alt);
	put_float32(buf + 4, properties.vspeed);
	put_float32(buf + 8, properties.ias);
	put_float32(buf + 12, properties.tas);
	put_float32(buf + 16, properties.ps);
	put_float32(buf + 20, properties.pd);
	put_float32(buf + 24, properties.temp);
	put_float32(buf + 28, properties.alt_raw);
	return 32;
}


static int handler_resetH(uint8_t *buf)
{
	config.ps0 = properties.ps;
	config.temp0 = properties.temp;
	config_setModified();
	return 0;
}


static int handler_set(uint8_t *buf)
{
	config.ps0 = get_float32(buf + 0);
	config.temp0 = get_float32(buf + 4);
	config_setModified();
	return 0;
}


static int handler_resetV(uint8_t *buf)
{
	config.pd0 = properties.pd;
	config_setModified();
	return 0;
}


static int handler_setconfig(uint8_t *buf)
{
	config.addr = buf[0];
	config.kref = get_float32(buf + 1);
	config.kps = get_float32(buf + 5);
	config.kpd = get_float32(buf + 9);
	config.var_h = get_float32(buf + 13);
	config.var_a = get_float32(buf + 17);
	config.lpf_alpha = get_float32(buf + 21);
	config_setModified();
	return 0;
}


static int handler_getconfig(uint8_t *buf)
{
	buf[0] = config.addr;
	put_float32(buf + 1, config.kref);
	put_float32(buf + 5, config.kps);
	put_float32(buf + 9, config.kpd);
	put_float32(buf + 13, config.var_h);
	put_float32(buf + 17, config.var_a);
	put_float32(buf + 21, config.lpf_alpha);
	return 25;
}


static int handler_loaderrequest(uint8_t *buf)
{
	RESET();
	return -1;
}


static void put_uint16(uint8_t *buf, uint16_t value)
{
	buf[0] = value & 0xFF;
	buf[1] = value >> 8;
}

static uint16_t get_uint16(uint8_t *buf)
{
	return buf[0] + buf[1] * 256;
}

static void put_uint32(uint8_t *buf, uint32_t value)
{
	buf[0] = value & 0xFF;
	buf[1] = (value >> 8) & 0xFF;
	buf[2] = (value >> 16) & 0xFF;
	buf[3] = (value >> 24) & 0xFF;
}

static uint32_t get_uint32(uint8_t *buf)
{
	return (uint32_t)get_uint16(buf) + (((uint32_t)get_uint16(buf + 2)) << 16);
}

static void put_float32(uint8_t *buf, float value)
{
	put_uint32(buf, *(uint32_t *)&value);
}

static float get_float32(uint8_t *buf)
{
	uint32_t v = get_uint32(buf);
	return *(float *)&v;
}

