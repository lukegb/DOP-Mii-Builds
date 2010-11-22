#ifndef _SYSMENUMATRIX_H_
#define _SYSMENUMATRIX_H_

#include <string>
#include <vector>
#include <mxml.h>

using namespace std;

namespace Titles
{
	class SysMenuItem
	{
	public: // Fields
		string Name;
		u16 Revision;
		u32 IosRequired;
		u16 IosRevision;
		u16 RegionId;
	public: // Methods
		SysMenuItem(mxml_node_t *xTitle, mxml_node_t *xRevision);
		~SysMenuItem();		
	};

	typedef vector<SysMenuItem>::iterator SysMenuMatrixIterator;

	class SysMenuMatrix : public vector<SysMenuItem>
	{
	private:
		void Load(int regionId);
	public:
		SysMenuMatrixIterator First();
		SysMenuMatrixIterator Last();

		static SysMenuItem* GetRevisionInfo(u16 revision);

		SysMenuMatrix(int regionId);
		~SysMenuMatrix();
	};	
}

#endif