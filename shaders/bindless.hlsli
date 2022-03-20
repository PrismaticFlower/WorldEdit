#pragma once

Texture2D texture_2d_table[] : register(t0, space1000);
Texture2DArray texture_2d_array_table[] : register(t0, space1001);
TextureCube texture_cube_table[] : register(t0, space1002);
TextureCubeArray texture_cube_array_table[] : register(t0, space1003);
Texture3D texture_3d_table[] : register(t0, space1004);
ByteAddressBuffer raw_buffer_table[] : register(t0, space1005);

Texture2D GetTexture2D(uint i)
{
   return texture_2d_table[i];
}

Texture2DArray GetTexture2DArray(uint i)
{
   return texture_2d_array_table[i];
}

TextureCube GetTextureCube(uint i)
{
   return texture_cube_table[i];
}

TextureCubeArray GetTextureCubeArray(uint i)
{
   return texture_cube_array_table[i];
}

Texture3D GetTexture3D(uint i)
{
   return texture_3d_table[i];
}

ByteAddressBuffer GetByteAddressBuffer(uint i)
{
   return raw_buffer_table[i];
}