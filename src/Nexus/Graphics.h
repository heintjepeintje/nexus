#ifndef __NX_GRAPHICS_H__
#define __NX_GRAPHICS_H__

#include "Core.h"
#include "Window.h"
#include "Math.h"

typedef nxHandle nxGraphicsContext;
typedef nxHandle nxPhysicalDevice;
typedef nxHandle nxSurface;
typedef nxHandle nxLogicalDevice;
typedef nxHandle nxSwapChain;
typedef nxHandle nxImage;
typedef nxHandle nxImageView;
typedef nxHandle nxCommandPool;
typedef nxHandle nxCommandBuffer;

typedef enum {
	NX_FORMAT_UNKNOWN = 0,
	NX_FORMAT_R8_UNORM,
	NX_FORMAT_R8_UINT,
	NX_FORMAT_R8_SINT,
	NX_FORMAT_R8G8_UINT,
	NX_FORMAT_R8G8_SINT,
	NX_FORMAT_R8G8_UNORM,
	NX_FORMAT_R8G8B8_UINT,
	NX_FORMAT_R8G8B8_SINT,
	NX_FORMAT_R8G8B8_UNORM,
	NX_FORMAT_R8G8B8A8_UINT,
	NX_FORMAT_R8G8B8A8_SINT,
	NX_FORMAT_R8G8B8A8_UNORM,
	NX_FORMAT_R16_UINT,
	NX_FORMAT_R16_SINT,
	NX_FORMAT_R16_UNORM,
	NX_FORMAT_R16G16_UINT,
	NX_FORMAT_R16G16_SINT,
	NX_FORMAT_R16G16_UNORM,
	NX_FORMAT_R16G16B16_UINT,
	NX_FORMAT_R16G16B16_SINT,
	NX_FORMAT_R16G16B16_UNORM,
	NX_FORMAT_R16G16B16A16_UINT,
	NX_FORMAT_R16G16B16A16_SINT,
	NX_FORMAT_R16G16B16A16_UNORM,
	NX_FORMAT_R32_UINT,
	NX_FORMAT_R32_SINT,
	NX_FORMAT_R32_FLOAT,
	NX_FORMAT_R32G32_UINT,
	NX_FORMAT_R32G32_SINT,
	NX_FORMAT_R32G32_FLOAT,
	NX_FORMAT_R32G32B32_UINT,
	NX_FORMAT_R32G32B32_SINT,
	NX_FORMAT_R32G32B32_FLOAT,
	NX_FORMAT_R32G32B32A32_UINT,
	NX_FORMAT_R32G32B32A32_SINT,
	NX_FORMAT_R32G32B32A32_FLOAT,
	NX_FORMAT_R64_UINT,
	NX_FORMAT_R64_SINT,
	NX_FORMAT_R64_FLOAT,
	NX_FORMAT_R64G64_UINT,
	NX_FORMAT_R64G64_SINT,
	NX_FORMAT_R64G64_FLOAT,
	NX_FORMAT_R64G64B64_UINT,
	NX_FORMAT_R64G64B64_SINT,
	NX_FORMAT_R64G64B64_FLOAT,
	NX_FORMAT_R64G64B64A64_UINT,
	NX_FORMAT_R64G64B64A64_SINT,
	NX_FORMAT_R64G64B64A64_FLOAT
} nxFormat;

typedef enum {
	NX_PHYSICAL_DEVICE_TYPE_UNKNOWN = 0,
	NX_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
	NX_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
	NX_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
	NX_PHYSICAL_DEVICE_TYPE_CPU
} nxPhysicalDeviceType;

typedef enum {
	NX_IMAGE_VIEW_TYPE_NONE = 0,
	NX_IMAGE_VIEW_TYPE_1D,
	NX_IMAGE_VIEW_TYPE_2D,
	NX_IMAGE_VIEW_TYPE_3D,
	NX_IMAGE_VIEW_TYPE_CUBE,
	NX_IMAGE_VIEW_TYPE_1D_ARRAY,
	NX_IMAGE_VIEW_TYPE_2D_ARRAY,
	NX_IMAGE_VIEW_TYPE_CUBE_ARRAY
} nxImageViewType;

typedef enum {
	NX_IMAGE_LAYOUT_UNKNOWN = 0,
	NX_IMAGE_LAYOUT_PRESENT
} nxImageLayout;

typedef enum {
	NX_COMMAND_POOL_TYPE_NONE = 0,
	NX_COMMAND_POOL_TYPE_GRAPHICS,
	NX_COMMAND_POOL_TYPE_PRESENT,
	NX_COMMAND_POOL_TYPE_TRANSFER
} nxCommandPoolType;

typedef struct {
	uint32_t minImageCount, maxImageCount;
	nxVec2u minImageSize;
	nxVec2u maxImageSize;
	uint32_t maxImageLayers;
} nxSurfaceCapabilities;

nxGraphicsContext nxCreateGraphicsContext();
uint32_t nxGetPhysicalDeviceCount(nxGraphicsContext context);
void nxGetPhysicalDevices(nxGraphicsContext context, nxPhysicalDevice *devices);
void nxDestroyGraphicsContext(nxGraphicsContext *context);

void nxGetPhysicalDeviceName(nxPhysicalDevice device, char *buffer);
uint32_t nxGetPhysicalDeviceID(nxPhysicalDevice device);
uint32_t nxGetPhysicalDeviceVendorID(nxPhysicalDevice device);
nxPhysicalDeviceType nxGetPhysicalDeviceType(nxPhysicalDevice device);
nxSurfaceCapabilities nxGetSurfaceCapabilities(nxPhysicalDevice device, nxSurface surface);
uint32_t nxGetSupportedSurfaceFormatCount(nxPhysicalDevice device, nxSurface surface);
void nxGetSupportedSurfaceFormats(nxPhysicalDevice device, nxSurface surface, nxFormat *formats);
void nxDestroyPhysicalDevice(nxPhysicalDevice *device);

nxSurface nxCreateSurface(nxGraphicsContext context, nxWindow window);
void nxDestroySurface(nxSurface *surface);

nxLogicalDevice nxCreateLogicalDevice(nxPhysicalDevice device, nxSurface surface);
void nxDestroyLogicalDevice(nxLogicalDevice *device);

nxSwapChain nxCreateSwapChain(nxLogicalDevice device, nxSurface surface, nxFormat format, nxVec2u imageSize, uint32_t imageCount);
void nxResizeSwapChainImages(nxSwapChain swapChain, nxVec2u newImageSize);
uint32_t nxGetSwapChainImageCount(nxSwapChain swapChain);
void nxGetSwapChainImages(nxSwapChain swapChain, nxImage *images);
uint32_t nxGetNextSwapChainImageIndex(nxSwapChain swapChain);
void nxPresentSwapChainImage(nxSwapChain swapChain, uint32_t index);
void nxDestroySwapChain(nxSwapChain *swapChain);

void nxDestroyImage(nxImage *image);

nxImageView nxCreateImageView(nxLogicalDevice device, nxImage image, nxImageViewType type, nxFormat format);
void nxDestroyImageView(nxImageView *view);

nxCommandPool nxCreateCommandPool(nxLogicalDevice device, nxCommandPoolType type);
void nxDestroyCommandPool(nxCommandPool *pool);

nxCommandBuffer nxAllocateCommandBuffer(nxCommandPool pool);
void nxFreeCommandBuffer(nxCommandBuffer *buffer);

void nxCmdConvertImageLayout(nxCommandBuffer buffer, nxImage image, nxImageLayout dest);

#endif // __NX_GRAPHICS_H__