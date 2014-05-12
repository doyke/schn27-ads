// mainloop.c

#include <common.h>
#include <adc.h>
#include <interface.h>
#include <maintimer.h>
#include <config.h>
#include <properties.h>
#include <arraysize.h>
#include <math.h>
#include <temperature.h>


static void estimator_update(uint8_t curtime, uint8_t init);
static uint8_t isConfigChanged(void);


void mainLoop(void)
{
	uint16_t data ps_raw = 0;
	uint16_t data pd_raw = 0;
	uint8_t time = 0;
	int startCnt = 100;

	for (;;)
	{
		uint8_t curtime = mainTimer_getTime();

		if (adc_get(&ps_raw, &pd_raw))				// new ADC readings ?
		{
			#define KPS (config.kps / config.kref / 65536.0f)
			#define KPD (config.kpd / config.kref / 65536.0f)

			float ps = ((ps_raw * KPS + 0.095f) / 0.009f) * 1000.0f;
			float pd = 5000.0f * pd_raw * KPD - 1000.0f;

			if (startCnt != 0)
			{
				--startCnt;
				properties.ps = ps;
				properties.pd = pd;
			}
			else
			{
				properties.ps = properties.ps * (1 - config.lpf_alpha) + ps * config.lpf_alpha;
				properties.pd = properties.pd * (1 - config.lpf_alpha) + pd * config.lpf_alpha;
			}

			#define b (-0.0065f)
			#define R (287.058f)
			#define g (9.80665f)
			#define r0 (1.225f)

			properties.alt_raw = config.temp0 / b * (pow(properties.ps / config.ps0, -R * b / g) - 1);
			estimator_update(curtime, isConfigChanged() || startCnt != 0);

			properties.ias = (properties.pd > config.pd0) ? sqrt(2 * (properties.pd - config.pd0) / r0) : 0;
			properties.tas = properties.ias * sqrt(r0 * R * properties.temp / properties.ps);
		}

		if (time != curtime)
		{
			time = curtime;
			interface_update();
			config_checkModified();
			temperature_update();
		}
	}
}


/////////////////////////////////////////////////////////////////


static uint8_t isConfigChanged(void)
{
	static uint8_t changeCnt = 0xFF;

	if (changeCnt != config_getChangeCnt())
	{
		changeCnt = config_getChangeCnt();
		return 1;
	}

	return 0;
}


static void estimator_update(uint8_t curtime, uint8_t init)
{
	static uint8_t time = 0;
	const float var_h = config.var_h;
	const float var_a = config.var_a;
	static float p_hh = 1.e10;
	static float p_hv = 0;
	static float p_vv = 0;

	float delta, s_inv, k_h, k_v;

	const float dt = ((uint8_t)(curtime - time)) * 0.001f;
	time = curtime;

	if (init)
	{
		properties.alt = properties.alt_raw;	// cancel parasitic step response
		return;
	}

	// Predict step.
	// Update state estimate.
	properties.alt += properties.vspeed * dt;

	// Update state covariance. The last term mixes in acceleration noise.
	p_hh += 2.0f * dt * p_hv + dt * dt * p_vv + var_a * dt * dt * dt * dt / 4.0f;
	p_hv +=                         dt * p_vv + var_a * dt * dt * dt / 2.0f;
	p_vv +=                                   + var_a * dt * dt;

	// Update step.
	delta = properties.alt_raw - properties.alt;  // Innovation.
	s_inv = 1.0f / (p_hh + var_h);  // Innovation precision.
	k_h = p_hh * s_inv;  // Kalman gain
	k_v = p_hv * s_inv;

	// Update state estimate.
	properties.alt += k_h * delta;
	properties.vspeed += k_v * delta;

	// Update state covariance.
    p_vv -= p_hv * k_v;
	p_hv -= p_hv * k_h;
	p_hh -= p_hh * k_h;
}





