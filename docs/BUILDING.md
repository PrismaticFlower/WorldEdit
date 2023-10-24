## Requirements

- Visual Studio 2022 with MSVC C++ Build Tools
- [vcpkg](https://github.com/microsoft/vcpkg) (with it's Visual Studio integration installed)

The following is not required but is useful as it let's you run the tests from within Visual Studio.

- [Test Adapter for Catch2](https://marketplace.visualstudio.com/items?itemName=JohnnyHendriks.ext01)

## Building

For simplicity a regular old Visual Studio solution and projects are used for building. There are 3 projects,

- WorldEdit
- WorldEditApp
- WorldEditTests

The bulk of code resides in WorldEdit with tests for this code unsurprisingly being in WorldEditTests. WorldEditApp contains the UI and the code that creates other components and ties them together. The resulting executable from `WorldEditApp` will be placed into `bin/$Config/` while `WorldEditTests` will go into `tests/bin/`.

## Running
Running `WorldEdit.exe` is simple just find it in `bin/$Config/` and launch it. It has no dependencies except the DX12 Agility SDK runtime (and on up-to-date Windows 11 it shouldn't even need this) which is in the `./D3D12` (relative the the executable).

While developing and debugging it maybe useful to make use of the `-project`, `-world` and `-gpu_debug_layer` command line arguments. See [command line.txt](./command%20line.txt).

`WorldEditTests.exe` depends on it's current directory being the `tests` folder. The default Debugging settings should already be set to this so running directly from within Visual Studio should work.