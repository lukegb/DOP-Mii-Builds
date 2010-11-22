#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <string>

using namespace std;

class Settings
{
private:
	static Settings _instance;
private:
	Settings();
	~Settings();
	Settings(const Settings&);
	Settings& operator=(const Settings&);
public:
	class 
	{
	public:		
		string AlternateUrl;
		string CacheFolder;
	} NUS;

	u32 DefaultIOS;
	bool FlashWiiMoteLeds;
	bool FlashWiiLeds;
public: 
	void Load();
	static Settings &Instance();
};

#endif