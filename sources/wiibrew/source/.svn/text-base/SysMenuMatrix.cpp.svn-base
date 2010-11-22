#include <stdio.h>
#include <cstdlib>

#include <gccore.h>

#include "Titles_xml.h"
#include "SysMenuMatrix.h"
#include "Gecko.h"

using namespace Titles;

SysMenuItem::SysMenuItem(mxml_node_t *xTitle, mxml_node_t *xRevision)
{
	this->Name = mxmlElementGetAttr(xTitle, "name");
	this->IosRequired = strtoul(mxmlElementGetAttr(xTitle, "iosRequired"), NULL, 0);
	this->IosRevision = strtoul(mxmlElementGetAttr(xTitle, "iosRevision"), NULL, 0);
	this->Revision = strtoul(mxmlElementGetAttr(xRevision, "id"), NULL, 0);
	this->RegionId = strtoul(mxmlElementGetAttr(xRevision, "regionId"), NULL, 0);
}

SysMenuItem::~SysMenuItem()
{
	Name.clear();
}

SysMenuMatrix::SysMenuMatrix(int regionId)
{
	Load(regionId);
}

SysMenuMatrix::~SysMenuMatrix()
{
	this->clear();
}

void SysMenuMatrix::Load(int regionId)
{
	char regionNumber[1];
	sprintf(regionNumber, "%u", regionId);
	mxml_node_t* xTree = NULL;
	mxml_node_t* xTop = NULL;
	mxml_node_t* xTitle = NULL;
	mxml_node_t* xRevision = NULL;

	xTree = mxmlLoadString(NULL, (char*)Titles_xml, MXML_NO_CALLBACK);
	if (!xTree) goto end;

	xTop = mxmlFindElement(xTree, xTree, "titles", NULL, NULL, MXML_DESCEND_FIRST);
	if (!xTop) goto end;
	
	for  
	( 
		xTitle = mxmlFindElement(xTop, xTop, "title", "type", "SYSMENU", MXML_DESCEND_FIRST); xTitle != NULL;
		xTitle = mxmlFindElement(xTitle, xTop, "title", "type", "SYSMENU", MXML_NO_DESCEND)
	)	
	{		
		xRevision = mxmlFindElement(xTitle, xTitle, "revision", "regionId", regionNumber, MXML_DESCEND_FIRST);
		if (xRevision)
		{			
			SysMenuItem *item = new SysMenuItem(xTitle, xRevision);
			this->push_back(*item);
		}
	}

end:
	mxmlDelete(xRevision);
	mxmlDelete(xTitle);
	mxmlDelete(xTop);
	mxmlDelete(xTree);
	memset(regionNumber, 0, 1);
}

SysMenuItem* SysMenuMatrix::GetRevisionInfo(u16 revision)
{
	SysMenuItem *result = NULL;
	mxml_node_t* xTree = NULL;
	mxml_node_t* xTop = NULL;
	mxml_node_t* xTitle = NULL;
	mxml_node_t* xRevision = NULL;

	char revisionStr[10];
	sprintf(revisionStr, "%u", revision);

	xTree = mxmlLoadString(NULL, (char*)Titles_xml, MXML_NO_CALLBACK);
	if (!xTree) goto end;

	xTop = mxmlFindElement(xTree, xTree, "titles", NULL, NULL, MXML_DESCEND_FIRST);
	if (!xTop) goto end;

	for  
	( 
		xTitle = mxmlFindElement(xTop, xTop, "title", "type", "SYSMENU", MXML_DESCEND_FIRST); xTitle != NULL;
		xTitle = mxmlFindElement(xTitle, xTop, "title", "type", "SYSMENU", MXML_NO_DESCEND)
	)	
	{		
		xRevision = mxmlFindElement(xTitle, xTitle, "revision", "id", revisionStr, MXML_DESCEND_FIRST);
		if (xRevision)
		{			
			result = new SysMenuItem(xTitle, xRevision);
			break;
		}
	}	

end:
	mxmlDelete(xRevision);
	mxmlDelete(xTitle);
	mxmlDelete(xTop);
	mxmlDelete(xTree);
	return result;
}

SysMenuMatrixIterator SysMenuMatrix::Last()
{
	return this->end()-1;
}

SysMenuMatrixIterator SysMenuMatrix::First()
{
	return this->begin();
}
