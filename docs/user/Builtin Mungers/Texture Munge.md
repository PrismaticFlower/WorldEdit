# Texture Munge

WorldEdit's builtin texture munge supports all the PC options listed in "misc_documentation.doc" from the modtools and then some. It is currently only 
used when munging for PC. In testing on my PC it is several times faster than `pc_TextureMunge`.

Also note that the algorithms for options like `-saturation`, `-bumpmap` and `-hiqbumpmap` will not be perfect matches for what is in pc_TextureMunge. They are best guesses for what will functionally be the same even if 
the results differ slightly. That doesn't mean they're bad though, the algorithm for `-bumpamp` for instance is the same as what WorldEdit has been using when it loads textures with the `-bumpamp` option.

## Supported Options

| Option | Arg | Description |
|--------|-----|-------------|
| -maps | \<max mips\> | Sets the maximum the number of mipmaps in the texture. There is almost never a reason to use this. |
| -bordercolor | \<hexcolor> | Sets a colour to fill the border of each mipmap with. Takes HTML style colours i.e `-bordercolor ff0000` for a red border. |
| -ubordercolor | \<hexcolor> | Same as `-border` but only fills the right and left edges. |
| -vbordercolor | \<hexcolor> | Same as `-border` but only fills the top and bottom edges. |
| -borderalpha | \<hexnumber> | Sets an alpha value to fill the border of each mipmap with. Takes a hexadecimal number i.e `-borderalpha 80` for a half transparent border. |
| -uborderalpha | \<hexnumber> | Same as `-borderalpha` but only fills the right and left edges. |
| -vborderalpha | \<hexnumber> | Same as `-borderalpha` but only fills the top and bottom edges. |
| -saturation | \<saturation> | Adjusts the saturation of the texture. 0.5 is no change, 0.0 is greyscale and 1.0 doubles the saturation. This will not exactly match the modtools' output but should be close enough. |
| -cubemap | | Sets the texture to be folded into a cubemap. Texture must have a 4:3 aspect ratio with the following layout. See [Cubemap Layout](#cubemap-layout) |
| -volume  | | Sets the texture to be a volume texture. This must be used with `-depth` and the texture's height must be divisible by the the supplied depth. The volume textures size will be `Width * (Height / Depth) * Depth`. |
| -depth   | \<depth> | Sets the depth of a volume texture. |
| -bumpmap | | Treat the texture as a height map and generate a normal map from it. See also `-bumpscale`. |
| -hiqbumpmap | | Treat the texture as a height map and generate a normal map from it using an alternative algorithm to `-bumpmap`. See also `-bumpscale`. |
| -overridenzto1 | | Force the Z (blue) channel in all mipmaps to 1. |
| -bumpscale | \<scale> | Sets the scale of the height map when generating a normal maps. Defaults to `1.0`. |
| -format | \<format> | Sets the output format for the texture. See [Formats](#formats). |
| -forceformat | \<format> | Alias for `-format`. |
| -detailbias | \<bias> | Sets the detail bias for a texture. According to "misc_documentation.doc" setting this can let you keep large textures loading on Low and Medium Texture Quality settings. This is supported for completeness but it is unlikely to be useful. | 

## Formats (PC)


### Meta Formats (PC)

| Format | Description |
|--------|-------------|
| detail | The texture will be exported in a greyscale format. If the texture is all black or all white AND has an alpha channel the alpha channel will be used for the greyscale map. Else the colour channels will be converted to greyscale and used. No alpha channel. |
| terrain_detail | Alias for `detail`. |
| bump | Export in a format suitable for a normal map. Normalize mipmaps after generating them. No alpha channel. |
| terrain_bump | Alias for `bump`. |
| bump_alpha | Same as `bump` but export in a format keeping the alpha channel as well. |
| terrain_bump_alpha | Alias for `bump_alpha`. |
| compressed | Export in a compressed format. No alpha channel. |
| compressed_alpha | Export in a compressed format with the alpha channel. |

### Explicit Formats (PC)

| Format | Description |
|--------|-------------|
| DXT1 | Classic DXT1/BC1 compressed format. Supports 1-bit alpha. Effective size for low mipmaps is 0.5 bytes/pixel. |
| DXT3 | Like DXT1 but with an uncompressed 4-bit alpha map. Effective size for low mipmaps is 1 bytes/pixel. Rarely useful, DXT5 is almost always better and smoother. |
| DXT5 | Like DXT1 but with a compressed 8-bit alpha map as well. |
| A8R8G8B8 | 8-bit colour, 8-bit alpha |
| X8R8G8B8 | 8-bit colour |
| A4R4G4B4 | 4-bit colour, 4-bit alpha |
| A1R5G5B5 | 5-bit colour format, 1-bit alpha. Rarely useful, DXT1 or DX5 is often better. |
| R5G6B5 | 5-bit red, 6-bit green, 5-bit blue. Rarely useful, DXT1 is often better. |
| X1R5G5B5 | 5-bit colour format. Rarely useful, DXT1 is often better. |
| A8L8 | 8-bit greyscale, 8-bit alpha |
| A8 | 8-bit alpha |
| L8 | 8-bit greyscale |
| A4L4 | 4-bit greyscale format with 4-bit alpha. Rarely useful, included for completeness. |
| V8U8 | Used by normal maps for use with the refraction or water shader.  |

### Cubemap Layout
Below is the layout for an unfolded cubemap given in ASCII art form.
```
 ___________
|--|+Y|--|--|
|-X|+Z|+X|-Z|
|--|-Y|--|--|
 ‾‾‾‾‾‾‾‾‾‾‾
```

## Error Codes
Below is a list of error codes the texture munger may produce.

### TGA_LOAD_FAIL

Failed to load .tga file. This could mean the file is corrupted, is saved in a niche format or in a non-standard format.

Use an external tool (like an image editor) to validate the texture and make sure it is saved in a supported data type.

### TGA_LOAD_UNEXPECTED_FORMAT

The .tga file was loaded successfully but it's format is not understood by WorldEdit. 

Use an external tool (like an image editor) to resave the .tga file.

### TGA_LOAD_TOO_LARGE_2D

The loaded .tga file was too large to use as a 2D texture.

Use an image editor to downsample and save a lower resolution copy of the texture. Then try munging again.

### TGA_LOAD_TOO_LARGE_CUBE

The loaded .tga file is to be used as a cubemap but is too large.

Use an image editor to downsample and save a lower resolution copy of the texture. Then try munging again.

### TGA_LOAD_TOO_LARGE_VOLUME

The loaded .tga file is to be used as a volume texture but is too large.

Use an image editor to downsample and save a lower resolution copy of the texture. Adjust the "-depth" option in the .tga.option file as needed as well. Then try munging again.

### TGA_LOAD_BAD_CUBE_SOURCE_ASPECT_RATIO

The loaded .tga file is to be used as a cubemap but has the incorrect aspect ratio. Cubemaps must have a 4:3 aspect ratio with the layout from [Cubemap Layout](#cubemap-layout).


### TGA_LOAD_BAD_VOLUME_HEIGHT

The loaded .tga file is to be used as a volume texture but it's height is not divisible by the depth set in the .tga.option file.


### TGA_LOAD_BAD_VOLUME_DEPTH

The requested depth for a volume texture is invalid. Either too large or 0.

### OPTION_LOAD_BAD_MAPS

Bad "-maps" option in .tga.option file. "-maps" must be followed by an unsigned integer setting the maximum number of mipmaps to export.

i.e "-maps 2"

### option_load_bad_border_color

Bad border color option in .tga.option file. The option must followed by a color in HTML notation or a hexadecimal number representing the color.

Examples:

"-bordercolor ff0000" to set the border to red
"-ubordercolor #00ff00" to set the left and right edges to green
"-bordercolor 0x0000ff" to set the border to blue

### OPTION_LOAD_BAD_BORDER_ALPHA

Bad border alpha option in .tga.option file. The option must followed by a hexadecimal number representing the alpha.

i.e "-borderalpha ff"

### OPTION_LOAD_AMBIGUOUS_TYPE

Both "-cubemap" and "-volume" have been set in the .tga.option file. Only one can be used at a time.

### OPTION_LOAD_BAD_SATURATION

Bad "-saturation" option in .tga.option file. "-saturation" must be followed by a number setting the saturation adjustment.

i.e "-saturation 0.75"

### OPTION_LOAD_BAD_DEPTH

Bad "-depth" option in .tga.option file. "-depth" must be followed by an unsigned integer setting the depth of the volume map.

i.e "-depth 16"

### OPTION_LOAD_BAD_BUMPSCALE

Bad "-bumpscale" option in .tga.option file. "-bumpscale" must be followed by a number setting the bump scale to use when generating the normal map.

i.e "-bumpscale 4.0"

### OPTION_LOAD_BAD_FORMAT

Bad "-format" or "-forceformat" option in .tga.option file. "-format" must be followed by the name of the format to use.

i.e "-format compressed"

See here for supported [Formats](#formats-pc).

### OPTION_LOAD_BAD_MAPS

Bad "-detailbias" option in .tga.option file. "-detailbias" must be followed by an unsigned integer setting the detail bias of the munged texture.

i.e "-detailbias 2"

### GENERATE_MIPMAPS_VOLUME_NON_POW2

WorldEdit can not generate mipmaps non-power of 2 volume textures. Either resize the texture or add "-maps 1" to the .tga.option file.

### GENERATE_NORMAL_MAPS_VOLUME

Can not generate normal maps for volume textures. This would be meaningless to the game. Remove both of "-bumpmap" and "-hiqbumpmap" from the .tga.option file.

### FORMAT_SELECT_COMPRESSED_NON_POW2

"-format compressed" has been used on a non-power of 2 texture. This is invalid either resize the texture or use an uncompressed format.

### FORMAT_SELECT_COMPRESSED_ALPHA_NON_POW2

"-format compressed_alpha" has been used on a non-power of 2 texture. This is invalid either resize the texture or use an uncompressed format.

### FORMAT_SELECT_DXT1_NON_POW2

"-format dxt1" has been used on a non-power of 2 texture. This is invalid either resize the texture or use an uncompressed format.

### FORMAT_SELECT_DXT3_NON_POW2

"-format dxt3" has been used on a non-power of 2 texture. This is invalid either resize the texture or use an uncompressed format.

### FORMAT_SELECT_DXT5_NON_POW2

"-format dxt5" has been used on a non-power of 2 texture. This is invalid either resize the texture or use an uncompressed format.

### FORMAT_CONVERT_NOT_ENOUGH_MEMORY

There was not enough memory to convert the texture's format. This is a bug in WorldEdit, please report it. Include the texture and it's .tga.option file

### WRITE_IO_OPEN_ERROR

Failed to open .texture file for output! Make sure the munge out directory is accessible and that nothing currently has the output .texture file open.

### WRITE_IO_GENERIC_ERROR

An unexpected IO error has occured while writing the texture. The Exception Message printed with the error may have more information.


## Compatibility
> This section is about compatibility with very, very old GPUs. It is extremely unlikely to be relevant to you.

Unlike `pc_TextureMunge` the builtin munger doesn't export multiple formats to enable texture quality settings or in case one isn't supported. 
To the best my research there should be very few GPUs this stops from loading your map. An original GeForce 256 seems to have supported the texture
formats the builtin munger will pick to output. And a tech demo for the ATI Radeon 8500 (a GPU from 2001) was using the formats in question.

I've had a harded time finding something that could shed light on the state of support for these formats for Intel GPUs at the time. The oldest I can find is 
from around 2007 for the Intel G35 Express Chipset and that GPU unsurprisingly does support them.

A better researcher than me (or a graphics programmer from 2004) could likely uncover more and give a more definitive answer but my conclusion is that
depending on these formats is safe even if your targeting GPUs from 20+ years ago.

Still if you're really worried you can always disable builtin mungers and just stick with `pc_TextureMunge`
