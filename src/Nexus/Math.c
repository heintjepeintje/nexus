#include "Math.h"

uint32_t nxMinU32(uint32_t a, uint32_t b) {
	return a < b ? a : b;
}

uint32_t nxMaxU32(uint32_t a, uint32_t b) {
	return a > b ? a : b;
}


uint32_t nxClampU32(uint32_t n, uint32_t min, uint32_t max) {
	return nxMinU32(nxMaxU32(n, min), max);
}

nxVec2u nxClampVec2u(nxVec2u v, nxVec2u min, nxVec2u max) {
	nxVec2u result = { };
	result.x = nxClampU32(v.x, min.x, max.x);
	result.y = nxClampU32(v.y, min.y, max.y);
	return result;
}