# Helper Box

## Requirements

* CMake 3.21+
* Visual Studio 2019
* Git

## Clone Repo

```bash
git clone --recursive https://github.com/franneck94/HelperBox.git
```

## Build from Source

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -T v142 -A Win32 -B .
```

## Usage

Refer to the documentation files in [here](./docs/).

## Note

If you want to use Toolbox in parallel, you have to start Toolbox **before** starting this HelperBox.
