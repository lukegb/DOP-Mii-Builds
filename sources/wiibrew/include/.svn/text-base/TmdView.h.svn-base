#ifndef _TMDVIEW_H_
#define _TMDVIEW_H_

struct TmdViewContent
{
	u32 Id;
	u32 Index;
	u16 Type;
	u64 Size;
} ATTRIBUTE_PACKED;
 
struct TmdView
{
	u8  Version; // 0x0000;
	u8  Filler[3];
	u64 IosTitleId; //0x0004
	u64 TitleId; // 0x00c
	u32 TitleType; //0x0014
	u16 GroupId; //0x0018
	u8  Reserved[0x3e]; //0x001a this is the same reserved 0x3e bytes from the tmd
	u16 TitleVersion; //0x0058
	u16 NumberContents; //0x005a
	TmdViewContent Contents[]; //0x005c
} ATTRIBUTE_PACKED;

#endif
