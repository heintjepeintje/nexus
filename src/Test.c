#include <Nexus/Core.h>
#include <Nexus/Window.h>
#include <Nexus/Graphics.h>

#include <malloc.h>

#include <stdio.h>

static nxWindow gWindow = NX_INVALID_HANDLE;
static nxGraphicsContext gContext = NX_INVALID_HANDLE;
static nxPhysicalDevice gPhysicalDevice = NX_INVALID_HANDLE;
static nxSurface gSurface = NX_INVALID_HANDLE;
static nxSurfaceCapabilities gSurfaceCapabilities = { };
static nxLogicalDevice gLogicalDevice = NX_INVALID_HANDLE;
static nxSwapChain gSwapChain = NX_INVALID_HANDLE;
static nxCommandPool gGraphicsCommandPool = NX_INVALID_HANDLE;
static nxCommandPool gPresentCommandPool = NX_INVALID_HANDLE;
static nxCommandPool gTransferCommandPool = NX_INVALID_HANDLE;
static nxCommandBuffer gGraphicsCommandBuffer = NX_INVALID_HANDLE;
static nxCommandBuffer gPresentCommandBuffer = NX_INVALID_HANDLE;
static nxCommandBuffer gTransferCommandBuffer = NX_INVALID_HANDLE;

void onResize(nxWindow window, uint32_t width, uint32_t height) {
	nxVec2u size = { width, height };
	size = nxClampVec2u(size, gSurfaceCapabilities.minImageSize, gSurfaceCapabilities.maxImageSize);
	nxResizeSwapChainImages(gSwapChain, size);
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

	char name[NX_PHYSICAL_DEVICE_NAME_MAX_SIZE] = {};
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

	nxShowWindow(gWindow);
	nxMaximizeWindow(gWindow);

	nxSetWindowSizeCallback(gWindow, onResize);
}

void update() {

}

void draw() {
	nxUpdateWindow(gWindow);
}

void shutdown() {
	nxFreeCommandBuffer(&gTransferCommandBuffer);
	nxFreeCommandBuffer(&gPresentCommandBuffer);
	nxFreeCommandBuffer(&gGraphicsCommandBuffer);

	nxDestroyCommandPool(&gTransferCommandPool);
	nxDestroyCommandPool(&gPresentCommandPool);
	nxDestroyCommandPool(&gGraphicsCommandPool);
	
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