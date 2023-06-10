# Building test project

## On Windows with Visual Studio

```bash
mkdir Build
cd Build
conan install .. --build=missing -s build_type=Debug
cmake .. -G "Visual Studio 16 2019" --toolchain=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

## On Linux

```bash
cd Build
conan install .. --build=missing -s build_type=Debug
cmake .. --toolchain=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

# Test server

Test server is listening 127.0.0.1:5000.

## Requirements

Requires python, flask.

```bash
pip install flask
```

## Start server

```bash
python test_server.py
```



