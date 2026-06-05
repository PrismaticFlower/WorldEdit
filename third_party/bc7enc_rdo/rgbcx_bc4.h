// BC4 encoder extracted from rgbcx.h and rgbcx.cpp, see those files for full source. WorldEdit only needs the BC4 encoder and this allows us to skip including the BC1 tables.

// Public Domain or MIT license (you choose - see below), written by Richard Geldreich 2020 <richgel99@gmail.com>.

#include <stdint.h>

#ifndef RGBCX_BC4_INCLUDE_H
#define RGBCX_BC4_INCLUDE_H

namespace rgbcx
{
   void encode_bc4(void* pDst, const uint8_t* pPixels, uint32_t stride);
}

#endif