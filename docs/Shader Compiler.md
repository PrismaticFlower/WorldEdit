## ShaderCompiler

The shader "compiler" used by WorldEdit is written in C# to mix things up. To avoid introducing another dependency for people looking to build WorldEdit it's provided pre-built and kept in a separate VS solution.

It reads a list of shaders (`shaders/WorldEdit.shaders`), launches DXC for them, takes the results and writes them (along with the info needed to recompiled the shader at runtime) to .cpp files. Each shader will get it's own file (so when you add a shader remember to add it to the VS project). Additionally `shader_list.cpp` will be written out with the complete list of shaders.

On subsequent builds the write times of a shader and it's dependencies will be read and compared to what they were when the shader was last built. If they're all the same the shader will not be rebuilt.

The format of `WorldEdit.shaders` is documented in the file's header.

## Background

For a while now WorldEdit's shaders were compiled at runtime at app start up. While not the best for start up times (though even with it WorldEdit's start time was around 1s) it was extremely convenient, especially for implementing dynamic shader reload.

In fact not wanting to let go of shader reloading is partly how the runtime compilation lasted so long. But start up times aren't even the only downside (though it is the one that affects users the most). We also have to bundle the DirectX Shader Compiler and ship the `shaders/` folder.

Now in solving all of this I had 4 key requirements.

1. Easily support shaders with multiple entry points in one file (i.e the vertex and pixel shaders together). The runtime system did support this.
2. No extra file IO at start up. Shaders should be embedded in the .exe itself.
3. Continue to support runtime shader reloading.
4. When building only shaders that need recompiling should get recompiled.

For reference here is how Visual Studio's builtin support compares to those goals. 

1. Not supported 🙁 You must make a separate file and include the main file in it.
2. Supported, it can save the shader as a .h that you can then include.
3. Not supported, you'd have to manually supply the parameters to load and rebuild the shader alongside where you include the .h file.
4. Supported, in fact unsurprisingly Visual Studio supports this the best.

And for completeness using DXC directly from a simple batch script or something.

1. Supported
2. Supported, it can save the shader as a .h that you can then include.
3. Not supported, you'd have to manually supply the parameters to load and rebuild the shader alongside where you include the .h file.
3. Not supported.

So I chose to go with what everyone seems to end up doing, I wrapped the actual shader compilers in another tool to call it and do what I want.
