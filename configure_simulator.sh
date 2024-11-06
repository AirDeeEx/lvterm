if [ -e "./CMakeLists.txt" ]
then
	cmake -Bbuild/ -GNinja \
		-DCMAKE_BUILD_TYPE=MinSizeRel \
		-DBUILD_SHARED_LIBS=ON \
		-DUSE_SIMULATOR=ON
else
	echo "Not cmake root dir!"
	exit 1
fi