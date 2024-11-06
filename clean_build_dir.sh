#!/bin/sh
if [ -e "./CMakeLists.txt" ]
then
	echo "Cleaning cmake build dir..."
	rm -r ./build/*
	exit 0
else
	echo "Not cmake root dir!"
	exit 1
fi