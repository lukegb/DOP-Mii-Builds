#include <stdio.h>
#include <stdarg.h>
#include <cstdlib>
#include <gccore.h>
#include <algorithm>
#include <unistd.h>

#include "IOSMatrix.h"
#include "Titles_xml.h"
#include "Global.h"
#include "Tools.h"
#include "System.h"

using namespace std;
using namespace Titles;

/* IosRevision */

IosRevision::IosRevision(u64 titleId, mxml_node_t *node)
{
	TitleId = titleId;
	Id = strtoul(mxmlElementGetAttr(node, "id"), NULL, 0);

	const char* description = mxmlElementGetAttr(node, "description");
	if (description && strlen(description) > 0) this->Description = description;

	const char* xIsStub = mxmlElementGetAttr(node, "isStub");
	this->IsStub = (xIsStub && !strcmpi(xIsStub, "true"));

	const char* xCanPatchFakeSign = mxmlElementGetAttr(node, "canPatchFakeSign");
	this->CanPatchFakeSign = (xCanPatchFakeSign && !strcmpi(xCanPatchFakeSign, "true"));

	const char* xCanPatchEsIdentify = mxmlElementGetAttr(node, "canPatchEsIdentify");
	this->CanPatchEsIdentify = (xCanPatchEsIdentify && !strcmpi(xCanPatchEsIdentify, "true"));

	const char* xCanPatchNandPermissions = mxmlElementGetAttr(node, "canPatchNandPermissions");
	this->CanPatchNandPermissions = (xCanPatchNandPermissions && !strcmpi(xCanPatchNandPermissions, "true")); 

    const char* xDowngradeBlocked = mxmlElementGetAttr(node, "downgradeBlocked");
    this->DowngradeBlocked = (xDowngradeBlocked && !strcmpi(xDowngradeBlocked, "true"));

	const char* xNusAvailable = mxmlElementGetAttr(node, "nus");
	this->NusAvailable = (!xNusAvailable || !strcmpi(xNusAvailable, "true"));

	mxml_node_t *xNote = mxmlFindElement(node, node, "note", NULL, NULL, MXML_DESCEND_FIRST);
	if (xNote)
	{
		const char* note = mxmlElementGetAttr(xNote, "value");
		if (note && strlen(note) > 0) this->Note = note;
		mxmlDelete(xNote);
	}

	this->ResetApplyPatchFlags();
}

IosRevision::~IosRevision()
{
	Note.clear();
	Description.clear();
}

void IosRevision::ResetApplyPatchFlags()
{
	this->ApplyFakeSignPatch = false;
	this->ApplyEsIdentifyPatch = false;
	this->ApplyNandPermissionsPatch = false;
    this->IgnoreAllPatches = false;
}

/* IosRevisionList */

IosRevisionIterator IosRevisionList::Item(u16 id)
{
	for (IosRevisionIterator item = this->begin(); item < this->end(); ++item)
	{
		if (item->Id == id) return item;
	}

	return (IosRevisionIterator)NULL;
}

IosRevisionIterator IosRevisionList::First()
{
	return this->begin();
}

IosRevisionIterator IosRevisionList::Last()
{
	if (!this->size()) return (IosRevisionIterator)NULL;
	return this->end()-1;
}

/* IosItem */
IosItem::~IosItem()
{
	Name.clear();
	Description.clear();
	Revisions.clear();
}

/* IosMatrix */

IosRevision* IosMatrix::GetIosRevision(u32 iosId, u16 revision)
{
	IosRevision *result = NULL;
	mxml_node_t *xTree = NULL;
	mxml_node_t *xTop = NULL;
	mxml_node_t *xTitle = NULL;
	mxml_node_t *xRevision = NULL;

	char titleId[16];
	sprintf(titleId, "%08X%08X", 1, iosId);

	char revisionStr[10];
	sprintf(revisionStr, "%u", revision);
	
	xTree = mxmlLoadString(NULL, (char*)Titles_xml, MXML_NO_CALLBACK);
	if (!xTree) goto end;

	xTop = mxmlFindElement(xTree, xTree, "titles", NULL, NULL, MXML_DESCEND_FIRST);
	if (!xTop) goto end;

	xTitle = mxmlFindElement(xTop, xTop, "title", "id", titleId, MXML_DESCEND_FIRST);
	if (!xTitle) goto end;

	xRevision = mxmlFindElement(xTitle, xTitle, "revision", "id", revisionStr, MXML_DESCEND_FIRST);
	if (!xRevision) goto end;

	result = new IosRevision(TITLEID(1, iosId), xRevision);	
	
end:
	mxmlDelete(xRevision);
	mxmlDelete(xTitle); 
	mxmlDelete(xTop); 
	mxmlDelete(xTree);
	return result;
}

