#include <mimalloc.h>

#define STBIR_MALLOC(size, user_data) mi_malloc(size)
#define STBIR_FREE(ptr, user_data) mi_free(ptr)
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include "stb_image_resize2.h"