#include <mxml.h>
#include <gccore.h>
#include <string>
#include <sstream>

#include "Global.h"
#include "Gecko.h"
#include "Video.h"
#include "Scripting.h"
#include "FileSystem.h"
#include "Title.h"
#include "SysMenuMatrix.h"
#include "ChannelMatrix.h"
#include "SysTitle.h"
#include "sysconf.h"
#include "Tools.h"
#include "Identify.h"
#include "Main.h"

using namespace IO;
using namespace Titles;

template <class T>
const char *to_string(T t, std::ios_base & (*f)(std::ios_base&))
{
  std::ostringstream oss;
  oss << f << t;
  return oss.str().c_str();
}

u8 Scripting::RunScript(const char* filename)
{
	Console::ClearScreen();
	
	gcprintf("Checking for file: %s\n", filename);
	if (!File::Exists(filename))
	{
		gcprintf(">>> File does not exist.\n\n");
		Console::PromptContinue();
		return ENOENT;
	}
	
	gcprintf("Loading file...\n");
	// So it exists? Load into memory...
	FILE *fp;
	mxml_node_t *tree;
	mxml_node_t *top;
	mxml_node_t *node;
	
	fp = fopen(filename, "r");
	tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
	//fclose(fp);
	
	gcprintf("Load complete.\n");
	// Okay, so it's all loaded into sctx.
	// Now, let's see where our default load location is...
	char defaultload[10] = "nus";
	
	top = mxmlFindElement(tree, tree, "dopscript", NULL, NULL, MXML_DESCEND_FIRST);
	if (top == NULL)
	{
		gcprintf(">>> Script invalid!\n\n");
		Console::PromptContinue();
		return E_SC_INVALIDSCRIPT;
	}
	
	if (mxmlElementGetAttr(top, "from") != NULL)
	{
		strncpy(defaultload, mxmlElementGetAttr(top, "from"), 8);
	}
	
	gcprintf("Default Loading from: %s\n", defaultload);
	
	/*while (node = mxmlWalkNext(node, tree, MXML_DESCEND)) {*/
	/*node = node->child;
	while ((node = node->next)) {*/
	char *failtext;
	node = top->child;
	while ((node = mxmlFindElement(node, tree, NULL, NULL, NULL, MXML_DESCEND))) {
		failtext = node->value.element.name;
		TitleType mytitletype;
		mytitletype = TitleType::UNKNOWN;
		if (strcmp(failtext,"title") == 0)
		{
			if (mxmlElementGetAttr(node, "id") == NULL) // skip
			{
				printf("It's a title... but I skipped it.\n\n");
				continue;
			}
			// crazeh pointers :D
			char *pointex;
			char tmpstr[20];
			char tmpstr2[20];
			char tmpstr3[9];
			strcpy(tmpstr, "0x");
			strcpy(tmpstr2, mxmlElementGetAttr(node,"id"));
			strncpy(tmpstr3, tmpstr2, 8);
			tmpstr3[8] = '\0';
			pointex = tmpstr2 + 8;
			strcat(tmpstr, tmpstr3);
			strcat(tmpstr, " 0x");
			strcat(tmpstr, pointex);
			
			long unsigned int titleid1 = strtoul(tmpstr, &pointex, 16);
			long unsigned int titleid2 = strtoul(pointex, NULL, 16);
			pointex = NULL;
			Title myTitle(titleid1, titleid2);
			mytitletype = myTitle.Type;
			memset(tmpstr, 0, 20);
			memset(tmpstr2, 0, 20);
			memset(tmpstr3, 0, 9);
			mxmlElementSetAttr(node, "titleid1", to_string<long>(titleid1, std::dec));
			mxmlElementSetAttr(node, "titleid2", to_string<long>(titleid2, std::dec));
			
			if (mytitletype == TitleType::IOS) {
				printf("IOS ver %lu?\n", titleid2);
				sprintf(tmpstr, "%lu", titleid2);
				mxmlElementSetAttr(node, "version", tmpstr);
			}
			myTitle = NULL;
		} else if (strcmp(failtext,"systemmenu") == 0) {
			mytitletype = TitleType::SYSMENU;
		} else if (strcmp(failtext,"ios") == 0) {
			mytitletype = TitleType::IOS;
		} else if (strcmp(failtext,"channel") == 0) {
			mytitletype = TitleType::CHANNEL;
		}
		bool savePriiloader;
		int region = -1;
		SysMenuMatrix *sysMM;
		SysMenuMatrixIterator sysMMI;
		SysMenuItem *sysMI;
		ChannelMatrix *chMM;
		ChannelIterator chMMI;
		Channel *chMI;
		char lookingFor[50];
		char tmpstr[20];
		Title *requiredTitle;
		u64 iosTitleId;
		u16 iosRevision;
		int getMethod;
		int i = 0;
		if (strcmp((mxmlElementGetAttr(node, "from") == NULL ? defaultload : mxmlElementGetAttr(node, "from")),"sd")==0) {
			getMethod = GETWADSD;
		} else if (strcmp((mxmlElementGetAttr(node, "from") == NULL ? defaultload : mxmlElementGetAttr(node, "from")),"usb")==0) {
			getMethod = GETWADUSB;
		} else {
			getMethod = GETNUS;
		}
		switch (mytitletype)
		{
			case TitleType::SYSMENU:
				// installIos, version (required), region
				if (mxmlElementGetAttr(node, "version") == NULL) {
					printf(">> ERROR: There is no System Menu version set. Skipping...\n");
					continue;
				}
				strncpy(tmpstr, mxmlElementGetAttr(node, "version"), 3);
				tmpstr[3] = '\0';
				
				strcpy(lookingFor, "System Menu ");
				strcat(lookingFor, tmpstr);
				if (mxmlElementGetAttr(node, "region") == NULL ||
					(strcasecmp(mxmlElementGetAttr(node, "region"),"E") != 0 &&
					strcasecmp(mxmlElementGetAttr(node, "region"),"U") != 0 &&
					strcasecmp(mxmlElementGetAttr(node, "region"),"J") != 0 &&
					strcasecmp(mxmlElementGetAttr(node, "region"),"K") != 0)
				) {
					printf("Installing your Wii region's version...\n");
					region = CONF_GetRegion();
				} else {
					if (strcasecmp(mxmlElementGetAttr(node, "region"),"E") == 0) {
						region = CONF_REGION_EU;
						printf("Installing EU System Menu...\n");
					} else if (strcasecmp(mxmlElementGetAttr(node, "region"),"U") == 0) {
						region = CONF_REGION_US;
						printf("Installing US System Menu...\n");
					} else if (strcasecmp(mxmlElementGetAttr(node, "region"),"J") == 0) {
						region = CONF_REGION_JP;
						printf("Installing Japanese System Menu...\n");
					} else if (strcasecmp(mxmlElementGetAttr(node, "region"),"K") == 0) {
						region = CONF_REGION_KR;
						printf("Installing Korean System Menu...\n");
					}
				}
				sysMM = new SysMenuMatrix(region);
				sysMI = NULL;
				for (sysMMI = sysMM->begin(); sysMMI != sysMM->end(); ++sysMMI)
				{
					if (strcmp((*sysMMI).Name.c_str(),lookingFor) == 0) {
						sysMI = new SysMenuItem(*sysMMI);
						break;
					}
				}
				if (sysMI == NULL)
				{
					printf(">> ERROR: Couldn't find the correct System Menu\n");
					continue;
				}
				if (CONF_GetRegion() != sysMI->RegionId) {
					// doublecheck they aren't on Korean wii and they ain't trying to update >4.1 to start with
					if (CONF_GetRegion() == CONF_REGION_KR && sysMI->Revision > 454) {
						// Hookay, 
						printf(">> ERROR: Will not update Wii due to possible 003 brick (korean)\n");
						continue;
					}
					// change the wii's region :D
					if (Identify::AsSystemMenu() < 0)
						Identify::AsSuperUser();
					if (SYSCONF_Init() < 0) {
						printf(">> ERROR: Couldn't initilise SYSCONF\n");
						continue;
					}
					printf("Changing Wii's region and area settings...\n");
					SYSCONF_SetRegion(sysMI->RegionId);
					SYSCONF_SetArea(sysMI->RegionId);
					if (SYSCONF_SaveChanges() < 0) {
						printf(">> ERROR: Couldn't save SYSCONF changes: %s\n",NandError::ToString(SYSCONF_SaveChanges())); // may as well try again, right?
						continue;
					}
				}
				
				savePriiloader = (mxmlElementGetAttr(node, "savePriiloader") == NULL ? false : (stricmp(mxmlElementGetAttr(node, "savePriiloader"),"false")!=0) ? true : false);
				gcprintf("Installing System Menu.\n");
				Console::SetFgColor(Color::Yellow, Bold::On);
				gcprintf("!!!DO NOT POWER OFF THE WII!!!\n");
				Console::ResetColors();
				gcprintf("Failure to do so can cause your Wii to be bricked.\n");
				if (Tools::IsPriiloaderInstalled() && savePriiloader) { Tools::BackupPriiloader(); } else { savePriiloader = false; }
				iosTitleId = TITLEID(1, sysMI->IosRequired);
				iosRevision = SysTitle::GetVersion(iosTitleId);
				if (iosRevision != sysMI->IosRevision) {
					printf("You have IOS%uv%u installed. I need IOS%uv%u.\n",sysMI->IosRequired, iosRevision, sysMI->IosRequired, sysMI->IosRevision);
					requiredTitle = new Title(iosTitleId);
					requiredTitle->Get(sysMI->IosRevision, getMethod);
					requiredTitle->PerformInstall();
					delete requiredTitle;
				}
				requiredTitle = new Title(1,2);
				requiredTitle->Get(sysMI->Revision, getMethod);
				requiredTitle->PerformInstall();
				delete requiredTitle;
				break;
			case TitleType::IOS:
				if (mxmlElementGetAttr(node, "version") == NULL) {
					printf(">> ERROR: There's no IOS version specified. Skipping...\n");
					continue;
				}
				//printf("It's an IOS: ATTRIBUTES:\n");
				// patchEsIdentify patchFakesign patchNand version(required) revision
/*				printf("Patch ES_Identify:      %s\n", mxmlElementGetAttr(node, "patchEsIdentify"));
				printf("Patch Fakesign:         %s\n", mxmlElementGetAttr(node, "patchFakesign"));
				printf("Patch NAND perms:       %s\n", mxmlElementGetAttr(node, "patchNand"));*/
/*				printf("IOS version: (eg36/58)  %s\n", mxmlElementGetAttr(node, "version"));
				printf("IOS revision: (vYYYY)   %s\n", mxmlElementGetAttr(node, "revision"));
				printf("Would skip?             %s\n", ((mxmlElementGetAttr(node, "version") == NULL) ? "YES" : "NO"));
				printf("\n");*/
				printf("Installing IOS%s\n", mxmlElementGetAttr(node, "version"));
                                iosTitleId = TITLEID(1, atoi(mxmlElementGetAttr(node, "version")));
                                iosRevision = atoi(mxmlElementGetAttr(node, "revision"));
                                requiredTitle = new Title(iosTitleId);
                                requiredTitle->Get(iosRevision, getMethod);
                                requiredTitle->PerformInstall();
                                delete requiredTitle;
				break;
			case TitleType::CHANNEL:
				if (mxmlElementGetAttr(node, "name") == NULL && mxmlElementGetAttr(node, "id") == NULL) {
					printf(">> ERROR: There's no channel specified. Skipping...\n");
					continue;
				}
				printf("Installing: %s\n", ((mxmlElementGetAttr(node, "name") == NULL) ? (mxmlElementGetAttr(node,"id")) : (mxmlElementGetAttr(node,"name"))));
				if (mxmlElementGetAttr(node, "id") != NULL) { // just repurposing iosTitleId <_<
					iosTitleId = TITLEID(strtoul(mxmlElementGetAttr(node, "titleid1"),NULL,0),strtoul(mxmlElementGetAttr(node, "titleid2"),NULL,0));
					printf("TITLE ID: %lu\n",(long unsigned int)iosTitleId);
				} else {
					if (mxmlElementGetAttr(node, "region") != NULL) {
						// okay, so now look up the region. *sigh*
						for (i = 0; i < REGIONS_LEN; i++) {
							if (Regions[i].Char == mxmlElementGetAttr(node, "region")[0]) {
								region = Regions[i].id;
								break;
							}
						}
					} else {
						region = CONF_GetRegion();
					}
	                                chMM = new ChannelMatrix(region);
	                                chMI = NULL;
	                                for (chMMI = chMM->begin(); chMMI != chMM->end(); ++chMMI)
	                                {
	                                        if (strcmp((*chMMI).Name.c_str(),mxmlElementGetAttr(node,"name")) == 0) {
	                                                chMI = new Channel(*chMMI);
	                                                break;
	                                        }
	                                }
	                                if (chMI == NULL)
	                                {
	                                        printf(">> ERROR: Couldn't find the channel '%s'\n", mxmlElementGetAttr(node,"name"));
	                                        continue;
        	                        }
					iosTitleId = chMI->TitleId;
				}
				if (mxmlElementGetAttr(node, "revision") != NULL) {
					// so read their revision
					iosRevision = atoi(mxmlElementGetAttr(node, "revision"));
				} else {
					iosRevision = 0;
				}
                                requiredTitle = new Title(iosTitleId);
                                requiredTitle->Get(iosRevision, getMethod);
                                requiredTitle->PerformInstall();
                                delete requiredTitle;
				break;
			case TitleType::BOOT2:
				printf("Will not script the installation of boot2!\n");
				break;
			case TitleType::UNKNOWN:
			default:
				printf("Skipping script element: unknown element type.\n");
				break;
		}
	}
	
	Console::WaitForA(false);
	return 0;
}
