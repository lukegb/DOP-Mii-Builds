#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_


class Controller
{
public:
	static int ScanPads(u32 *button);
	static void WaitAnyKey();
	static u32 WaitKey(u32 button);
};

#endif