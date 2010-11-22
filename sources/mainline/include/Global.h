#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <string>
#include <vector>
#include "gctypes.h"
#include "Gecko.h"
#include "Error.h"
#include "Identify.h"

using namespace std;

/* Constants */
#define ProgramVersion "14.1"
#define DEFAULT_IOS 36
#define IOS36Version 3351

#define STRINGIFY(name) # name

#define ROUND_UP(x,n) (-(-(x) & -(n)))
#define ROUND_TO32(x) (-(-(x) & -(32)))

// Turn upper and lower into a full title ID
#define TITLEID(titleId1,titleId2)		(((u64)(titleId1) << 32) | (titleId2))
// Get upper or lower half of a title ID
#define TITLEID1(titleId)		((u32)((titleId) >> 32))
// Turn upper and lower into a full title ID
#define TITLEID2(titleId)		((u32)(titleId))

#define ES_ERROR_1028 -1028 // No ticket installed 
#define ES_ERROR_1035 -1035 // Title with a higher version is already installed 
#define ES_ERROR_2011 -2011 // Signature check failed (Needs Fakesign)

typedef vector<std::string> StringList;
typedef vector<std::string>::iterator StringIterator;
typedef vector<u8> u8List;
typedef vector<u8>::iterator u8Iterator;
typedef vector<u32> u32List;
typedef vector<u32>::iterator u32Iterator;
typedef vector<u64> u64List;
typedef vector<u64>::iterator u64Iterator;

#endif
