prepare:
	rm -rf build
	mkdir build
	cd build && cmake .. -G "Visual Studio 16 2019" -T v142 -A win32 -B .
