## Requirements

- Visual Studio 2022 with MSVC C++ Build Tools

The following is not required but is useful as it let's you run the tests from within Visual Studio.

- [Test Adapter for Catch2](https://marketplace.visualstudio.com/items?itemName=JohnnyHendriks.ext01)

## Building

For simplicity a regular old Visual Studio solution and projects are used for building. There are 3 main projects,

- WorldEdit
- WorldEditApp
- WorldEditTests

The bulk of code resides in WorldEdit with tests for this code unsurprisingly being in WorldEditTests. WorldEditApp contains the UI and the code that creates other components and ties them together. The resulting executable from `WorldEditApp` will be placed into `bin/$Config/` while `WorldEditTests` will go into `tests/bin/`.

However before building these projects you must first build the third party dependencies. For compile time benefits they are kept outside the main solution and must be built separately. This can be done by building `third_party/third_party.sln` for all three configs (Debug/Develop/Release) by using the `third_party/build_all.bat` script from a native tools command prompt.

You should only need to rebuild the dependencies when you update the compiler or very specific compiler flags. 

### Dependencies

Previously most dependencies these were aquired through vcpkg with only a couple vendored in. This did work great for the most part and was simple to use. But it wasn't without drawbacks.

For one using non-default compile options like `/guard:ehcont` is much harder to do and by vcpkg's nature dependencies are missing from version control and have to be acquired separately.

Since one of the cool things about hobby projects is getting to try different approaches I've now stopped using vcpkg for this project and have instead added the dependencies directly, along with build files to `third_party/`. I'm sure this is horrible for various reasons but I'm interested to explore it for myself and the upside of gaining more control over the build for things like `/guard:ehcont` seems worth it.

## Running
Running `WorldEdit.exe` is simple just find it in `bin/$Config/` and launch it. It has no dependencies except the DX12 Agility SDK runtime (and on up-to-date Windows 11 it shouldn't even need this) which is in the `./D3D12` (relative the the executable).

While developing and debugging it maybe useful to make use of the `-project`, `-world` and `-gpu_debug_layer` command line arguments. See [command line.txt](./command%20line.txt).

`WorldEditTests.exe` depends on it's current directory being the `tests` folder. The default Debugging settings should already be set to this so running directly from within Visual Studio should work.