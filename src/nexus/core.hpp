#pragma once

#include <cstdint>
#include <memory>
#include <string>

#define NX_ASSERT(x) if (!(x)) { __debugbreak(); }