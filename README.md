# Helper Box

## Usage

Refer to the [documentation](https://franneck94.github.io/HelperBox/).

If you want to use ToolBox++ in parallel, you have to start Toolbox **before** starting this HelperBox.  

If you want to launch the HelperBox for a certain GW instance (Multi-launcher use-case), you have to get the process id of the GW instance:

```bash
tasklist | grep Gw # for unix like shell
tasklist # for standard cmd
```

Then launch the HelperBox.exe from the CMD:

```bash
HelperBox.exe /pid PID
```

## Developement

### Requirements

* CMake 3.21+
* Visual Studio 2019 (v142)
* C++17
* Git

```bash
git clone --recursive https://github.com/franneck94/HelperBox.git
```

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -T v142 -A Win32 -B .
```
