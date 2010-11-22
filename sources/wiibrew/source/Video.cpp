#include <cstdlib>
#include <stdarg.h>
#include <wiiuse/wpad.h>

#include "Global.h"
#include "Controller.h"
#include "Video.h"
#include "Gecko.h"
#include "Tools.h"
#include "System.h"
#include "svnrev.h"
#include "BuildType.h"

u32* Video::FrontBuffer = NULL;
GXRModeObj *Video::VMode = NULL;
u32 Console::Cols = 0;
u32 Console::Rows = 0;
u32 Video::Width = 0;
u32 Video::Height = 0;

void Console::SetPosition(u8 row, u8 column)
{
    // The console understands VT terminal escape codes
    // This positions the cursor on row 2, column 0
    // we can use variables for this with format codes too
    // e.g. printf ("\x1b[%d;%dH", row, column );
    printf("\x1b[%u;%uH", row, column);
}

void Console::SetColPosition(u8 column)
{
	Console::SetPosition(Console::GetCurrentRow(), column);
}

void Console::SetRowPosition(u8 row)
{
	Console::SetPosition(row, 0);
}

void Console::ClearLine() 
{
	gcprintf("\r\x1b[2K\r");
	fflush(stdout);
}

void Console::ClearScreen()
{
	/* Clear console */
	printf("\x1b[2J");
	fflush(stdout);
}

int Console::GetCurrentRow()
{
	int col, row;
	CON_GetPosition(&col, &row);
	return row;
}

int Console::GetCurrentCol()
{
	int col, row;
	CON_GetPosition(&col, &row);
	return col;
}

void Console::SetFgColor(Color color)
{
	Console::SetFgColor(color, Bold::Off);
}

void Console::SetFgColor(Color color, Bold bold) 
{
	printf("\x1b[%u;%dm", (u8)color + 30, (u8)bold);
}

void Console::SetBgColor(Color color)
{
	Console::SetBgColor(color, Bold::Off);
}

void Console::SetBgColor(Color color, Bold bold) 
{
	gcprintf("\x1b[%u;%dm", (u8)color + 40, (u8)bold);
}

void Console::PrintCenter(int width, const char *fmt, ...)
{
	char text[4096];

	va_list ap;
	va_start(ap,fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);

	int textLen = strlen(text);
	int leftPad = (width - textLen) / 2;
	int rightPad = (width - textLen) - leftPad;
	if (leftPad < rightPad) 
	{
		leftPad++;
		rightPad--;
	}
	printf("%*s%s%*s", leftPad, " ", text, rightPad, " ");
	memset(text, 0, sizeof(text));
}

void Console::PrintCenterGC(int width, const char *fmt, ...)
{
	char text[4096];
	va_list ap;
	va_start(ap,fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);

	int textLen = strlen(text);
	int leftPad = (width - textLen) / 2;
	int rightPad = (width - textLen) - leftPad;
	if (leftPad < rightPad) 
	{
		leftPad++;
		rightPad--;
	}
	gcprintf("%*s%s%*s", leftPad, " ", text, rightPad, " ");
	memset(text, 0, sizeof(text));
}

void Console::SetColors(Color bgColor, Color fgColor)
{
	Console::SetColors(bgColor, Bold::Off, fgColor, Bold::Off);
}

void Console::SetColors(Color bgColor, Bold bgBold, Color fgColor, Bold fgBold) 
{
	Console::SetFgColor(fgColor, fgBold);
	Console::SetBgColor(bgColor, bgBold);
}

void Console::ResetColors()
{
	Console::SetColors(Color::Black, Bold::Off, Color::White, Bold::Off); 
}

bool Console::PromptYesNo()
{
	gcprintf("[A] Yes    [B] No    [Home] Return To Loader\n");

	u32 button;
	while (Controller::ScanPads(&button))
	{
		if (System::State != SystemState::Running) return false;
		if (button == WPAD_BUTTON_HOME) System::Exit(true);
		if (button == WPAD_BUTTON_A) return true;
		if (button == WPAD_BUTTON_B) return false;		
	}

	return false;
}

