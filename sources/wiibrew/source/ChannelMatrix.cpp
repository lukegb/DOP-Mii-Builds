#include "ChannelMatrix.h"

#include "Gecko.h"
#include "Global.h"
#include "Titles_xml.h"
#include "Tools.h"

using namespace std;
using namespace Titles;

/* Channel */
Channel::~Channel()
{
	Name.clear();
	Region.clear();
}

Channel::Channel(u32 regionId, mxml_node_t *xTitle) : SubTitleId(0)
{
	this->TitleId = 0;
	this->Region = "ALL";
	char regionNumber[1];
	mxml_node_t* xSubTitle = NULL;

	sprintf(regionNumber, "%u", regionId);

	this->Name = mxmlElementGetAttr(xTitle, "name");

	const char* xTitleId = mxmlElementGetAttr(xTitle, "id");
	/* 
	Check and see if this is a regional channel
	If Not then we will assign TitleID at the sub Channel Level
	*/
	if (xTitleId && strlen(xTitleId) > 0) this->TitleId = strtoull(xTitleId, NULL, 16);

	/* Check For Sub Title */
	xSubTitle = mxmlFindElement(xTitle, xTitle, "subtitle", "regionId", regionNumber, MXML_DESCEND_FIRST);
	if (xSubTitle)
	{
		this->SubTitleId = strtoull(mxmlElementGetAttr(xSubTitle, "id"), NULL, 16);
		this->Region = mxmlElementGetAttr(xSubTitle, "region");

		if (this->TitleId == 0) this->TitleId = this->SubTitleId;
		if (this->TitleId == this->SubTitleId) this->SubTitleId = 0;
		mxmlDelete(xSubTitle);
	}
	
	memset(regionNumber, 0, 1);
}

/* ChannelMatrix */
ChannelMatrix::ChannelMatrix(u32 regionId)
{
	mxml_node_t* xTree = NULL;
	mxml_node_t* xTop = NULL;
	mxml_node_t* xTitle = NULL;

	xTree = mxmlLoadString(NULL, (char*)Titles_xml, MXML_NO_CALLBACK);
	if (!xTree) goto end;

	xTop = mxmlFindElement(xTree, xTree, "titles", NULL, NULL, MXML_DESCEND_FIRST);
	if (!xTop) goto end;
	

	for  
	( 
		xTitle = mxmlFindElement(xTop, xTop, "title", "type", "CHANNEL", MXML_DESCEND_FIRST); xTitle != NULL;
		xTitle = mxmlFindElement(xTitle, xTop, "title", "type", "CHANNEL", MXML_NO_DESCEND)
	)	
	{		
		Channel *channel = new Channel(regionId, xTitle);
		this->push_back(*channel);
	}

end:
	mxmlDelete(xTitle);
	mxmlDelete(xTop);
	mxmlDelete(xTree);
}

ChannelIterator ChannelMatrix::Last()
{
	return this->end()-1;
}

ChannelIterator ChannelMatrix::First()
{
	return this->begin();
}

ChannelIterator ChannelMatrix::DefaultChannel()
{
	for (ChannelIterator channel = this->begin(); channel < this->end(); ++channel)
	{
		if (channel->TitleId == 0x1000248414241ULL) return channel;
	}

	return (ChannelIterator)NULL;
}
