# Helper Box

## Requirements

* CMake 3.16+
* Visual Studio MSVC Compiler 2019 or above (C++17)
* Git

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -T v142 -A Win32 -B .
```

## Usage

Call the executable in the CMD like this:

```bash
HelperBox.exe /localdll
```

## Note

If you want to use Toolbox in parallel, you have to start Toolbox *before* starting this HelperBox.