bool Console::PromptContinue() 
{
	gcprintf("Are you sure you want to continue?\n");
	return PromptYesNo();
}

void Console::WaitForA(bool canReturn)
{
	if (canReturn) {
	        gcprintf("[A] Continue    [Home] Return To Loader\n");
	} else {
		gcprintf("[A] Continue\n");
	}

        u32 button;
        while (Controller::ScanPads(&button))
        {
                if (System::State != SystemState::Running && canReturn) return;
                if (button == WPAD_BUTTON_HOME && canReturn) System::Exit(true);
                if (button == WPAD_BUTTON_A) return;
        }

        return;
}

void Console::WaitForA()
{
	WaitForA(true);
}

void Console::PrintSolidLine()
{
	PrintSolidLine(true);
}

void Console::PrintSolidLine(bool includeNewLineBefore)
{
	if (includeNewLineBefore) printf("\n");
	for (u32 i = 0; i < Cols ; i++) printf("%c", 196);
}

/* Video */
void Video::Initialize()
{
    // Initialise the video system
    VIDEO_Init();

    // Obtain the preferred video mode from the system
    // This will correspond to the settings in the Wii menu
    VMode = VIDEO_GetPreferredMode(NULL);		

	// Fixes Screen Resolution
	if( VMode->viTVMode == VI_NTSC || CONF_GetEuRGB60() || CONF_GetProgressiveScan() )
		GX_AdjustForOverscan(VMode, VMode, 0, (u16)(VMode->viWidth * 0.026));

    // Set up the video registers with the chosen mode
    VIDEO_Configure(VMode);		

    // Allocate memory for the display in the uncached region
    FrontBuffer = (u32*)MEM_K0_TO_K1(SYS_AllocateFramebuffer(VMode));	
	
	VIDEO_ClearFrameBuffer(VMode, FrontBuffer, COLOR_BLACK);

	// Tell the video hardware where our display memory is	
    VIDEO_SetNextFramebuffer(FrontBuffer);

    // Make the display visible
    VIDEO_SetBlack(FALSE);

    // Flush the video register changes to the hardware
    VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
    if (VMode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
	else while (VIDEO_GetNextField()) VIDEO_WaitVSync();
	
	/* Initialize the Console */
	// Trick to make the banner "stick"
	CON_InitEx(VMode, 0, 0, VMode->viWidth, VMode->viHeight);
	CON_GetMetrics((int*)&Console::Cols, (int*)&Console::Rows);

	Console::SetColors(Color::Red, Color::White);
	
	printf("%*s", Console::Cols, " ");

	char text[Console::Cols];
	char debugflags[3];
#ifdef DEBUG
#ifdef NETDEBUG
	sprintf(debugflags, " ND");
#else /* NETDEBUG */
	sprintf(debugflags, " -D");
#endif /* NETDEBUG */
#else /* DEBUG */
	//strcpy(debugflags, "");
	debugflags[0] = '\0';
#endif /* DEBUG */
	sprintf(text, "DOP-Mii: WiiBrew Edition v%s (SVN r%s)%s", ProgramVersion, SVN_REV_STR, debugflags);
	Console::PrintCenter(Console::Cols, text);

	printf("%*s", Console::Cols, " ");	

	VIDEO_WaitVSync();
    if (VMode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	// Initialize the console, required for printf
	// Offset height so it doesn't overlap with the banner
	CON_InitEx(VMode, 4, 55, VMode->viWidth - 4, VMode->viHeight - 55);
	CON_GetMetrics((int*)&Console::Cols, (int*)&Console::Rows);

	Console::ResetColors();

	gprintf("Console Metrics: Cols = %u, Rows = %u\n", Console::Cols, Console::Rows);
}
