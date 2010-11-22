#ifndef _VIDEO_H_
#define _VIDEO_H_

#define UpArrow "\x1E"
#define DownArrow "\x1F"
#define LeftArrow "\x11"
#define RightArrow "\x10"
#define AnsiYellowBoldFG "\x1b[33;1m"
#define AnsiSelection "\x1b[47;0m\x1b[30;0m"
#define AnsiNormal "\x1b[40;0m\x1b[37;0m"

enum class Color
{
	Black = 0,
	Red = 1,
	Green = 2,
	Yellow = 3,
	Blue = 4,
	Magenta = 5,
	Cyan = 6,
	White = 7
};

enum class Bold
{
	Off = 0,
	On = 1
};

class Console
{
public:
	static u32 Cols;
	static u32 Rows;	

	static void ClearLine();
	static void ClearScreen();
	static int  GetCurrentRow();
	static int  GetCurrentCol();
	static void SetPosition(u8 row, u8 column);
	static void SetColPosition(u8 column);
	static void SetRowPosition(u8 row);

	static void SetColors(Color bgColor, Color fgColor);
	static void SetColors(Color bgColor, Bold bgBold, Color fbColor, Bold fgBold);
	static void SetBgColor(Color color);
	static void SetBgColor(Color color, Bold bold);
	static void SetFgColor(Color color);
	static void SetFgColor(Color color, Bold bold);
	
	static void PrintCenter(int width, const char *fmt, ...);
	static void PrintCenterGC(int width, const char *fmt, ...);
	static void PrintSolidLine();
	static void PrintSolidLine(bool includeNewLineBefore);
	static void ResetColors();
	static bool PromptYesNo();
	static bool PromptContinue();
	static void WaitForA(bool canReturn);
	static void WaitForA();
};

class Video
{
private: // Fields
	static u32* FrontBuffer;
	static GXRModeObj* VMode;
public: // Fields
	static u32 Width;	
	static u32 Height;
public:	// Methods
	static void Initialize();	
};

#endif
