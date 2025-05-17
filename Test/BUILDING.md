# Building test project

## On Windows with Visual Studio

```bash
mkdir Build
conan install . --build=missing -s build_type=Debug --output-folder=Build
cd Build
cmake .. -G "Visual Studio 16 2019" --toolchain=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

## On Linux

```bash
mkdir Build
conan install . --build=missing -s build_type=Debug --output-folder=Build
cd Build
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



