# Helper Box

Refer to the [documentation](https://franneck94.github.io/HelperBox/).

If you want to use ToolBox++ in parallel, you have to start Toolbox **before** starting this HelperBox.  
You may need to launch the HelperBox.exe as admin.  

Note: Most windows are only displayed in Embark Beach and in the Underworld.

## Developement

* CMake 3.21+, Git
* MSVC 2022 with C++20
* Python 3 for the Documenation

```bash
git clone --recursive https://github.com/franneck94/HelperBox.git
```

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A Win32 -B .
```
