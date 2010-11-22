#ifndef _MAIN_H_
#define _MAIN_H_

#include "SysMenuMatrix.h"
#include "IOSMatrix.h"
#include "ChannelMatrix.h"

using namespace Titles;

#define REGIONS_LEN		(int)(sizeof(Regions) / sizeof(Region))
#define REGIONS_LO		(int)0
#define REGIONS_HI		(int)REGIONS_LEN - 1

typedef struct Region
{
	u32 id;
	const char Name[30];
	const char Char;
} ATTRIBUTE_PACKED Region;

const struct Region Regions[] = 
{
	{0, "North America (U)", 'U'},
	{1, "Europe (E)", 'E'},
	{2, "Japan (J)", 'J'},
	{3, "Korea (K)", 'K'}
};

using namespace Titles;

class Main
{
private:
	int SelectedRegion;		
	SysMenuMatrixIterator SelectedSysMenu;
	IosMatrixIterator CurrentIOS;
	u32List SysCheckTable;

	Titles::IosMatrix *IosMatrix;
private:
	/*void ShowIosMenu();*/
	int  InstallIOS(IosMatrixIterator ios, IosRevisionIterator revision);	
	int  InstallIOS(IosMatrixIterator ios, IosRevisionIterator revision, bool altSlot);	
	int  UninstallIOS(IosMatrixIterator ios);
	int  InstallFakeSignIOS36();
	void RefreshIosMatrix();

	void ShowChannelsMenu();
	int  InstallChannel(ChannelIterator channel);
	int  UninstallChannel(ChannelIterator channel);
	ChannelMatrix* InitChannelMatrix();
	
	void ShowSysMenusMenu();
	SysMenuMatrix* InitSysMenuMatrix();

	void BuildSysCheckTable();
	void RunSysCheck();

	void ShowBoot2Menu();
	int  UpgradeBoot2();
	
	bool ShowAHBPROTMenu();
	void ShowInitialMenu();
	void ShowMainMenu();	
	void ShowWelcomeScreen();		
public:
	void Run();
	void ShowIosMenu();

	Main();
	~Main();
};


#endif