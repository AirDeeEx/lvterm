#!/bin/sh

if [ -e "./CMakeLists.txt" ]
then
	cmake -Bbuild/ -GNinja \
		-DCMAKE_BUILD_TYPE=MinSizeRel \
		-DBUILD_SHARED_LIBS=OFF \
		-DUSE_SIMULATOR=OFF
else
	echo "Not cmake root dir!"
	exit 1
fi