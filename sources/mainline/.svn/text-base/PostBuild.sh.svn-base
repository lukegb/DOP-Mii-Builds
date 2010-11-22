#!/bin/bash

BASE="`basename $PWD`"
ELF="$BASE.elf"
DOL="$BASE.dol"

$DEVKITPPC/bin/powerpc-eabi-nm "$ELF" | grep __wpads$ | sed -e 's/^\([^ ]*\) .*$/#define __WPADS_ADDR 0x\1/' > t__wpads.h

if ! `cmp -s t__wpads.h include/__wpads.h`
then
	cp -f t__wpads.h include/__wpads.h	
	make remake
fi

rm -f t__wpads.h
