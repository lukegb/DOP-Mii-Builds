#ifndef _CHANNELMATRIX_H_
#define _CHANNELMATRIX_H_

#include <gctypes.h>
#include <string>
#include <vector>
#include <mxml.h>

using namespace std;

namespace Titles
{
	class Channel
	{		
	public: // Fields
		string Name;
		u64 TitleId;
		u64 SubTitleId;
		string Region;
	public: // Methods
		Channel(u32 regionId, mxml_node_t *xTitle);
		~Channel();
	};

	typedef vector<Channel>::iterator ChannelIterator;

	class ChannelMatrix : public vector<Channel>
	{
	public: // Methods

		ChannelIterator First();
		ChannelIterator Last();
		ChannelIterator DefaultChannel();
		ChannelMatrix(u32 regionId);
	};
};

#endif