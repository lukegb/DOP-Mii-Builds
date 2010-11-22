// Copyright 2010 Joseph Jordan <joe.ftpii@psychlaw.com.au>
// This code is licensed to you under the terms of the GNU GPL, version 2;
// see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
#include <gccore.h>
#include <ogc/machine/processor.h>
#include <string.h>
#include <stdio.h>

#include "RuntimeIOSPatch.h"
#include "Gecko.h"
#include "Video.h"

#define HAVE_AHBPROT ((*(vu32*)0xcd800064 == 0xFFFFFFFF) ? 1 : 0)
#define MEM_REG_BASE 0xd8b4000
#define MEM_PROT (MEM_REG_BASE + 0x20a)

void RuntimeIOSPatch::disable_memory_protection() {
    write32(MEM_PROT, read32(MEM_PROT) & 0x0000FFFF);
}

u32 RuntimeIOSPatch::apply_patch(const char *name, const u8 *old, u32 old_size, const u8 *patch, u32 patch_size, u32 patch_offset) {
    ApplyingPatch(name);
    u8 *ptr = (u8 *)0x93400000;
    u32 found = 0;
    u8 *location = NULL;
    while ((u32)ptr < (0x94000000 - patch_size)) {
        if (!memcmp(ptr, old, old_size)) {
            found++;
            location = ptr + patch_offset;
            u8 *start = location;
            u32 i;
            for (i = 0; i < patch_size; i++) {
                *location++ = patch[i];
            }
            DCFlushRange((u8 *)(((u32)start) >> 5 << 5), (patch_size >> 5 << 5) + 64);
        }
        ptr++;
    }
    return found;
}

const u8 RuntimeIOSPatch::di_readlimit_old[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x0A, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
    0x7E, 0xD4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08
};
const u8 RuntimeIOSPatch::di_readlimit_patch[] = { 0x7e, 0xd4 };

const u8 RuntimeIOSPatch::isfs_permissions_old[] = { 0x42, 0x8B, 0xD0, 0x01, 0x25, 0x66 };
const u8 RuntimeIOSPatch::isfs_permissions_patch[] = { 0x42, 0x8B, 0xE0, 0x01, 0x25, 0x66 };
const u8 RuntimeIOSPatch::setuid_old[] = { 0xD1, 0x2A, 0x1C, 0x39 };
const u8 RuntimeIOSPatch::setuid_patch[] = { 0x46, 0xC0 };
const u8 RuntimeIOSPatch::es_identify_old[] = { 0x28, 0x03, 0xD1, 0x23 };
const u8 RuntimeIOSPatch::es_identify_patch[] = { 0x00, 0x00 };
const u8 RuntimeIOSPatch::hash_old[] = { 0x20, 0x07, 0x23, 0xA2 };
const u8 RuntimeIOSPatch::hash_patch[] = { 0x00 };
const u8 RuntimeIOSPatch::new_hash_old[] = { 0x20, 0x07, 0x4B, 0x0B };

u32 RuntimeIOSPatch::Apply() {
    printf("\n\n");
    u32 count = 0;
    if (HAVE_AHBPROT) {
        disable_memory_protection();
        count += PrintResult(apply_patch("DI ReadLimit", di_readlimit_old, sizeof(di_readlimit_old), di_readlimit_patch, sizeof(di_readlimit_patch), 12));
        count += PrintResult(apply_patch("NAND Permissions", isfs_permissions_old, sizeof(isfs_permissions_old), isfs_permissions_patch, sizeof(isfs_permissions_patch), 0));
        count += PrintResult(apply_patch("ES_SetUID", setuid_old, sizeof(setuid_old), setuid_patch, sizeof(setuid_patch), 0));
        count += PrintResult(apply_patch("ES_Identify", es_identify_old, sizeof(es_identify_old), es_identify_patch, sizeof(es_identify_patch), 2));
        count += PrintResult(apply_patch("Trucha (may fail)", hash_old, sizeof(hash_old), hash_patch, sizeof(hash_patch), 1));
        count += PrintResult(apply_patch("New Trucha (may fail)", new_hash_old, sizeof(new_hash_old), hash_patch, sizeof(hash_patch), 1));
    }
    return count;
}

void RuntimeIOSPatch::ApplyingPatch(const char* which) {
    	printf("Applying patch: ");
	Console::SetFgColor(Color::White, Bold::On);
	printf(which);
	Console::ResetColors();
	Console::SetColPosition(Console::Cols - 11);
	printf("[        ]");
	Console::SetColPosition(Console::Cols - 9);
}

u32 RuntimeIOSPatch::PrintResult(u32 successful) {
	if (successful) {
		Console::SetFgColor(Color::Green, Bold::On);
		printf(" DONE ");
		Console::ResetColors();
	} else {
		Console::SetFgColor(Color::Red, Bold::On);
		printf("FAILED");
		Console::ResetColors();
	}
	printf("\n");
	return successful;
}
