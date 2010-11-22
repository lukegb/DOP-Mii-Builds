#ifndef _WAD_H_
#define _WAD_H_

#include "Title.h"

/* 'WAD Header' structure */
typedef struct WadHeader
{
    /* Header length */
    u32 HeaderSize;

    /* WAD type */
    u16 Type;

    u16 Padding;

    /* Data length */
    u32 CertsSize;
    u32 CrlSize;
    u32 TicketSize;
    u32 TmdSize;
    u32 DataSize;
    u32 FooterSize;
} ATTRIBUTE_PACKED WadHeader;

#endif