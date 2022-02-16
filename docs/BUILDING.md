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

The bulk of code resides in WorldEdit with tests for this code unsurprisingly being in WorldEditTests. WorldEditApp contains the UI and the code that creates other componenets and ties them together. The resulting executable from `WorldEditApp` will be placed into `bin/$Config/` while `WorldEditTests` will go into `tests/bin/`.

## Running
 `WorldEditApp.exe` depends on the folders `shaders` and `fonts` being on the current path in order to run. For convenience the Post Build task will copy these folders into the output folder. This let's you easilly run executable.
 
For launching from in Visual Studio though the work directory must be explicitly set (either to the correct `bin/` folder or to the solution's root folder), go into the project properties, Debugging and change Work Directory to `$(SolutionDir)`. Starting the project from Visual Studio should now work. `WorldEditApp.exe` also depends on the other contents of the `bin` folder (excluding `WorldEditTests.exe`).

The settings on the Debugging page are per-user and not synced into the Git repository, thus you must set them up yourself.

`WorldEditTests.exe` depends on it's current directory being the `tests` folder. The default Debugging settings should already be set to this so running directly from within Visual Studio should work.