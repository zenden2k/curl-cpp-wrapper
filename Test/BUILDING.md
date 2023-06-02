# Building test project

## On Windows with Visual Studio

```bash
mkdir Build
cd Build
conan install .. --build=missing -s build_type=Debug
cmake .. -G "Visual Studio 16 2019" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake  -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

## On Linux

```bash
cd Build
conan install .. --build=missing -s build_type=Debug
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

# Test server

## Requirements

Requires python 2, flask

```bash
pip install flask
```

## Start server

```bash
python test_server.py
```



