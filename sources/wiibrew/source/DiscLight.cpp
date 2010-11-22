#include <cstdio>
#include <ogcsys.h>
#include "DiscLight.h"
#include "Gecko.h"

#define DISC_SLOT_LED 0x20;
const bool On = true;
const bool Off = false;

lwp_t DiscLight::Thread = 0;
vu32 *DiscLight::LightReg = (u32*)0xCD0000C0;
bool DiscLight::LightOn = false;
u8 DiscLight::LightLevel = 0;
struct timespec DiscLight::TimeOn = {0};
struct timespec DiscLight::TimeOff = {0};

void DiscLight::TurnOn()
{
	if (Thread) return;
	LWP_CreateThread(&Thread, Loop, NULL, NULL, 0, 80);
}

void DiscLight::TurnOff()
{
	LightOn = false;
}

void DiscLight::Toggle()
{
	if (LightOn) TurnOff();
	else TurnOn();
}

bool DiscLight::get_LightOn()
{
	return LightOn;
}

u8 DiscLight::get_LightLevel()
{
	return LightLevel;
}

void DiscLight::SetLevel(u8 level)
{
	LightLevel = MIN(level, 255);

	u32 levelOn = level * 40000;
	u32 levelOff = 10200000 - levelOn;

	TimeOn.tv_nsec = levelOn;
	TimeOff.tv_nsec = levelOff;
}

void DiscLight::Turn(bool enable)
{
	u32 value = *LightReg&~DISC_SLOT_LED;
	if (enable) value |= DISC_SLOT_LED;
	*LightReg = value;
}

void* DiscLight::Loop(void *arg)
{
	LightOn = true;
	while (LightOn)
	{
		// Turn on the light and sleep for a bit
		Turn(On);
		nanosleep(&TimeOn);
		// Turn off the light (if required) and sleep for a bit
		if (TimeOff.tv_nsec > 0) Turn(Off);
		nanosleep(&TimeOff);
	}

	// Turn off the light
	Turn(Off);

	Thread = 0;
	return NULL;
}