#ifndef _IOSMATRIX_H_
#define _IOSMATRIX_H_

#include <string>
#include <vector>
#include <mxml.h>
#include "Global.h"

using namespace std;

namespace Titles
{
	class IosRevision
	{
	public: // Fields
		u16 Id;
		u64 TitleId;
		string Note;
		string Description;
		bool IsStub;
		bool NusAvailable;
		bool CanPatchFakeSign;
		bool CanPatchEsIdentify;
		bool CanPatchNandPermissions;
        bool DowngradeBlocked;

		bool ApplyFakeSignPatch;
		bool ApplyEsIdentifyPatch;
		bool ApplyNandPermissionsPatch;
        bool IgnoreAllPatches;
	public: // Methods
		void ResetApplyPatchFlags();
		IosRevision(u64 titleId, mxml_node_t *node);
		~IosRevision();
	} ATTRIBUTE_ALIGN(32);	

	typedef vector<IosRevision>::iterator IosRevisionIterator;

	class IosRevisionList : public vector<IosRevision>
	{
	private:
	public:
		IosRevisionIterator Item(u16 id);
		IosRevisionIterator First();
		IosRevisionIterator Last();
	};	

	class IosItem
	{	
	public: // Fields
		u64 TitleId;
		u32 Id;
		string Name;
		string Description;
		IosRevisionList Revisions;		
	public: // Methods
		~IosItem();
	} ATTRIBUTE_ALIGN(32);

	typedef vector<IosItem>::iterator IosMatrixIterator;	
	typedef vector<IosItem>::reference IosMatrixReference;
	typedef vector<IosItem> IosItemList;

	class IosMatrix : public IosItemList
	{
	private: // Methods
		void Load();
		static bool SortList(const IosItem &left, const IosItem &right);
	public: // Methods
		static IosRevision* GetIosRevision(u32 iosId, u16 revision);		
		IosRevisionList GetStubbedIosRevisions();
		IosMatrixIterator First();
		IosMatrixIterator Last();
		IosMatrixIterator Item(u32 id);
		IosMatrixIterator Item(u64 titleId);

		u32List GetList();
		u32 LastIndex();				
 
		IosMatrix();
		~IosMatrix();
	};
}

#endif