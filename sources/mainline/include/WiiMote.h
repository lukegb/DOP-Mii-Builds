#ifndef _WIIMOTE_H_
#define _WIIMOTE_H_

#define WIIUSE_INTERNAL_H_INCLUDED
#include <wiiuse/wiiuse.h>
#include <wiiuse/wpad.h>

class WiiMote
{
public:
	static void SetLeds(int chan, int wiimote_led_x);
};

#endif