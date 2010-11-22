#ifndef _DISCLIGHT_H_
#define _DISCLIGHT_H_

#include <gccore.h>

class DiscLight
{
private:
	static lwp_t Thread;
	static bool LightOn;
	static u8 LightLevel;
	static struct timespec TimeOn;
	static struct timespec TimeOff;
private:
	static void* Loop(void *arg);
	static vu32* LightReg;
	static void Turn(bool enable);
public:
	static bool get_LightOn();
	static u8   get_LightLevel();
	static void TurnOn();
	static void TurnOff();	
	static void Toggle();
	static void SetLevel(u8 level);
};

#endif