all: Debug

default:
	cmake -S . -B build
	cmake --build build

Debug:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build

debug: Debug

dev: Debug

DEB:
	cmake -S . -B build
	cmake --build build
	cpack -G DEB --config build/CPackConfig.cmake

RPM:
	cmake -S . -B build
	cmake --build build
	cpack -G RPM --config build/CPackConfig.cmake

clean:
	rm -rf build _CPack_Packages *.rpm *.deb

