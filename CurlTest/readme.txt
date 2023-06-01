# Building CurlTest project

On Windows with Visual Studio:

cd Build
conan install .. --build=missing -s build_type=Debug
cmake .. -G "Visual Studio 16 2019" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake 
cmake --build .

On Linux:

With Conan

cd Build
conan install .. --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .

Without Conan

First you need to install libcurl development libraries with your package manager:

sudo apt-get install libcurl4-openssl-dev
cmake ..
cmake --build .

In MSYS2 (just for test):

pacman -S libcurl-devel
cd Build
cmake ..
cmake --build .

