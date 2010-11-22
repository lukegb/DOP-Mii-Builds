#!/bin/bash

if [ ! -z "$1" ];
then
	if [ ! -s include/BuildType.h ];
	then
		echo > include/BuildType.h
		for currentdefine in "$@"; do
			echo "#define $currentdefine" >> include/BuildType.h
		done
	fi
else
	if [[ ! -f include/BuildType.h || -s include/BuildType.h ]];
	then
		echo "" > include/BuildType.h
	fi 
fi 
