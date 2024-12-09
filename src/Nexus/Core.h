#ifndef __NX_CORE_H__
#define __NX_CORE_H__

#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>

#define NX_INVALID_HANDLE 0
typedef uint32_t nxHandle;

#define NX_FALSE 0
#define NX_TRUE 1

#define NX_ASSERT(x) if (!(x)) { __debugbreak(); }

#endif // __NX_CORE_H__