# Helper Box

Refer to the [documentation](https://franneck94.github.io/HelperBox/).

If you want to use ToolBox++ in parallel, you have to start Toolbox **before** starting this HelperBox.  
You may need to launch the HelperBox.exe as admin.  

Note: Most windows are only displayed in Embark Beach and in the Underworld.

## Developement

* CMake 3.21+
* MSVC 2019 or 2022
* C++20
* Git
* Python 3 for the Documenation

```bash
git clone --recursive https://github.com/franneck94/HelperBox.git
```

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A Win32 -B .
```
