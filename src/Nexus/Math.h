#ifndef __NX_MATH_H__
#define __NX_MATH_H__

#include "Core.h"

typedef struct { float x, y; } nxVec2;
typedef struct { int32_t x, y; } nxVec2i;
typedef struct { uint32_t x, y; } nxVec2u;

typedef struct { float x, y, z; } nxVec3;
typedef struct { int32_t x, y, z; } nxVec3i;
typedef struct { uint32_t x, y, z; } nxVec3u;

typedef struct { float x, y, z, w; } nxVec4;
typedef struct { int32_t x, y, z, w; } nxVec4i;
typedef struct { uint32_t x, y, z, w; } nxVec4u;

uint32_t nxMinU32(uint32_t a, uint32_t b);
uint32_t nxMaxU32(uint32_t a, uint32_t b);

uint32_t nxClampU32(uint32_t n, uint32_t min, uint32_t max);

nxVec2u nxClampVec2u(nxVec2u v, nxVec2u min, nxVec2u max);

#endif // __NX_MATH_H__