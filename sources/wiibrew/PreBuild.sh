#!/bin/bash

if [ ! -f include/__wpads.h ] 
then
	echo "#define __WPADS_ADDR 0x000000000" > include/__wpads.h
fi
