#include <stdio.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <ogc/pad.h>

#include "Controller.h"
#include "Tools.h"
//#include "Main.h"
#include "System.h"
#include "Gecko.h"

int Controller::ScanPads(u32 *button)
{
	*button = 0;
	u32 pressed;
	u16 pressedGC;
	u32 held;
	u32 heldGC;

	WPAD_ScanPads();
	pressed =  WPAD_ButtonsDown(WPAD_CHAN_0) | WPAD_ButtonsDown(WPAD_CHAN_1) | WPAD_ButtonsDown(WPAD_CHAN_2) | WPAD_ButtonsDown(WPAD_CHAN_3);
	held = WPAD_ButtonsHeld(WPAD_CHAN_0) | WPAD_ButtonsHeld(WPAD_CHAN_1) | WPAD_ButtonsHeld(WPAD_CHAN_2) | WPAD_ButtonsHeld(WPAD_CHAN_3);
	
	PAD_ScanPads();
	pressedGC = PAD_ButtonsDown(PAD_CHAN0) | PAD_ButtonsDown(PAD_CHAN1) | PAD_ButtonsDown(PAD_CHAN2) | PAD_ButtonsDown(PAD_CHAN3);
	heldGC = PAD_ButtonsHeld(PAD_CHAN0) | PAD_ButtonsHeld(PAD_CHAN1) | PAD_ButtonsHeld(PAD_CHAN2) | PAD_ButtonsHeld(PAD_CHAN3);

	if (pressed || pressedGC)
	{
		if (pressedGC) usleep(20000);

		pressed += (held - pressed);
		pressedGC += (heldGC - pressedGC);
		
		if (pressed&WPAD_BUTTON_A || pressed&WPAD_CLASSIC_BUTTON_A || pressedGC&PAD_BUTTON_A) *button = WPAD_BUTTON_A;
		if (pressed&WPAD_BUTTON_B || pressed&WPAD_CLASSIC_BUTTON_B || pressedGC&PAD_BUTTON_B) *button += WPAD_BUTTON_B;
		if (pressed&WPAD_BUTTON_LEFT || pressed&WPAD_CLASSIC_BUTTON_LEFT || pressedGC&PAD_BUTTON_LEFT) *button += WPAD_BUTTON_LEFT;
		if (pressed&WPAD_BUTTON_RIGHT || pressed&WPAD_CLASSIC_BUTTON_RIGHT || pressedGC&PAD_BUTTON_RIGHT) *button += WPAD_BUTTON_RIGHT;
		if (pressed&WPAD_BUTTON_UP || pressed&WPAD_CLASSIC_BUTTON_UP || pressedGC&PAD_BUTTON_UP) *button += WPAD_BUTTON_UP;
		if (pressed&WPAD_BUTTON_DOWN || pressed&WPAD_CLASSIC_BUTTON_DOWN || pressedGC&PAD_BUTTON_DOWN) *button += WPAD_BUTTON_DOWN;
		if (pressed&WPAD_BUTTON_HOME || pressed&WPAD_CLASSIC_BUTTON_HOME || pressedGC&PAD_BUTTON_START) *button += WPAD_BUTTON_HOME;
		if (pressed&WPAD_BUTTON_MINUS || pressed&WPAD_CLASSIC_BUTTON_MINUS || pressedGC&PAD_TRIGGER_L) *button += WPAD_BUTTON_MINUS;
		if (pressed&WPAD_BUTTON_PLUS || pressed&WPAD_CLASSIC_BUTTON_PLUS || pressedGC&PAD_TRIGGER_R) *button += WPAD_BUTTON_PLUS;
		if (pressed&WPAD_BUTTON_1) *button += WPAD_BUTTON_1;
		if (pressed&WPAD_BUTTON_2) *button += WPAD_BUTTON_2;
	}

	return 1;
}

void Controller::WaitAnyKey() 
{ 
	u32 button;
	while (ScanPads(&button))
	{
		if (button || System::State != SystemState::Running) return;
	}
}

u32 Controller::WaitKey(u32 button) 
{
	u32 pressed;
	
	do
	{
		ScanPads(&pressed);
		if (pressed == WPAD_BUTTON_HOME) System::Exit();
		if (System::State != SystemState::Running) return 0;
	} while (!(pressed & button));

	return pressed;		
}

extern "C"
{
	int ScanPads(u32 *button) { return Controller::ScanPads(button); }
	void WaitAnyKey() { Controller::WaitAnyKey(); }
	u32 WaitKey(u32 button) { return Controller::WaitKey(button); }
}