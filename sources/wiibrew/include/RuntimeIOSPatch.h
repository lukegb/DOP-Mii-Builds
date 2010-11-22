// Copyright 2010 Joseph Jordan <joe.ftpii@psychlaw.com.au>
// This code is licensed to you under the terms of the GNU GPL, version 2;
// see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
#ifndef _IOSPATCH_H
#define _IOSPATCH_H

#include <gccore.h>
class RuntimeIOSPatch
{
  public:
    static u32 Apply();
  private:
    static void disable_memory_protection();
    static u32 apply_patch(const char *name, const u8 *old, u32 old_size, const u8 *patch, u32 patch_size, u32 patch_offset);
    static u32 PrintResult(u32 successful);
    static void ApplyingPatch(const char* which);
    static const u8 di_readlimit_old[];
    static const u8 di_readlimit_patch[];
    static const u8 isfs_permissions_old[];
    static const u8 isfs_permissions_patch[];
    static const u8 setuid_old[];
    static const u8 setuid_patch[];
    static const u8 es_identify_old[];
    static const u8 es_identify_patch[];
    static const u8 hash_old[];
    static const u8 hash_patch[];
    static const u8 new_hash_old[];
};
#endif /* _IOSPATCH_H */