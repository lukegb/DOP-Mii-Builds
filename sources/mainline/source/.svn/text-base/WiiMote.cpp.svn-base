#define WIIUSE_INTERNAL_H_INCLUDED
#include <wiiuse/wpad.h>
#include <wiiuse/wiiuse.h>
#include "__wpads.h"
#include "WiiMote.h"
#include "Gecko.h"
#include <ogc/machine/processor.h>

#define WIIMOTE_STATE_CONNECTED					0x00010
#define WIIMOTE_STATE_HANDSHAKE					0x00004	/* actual connection exists but no handshake yet */
#define WIIMOTE_STATE_HANDSHAKE_COMPLETE		0x00008	/* actual connection exists but no handshake yet */
#define WIIMOTE_STATE_RUMBLE					0x00080

#define WIIMOTE_IS_SET(wm, s)			((wm->state & (s)) == (s))
#define WIIMOTE_ENABLE_STATE(wm, s)		(wm->state |= (s))
#define WIIMOTE_DISABLE_STATE(wm, s)	(wm->state &= ~(s))


void WiiMote::SetLeds(int chan, int wiimote_led_x)
{
	int level;
	_CPU_ISR_Disable(level);
	struct wiimote_t **__wpads = *((struct wiimote_t***)__WPADS_ADDR);
	struct wiimote_t *wm = __wpads[chan];

	int status = WPAD_Probe(chan, 0);
	if (status == WPAD_ERR_NONE && wm->leds != 0) wiiuse_set_leds(wm, wiimote_led_x, NULL);
	else if (status == WPAD_ERR_NO_CONTROLLER) wm->leds = 0;
	_CPU_ISR_Restore(level);	
}
