#include <Nexus/Core.h>
#include <Nexus/Window.h>
#include <Nexus/Graphics.h>

#include <malloc.h>

#include <stdio.h>

static nxWindow gWindow;
static nxGraphicsContext gContext;
static nxPhysicalDevice gPhysicalDevice;
static nxSurface gSurface;
static nxSurfaceCapabilities gSurfaceCapabilities = { };
static nxLogicalDevice gLogicalDevice;
static nxSwapChain gSwapChain;
static nxCommandPool gGraphicsCommandPool;
static nxCommandPool gPresentCommandPool;
static nxCommandPool gTransferCommandPool;
static nxCommandBuffer gGraphicsCommandBuffer;
static nxCommandBuffer gPresentCommandBuffer;
static nxCommandBuffer gTransferCommandBuffer;
static uint32_t gImageCount;
static nxImage *gImages;
static nxImageView *gImageViews;

void onResize(nxWindow window, uint32_t width, uint32_t height) {
	nxVec2u size = { width, height };
	size = nxClampVec2u(size, gSurfaceCapabilities.minImageSize, gSurfaceCapabilities.maxImageSize);
	nxResizeSwapChainImages(gSwapChain, size);
	nxGetSwapChainImages(gSwapChain, gImages);
	for (uint32_t i = 0; i < gImageCount; i++) {
		nxDestroyImageView(&gImageViews[i]);
		gImageViews[i] = nxCreateImageView(gLogicalDevice, gImages[i], NX_IMAGE_VIEW_TYPE_2D, NX_FORMAT_R8G8B8A8_UNORM);
	}
}

void init() {
	gWindow = nxCreateWindow("Hello World!", 1920 / 2, 1080 / 2);

	gContext = nxCreateGraphicsContext();

	uint32_t physicalDeviceCount = nxGetPhysicalDeviceCount(gContext);

	nxPhysicalDevice *physicalDevices = (nxPhysicalDevice*)malloc(physicalDeviceCount * sizeof(nxPhysicalDevice));
	nxGetPhysicalDevices(gContext, physicalDevices);

	for (uint32_t i = 0; i < physicalDeviceCount; i++) {
		nxPhysicalDevice device = physicalDevices[i];

		if (nxGetPhysicalDeviceType(device) == NX_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			gPhysicalDevice = device;
			break;
		}
		nxDestroyPhysicalDevice(&device);
	}

	char name[NX_NAME_MAX_SIZE] = {};
	nxGetPhysicalDeviceName(gPhysicalDevice, name);
	printf("Device: %s\n", name);

	gSurface = nxCreateSurface(gContext, gWindow);

	gLogicalDevice = nxCreateLogicalDevice(gPhysicalDevice, gSurface);

	uint32_t supportedFormatCount = nxGetSupportedSurfaceFormatCount(gPhysicalDevice, gSurface);

	nxFormat *supportedFormats = (nxFormat*)malloc(supportedFormatCount * sizeof(nxFormat));
	nxGetSupportedSurfaceFormats(gPhysicalDevice, gSurface, supportedFormats);

	nxFormat format = NX_FORMAT_UNKNOWN;

	printf("Supported formats: %u\n", supportedFormatCount);

	for (uint32_t i = 0; i < supportedFormatCount; i++) {
		if (supportedFormats[i] != NX_FORMAT_R8G8B8A8_UNORM) continue; 
		format = supportedFormats[i];
	}

	gSurfaceCapabilities = nxGetSurfaceCapabilities(gPhysicalDevice, gSurface);
	printf("Min image size: %ux%u\n", gSurfaceCapabilities.minImageSize.x, gSurfaceCapabilities.minImageSize.y);
	printf("Max image size: %ux%u\n", gSurfaceCapabilities.maxImageSize.x, gSurfaceCapabilities.maxImageSize.y);

	gSwapChain = nxCreateSwapChain(gLogicalDevice, gSurface, format, gSurfaceCapabilities.minImageSize, gSurfaceCapabilities.minImageCount);
	
	gGraphicsCommandPool = nxCreateCommandPool(gLogicalDevice, NX_COMMAND_POOL_TYPE_GRAPHICS);
	gPresentCommandPool = nxCreateCommandPool(gLogicalDevice, NX_COMMAND_POOL_TYPE_PRESENT);
	gTransferCommandPool = nxCreateCommandPool(gLogicalDevice, NX_COMMAND_POOL_TYPE_TRANSFER);

	gGraphicsCommandBuffer = nxAllocateCommandBuffer(gGraphicsCommandPool);
	gPresentCommandBuffer = nxAllocateCommandBuffer(gPresentCommandPool);
	gTransferCommandBuffer = nxAllocateCommandBuffer(gTransferCommandPool);

	gImageCount = nxGetSwapChainImageCount(gSwapChain);
	gImages = (nxImage*)malloc(gImageCount * sizeof(nxImage));
	gImageViews = (nxImageView*)malloc(gImageCount * sizeof(nxImageView));
	nxGetSwapChainImages(gSwapChain, gImages);

	for (uint32_t i = 0; i < gImageCount; i++) {
		gImageViews[i] = nxCreateImageView(gLogicalDevice, gImages[i], NX_IMAGE_VIEW_TYPE_2D, NX_FORMAT_R8G8B8A8_UNORM);
	}

	nxShowWindow(gWindow);
	nxMaximizeWindow(gWindow);

	nxSetWindowSizeCallback(gWindow, onResize);
}

void update() {

}

void draw() {
	uint32_t nextImage = nxGetNextSwapChainImageIndex(gSwapChain);
	nxPresentSwapChainImage(gSwapChain, nextImage);
	nxUpdateWindow(gWindow);
}

void shutdown() {

	nxFreeCommandBuffer(&gTransferCommandBuffer);
	nxFreeCommandBuffer(&gPresentCommandBuffer);
	nxFreeCommandBuffer(&gGraphicsCommandBuffer);

	nxDestroyCommandPool(&gTransferCommandPool);
	nxDestroyCommandPool(&gPresentCommandPool);
	nxDestroyCommandPool(&gGraphicsCommandPool);

	for (uint32_t i = 0; i < gImageCount; i++) {
		nxDestroyImageView(&gImageViews[i]);
	}

	free(gImageViews);
	free(gImages);

	nxDestroySwapChain(&gSwapChain);
	nxDestroyLogicalDevice(&gLogicalDevice);
	nxDestroySurface(&gSurface);
	nxDestroyPhysicalDevice(&gPhysicalDevice);
	nxDestroyGraphicsContext(&gContext);
	nxDestroyWindow(&gWindow);
}

int main(int argc, char **argv) {
	init();

	while (!nxIsWindowClosed(gWindow)) {
		update();
		draw();
	}

	shutdown();
}