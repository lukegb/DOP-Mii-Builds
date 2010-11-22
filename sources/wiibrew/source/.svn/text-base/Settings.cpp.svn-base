#include <stdio.h>
#include <gccore.h>
#include <mxml.h>

#include "Settings.h"
#include "FileSystem.h"
#include "Gecko.h"
#include "Global.h"

using namespace IO;

Settings Settings::_instance;

Settings& Settings::Instance()
{
	return _instance;
}

Settings::Settings()
{
	this->FlashWiiLeds = true;
	this->FlashWiiMoteLeds = true;
	this->DefaultIOS = (u32)DEFAULT_IOS;
	this->NUS.CacheFolder = "sd:/nus/{TITLEID1}/{TITLEID2}/v{VERSION}";
}

Settings::~Settings()
{
	this->NUS.AlternateUrl.clear();
	this->NUS.CacheFolder.clear();
}

void Settings::Load()
{
	mxml_node_t *xTop = NULL;
	mxml_node_t *xTree = NULL;
	mxml_node_t *xNus = NULL;
	mxml_node_t *xElement = NULL;
	FILE *file = NULL;
	char *value;

	if (!SD::Mount()) return;	
	
	file = File::OpenBinary("sd:/config/DOP-Mii.cfg");
	if (!file) goto final;

	xTop = mxmlLoadFile(NULL, file, MXML_NO_CALLBACK);
	if (!xTop) goto final;

	xTree = mxmlFindElement(xTop, xTop, "settings", NULL, NULL, MXML_DESCEND_FIRST);
	if (!xTree) goto final;

	xElement = mxmlFindElement(xTree, xTree, "defaultIOS", NULL, NULL, MXML_DESCEND_FIRST);
	if (!xElement) this->DefaultIOS = (u32)DEFAULT_IOS;
	else this->DefaultIOS = strtoul(xElement->child->value.text.string, NULL, 10);

	xNus = mxmlFindElement(xTree, xTree, "nus", NULL, NULL, MXML_DESCEND_FIRST);
	if (xNus)
	{
		// NUS AlternateURL
		xElement = mxmlFindElement(xNus, xNus, "alternateUrl", NULL, NULL, MXML_DESCEND_FIRST);
		if (xElement) 
		{
			NUS.AlternateUrl = xElement->child->value.text.string;
			mxmlDelete(xElement); xElement = NULL;
		}		

		// NUS Cache Folder
		xElement = mxmlFindElement(xNus, xNus, "cacheFolder", NULL, NULL, MXML_DESCEND_FIRST);
		if (xElement) 
		{
			NUS.CacheFolder = xElement->child->value.text.string;
			mxmlDelete(xElement); xElement = NULL;
		}
	}

	xElement = mxmlFindElement(xTree, xTree, "wiiMoteLeds", NULL, NULL, MXML_DESCEND_FIRST);
	if (xElement) 
	{
		value = xElement->child->value.text.string;
		if (!strcmpi(value, "off") || !strcmpi(value, "false")) this->FlashWiiMoteLeds = false;
	}

	xElement = mxmlFindElement(xTree, xTree, "wiiLeds", NULL, NULL, MXML_DESCEND_FIRST);
	if (xElement)
	{
		value = xElement->child->value.text.string;
		if (!strcmpi(value, "off") || !strcmpi(value, "false")) this->FlashWiiLeds = false;
	}

final:
	mxmlDelete(xNus);
	mxmlDelete(xTree);
	mxmlDelete(xTop);
	if (file) fclose(file);
}