IosMatrix::IosMatrix()
{
	Load();
}

IosMatrix::~IosMatrix()
{
	this->clear();
}

bool IosMatrix::SortList(const IosItem &left, const IosItem &right)
{		
	if ((left.Name == "BC") & (right.Id < 256)) return true;
	if ((left.Id < 256) & (right.Id > 255)) return false;
	if ((left.Name == "MIOS") & (right.Id < 256)) return true;
	if ((left.Id < 256) & (right.Id > 255)) return false;
	if ((left.Name == "BC") & (right.Name == "MIOS")) return true;
	return (left.Id < right.Id);
}

void IosMatrix::Load()
{
	mxml_node_t *xTree = NULL;
	mxml_node_t *xTop = NULL;
	mxml_node_t *xTitle = NULL;
	mxml_node_t *xRevision = NULL;
	IosItem *item;	
	u32List iosList;

	xTree = mxmlLoadString(NULL, (char*)Titles_xml, MXML_NO_CALLBACK);
	if (!xTree) goto final;

	xTop = mxmlFindElement(xTree, xTree, "titles", NULL, NULL, MXML_DESCEND_FIRST);
	if (!xTop) goto final;

	for 
	(
		xTitle = mxmlFindElement(xTop, xTop, "title", "type", "IOS", MXML_DESCEND_FIRST); xTitle != NULL;
		xTitle = mxmlFindElement(xTitle, xTop, "title", "type", "IOS", MXML_NO_DESCEND)
	)
	{
		// Load Details
		item = new IosItem();
		item->TitleId = strtoull(mxmlElementGetAttr(xTitle, "id"), NULL, 16);			
		item->Id = TITLEID2(item->TitleId);

		const char* description = mxmlElementGetAttr(xTitle, "description");
		if (description) item->Description = description;

		const char* name = mxmlElementGetAttr(xTitle, "name");
		if (name) item->Name = name;

		// Load Revisions
		for 
		(
			xRevision = mxmlFindElement(xTitle, xTitle, "revision", NULL, NULL, MXML_DESCEND_FIRST); xRevision != NULL;
			xRevision = mxmlFindElement(xRevision, xTitle, "revision", NULL, NULL, MXML_NO_DESCEND)
		)
		{
			IosRevision *revision = new IosRevision(item->TitleId, xRevision);
			item->Revisions.push_back(*revision);
		}
		
		this->push_back(*item);
	}

	if (System::GetInstalledIosIdList(iosList) < 0) goto final;

	for (u32Iterator ios = iosList.begin(); ios < iosList.end(); ++ios)
	{
		if (this->Item(*ios) == (IosMatrixIterator)NULL) 
		{
			item = new IosItem();
			item->Id = *ios;
			item->TitleId = TITLEID(1, *ios);
			item->Description = "Unknown IOS. Discovered during IOS Scanning.";
			char name[10];
			sprintf(name, "IOS%u", *ios);
			item->Name = name;			
			memset(name, 0, 10);			
			this->push_back(*item);
		}
	}

	sort(this->begin(), this->end(), SortList);

final:

	iosList.clear();
	mxmlDelete(xRevision);
	mxmlDelete(xTitle);
	mxmlDelete(xTop);
	mxmlDelete(xTree);
}

u32 IosMatrix::LastIndex()
{
	return this->size()-1;
}

IosMatrixIterator IosMatrix::Item(u32 id)
{
	for (IosMatrixIterator item = this->begin(); item < this->end(); ++item)
	{
		if (item->Id == id) return item;
	}

	return (IosMatrixIterator)NULL;
}

IosMatrixIterator IosMatrix::Item(u64 titleId)
{
	for (IosMatrixIterator item = this->begin(); item < this->end(); ++item)
	{
		if (item->TitleId == titleId) return item;
	}

	return (IosMatrixIterator)NULL;
}

IosRevisionList IosMatrix::GetStubbedIosRevisions()
{
	IosRevisionList result;
	for (IosMatrixIterator item = this->begin(); item < this->end(); ++item)
	{
		for (IosRevisionIterator rev = item->Revisions.begin(); rev < item->Revisions.end(); ++rev)
		{
			if (rev->IsStub) result.push_back(*rev);
		}
	}
	return result;
}

u32List IosMatrix::GetList()
{
	u32List result;

	for (IosMatrixIterator item = this->begin(); item < this->end(); ++item)
	{
		result.push_back(item->Id);
	}

	return result;
}

IosMatrixIterator IosMatrix::Last()
{
	return this->end()-1;
}

IosMatrixIterator IosMatrix::First()
{
	return this->begin();
}