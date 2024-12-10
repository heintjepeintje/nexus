#include <Nexus/Graphics.h>

#ifdef _WIN32
#	define VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32
#include <vulkan/vulkan.h>

#include <stdio.h>

#define NX_REQUIRED_INSTANCE_EXTENSION_COUNT 3
#define NX_REQUIRED_INSTANCE_LAYER_COUNT 1
#define NX_REQUIRED_DEVICE_EXTENSION_COUNT 1

#define NX_QUEUE_COUNT 3

#define NX_GRAPHICS_QUEUE_INDEX 0
#define NX_PRESENT_QUEUE_INDEX 1
#define NX_TRANSFER_QUEUE_INDEX 2
#define NX_COMPUTE_QUEUE_INDEX 3

#define NX_VKCALL(x) NX_ASSERT((x) == VK_SUCCESS)

typedef struct {
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	uint8_t used;

	VkPhysicalDevice *physicalDevices;
	uint32_t physicalDeviceCount;
} vulkanGraphicsContext;

typedef struct {
	VkPhysicalDevice device;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memoryProperties;

	uint8_t used;
} vulkanPhysicalDevice;

typedef struct {
	nxGraphicsContext context;

	VkSurfaceKHR surface;
	uint8_t used;
} vulkanSurface;

typedef struct {
	nxGraphicsContext context;

	VkDevice device;
	uint32_t graphicsQueueFamily;
	uint32_t presentQueueFamily;
	uint32_t transferQueueFamily;
	// uint32_t computeQueueFamily;

	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;
	// VkQueue computeQueue;

	uint8_t used;
} vulkanLogicalDevice;

typedef struct {
	nxLogicalDevice device;

	VkCommandPool pool;
	nxCommandPoolType type;
	uint8_t used;
} vulkanCommandPool;

typedef struct {
	nxCommandPool pool;

	VkCommandBuffer buffer;
	nxCommandPoolType type;
	uint8_t used;
} vulkanCommandBuffer;

typedef struct {
	nxLogicalDevice device;
	nxSurface surface;

	VkSwapchainKHR swapChain;
	nxVec2u imageSize;
	VkFormat format;

	uint32_t imageCount;
	VkImage *images;

	VkSemaphore semaphore;
	uint32_t nextImageIndex;

	uint8_t used;
} vulkanSwapChain;

typedef struct {
	nxLogicalDevice device;

	VkImage image;
	uint8_t used;
} vulkanImage;

typedef struct {
	nxLogicalDevice device;

	VkImageView imageView;
	uint8_t used;
} vulkanImageView;

vulkanGraphicsContext gGraphicsContexts[NX_MAX_GRAPHICS_CONTEXTS] = { };
vulkanPhysicalDevice gPhysicalDevices[NX_MAX_PHYSICAL_DEVICES] = { };
vulkanSurface gSurfaces[NX_MAX_SURFACES] = { };
vulkanLogicalDevice gLogicalDevices[NX_MAX_LOGICAL_DEVICES] = { };
vulkanCommandPool gCommandPools[NX_MAX_COMMAND_POOLS] = { };
vulkanCommandBuffer gCommandBuffers[NX_MAX_COMMAND_BUFFERS] = { };
vulkanSwapChain gSwapChains[NX_MAX_SWAP_CHAINS] = { };
vulkanImage gImages[NX_MAX_IMAGES] = { };
vulkanImageView gImageViews[NX_MAX_IMAGE_VIEWS] = { };

#define NX_VK_GRAPHICS_CONTEXT(id) (gGraphicsContexts[id - NX_GRAPHICS_CONTEXT_OFFSET])
#define NX_VK_PHYSICAL_DEVICE(id) (gPhysicalDevices[id - NX_PHYSICAL_DEVICE_OFFSET])
#define NX_VK_SURFACE(id) (gSurfaces[id - NX_SURFACE_OFFSET])
#define NX_VK_LOGICAL_DEVICE(id) (gLogicalDevices[id - NX_LOGICAL_DEVIC_OFFSET])
#define NX_VK_COMMAND_POOL(id) (gCommandPools[id - NX_COMMAND_POOL_OFFSET])
#define NX_VK_COMMAND_BUFFER(id) (gCommandBuffers[id - NX_COMMAND_BUFFER_OFFSET])
#define NX_VK_SWAP_CHAIN(id) (gSwapChains[id - NX_SWAP_CHAIN_OFFSET])
#define NX_VK_IMAGE(id) (gImages[id - NX_IMAGE_OFFSET])
#define NX_VK_IMAGE_VIEW(id) (gImageViews[id - NX_IMAGE_VIEW_OFFSET])

static VKAPI_ATTR VkBool32 VKAPI_CALL onDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* data, void *user) {
	switch (severity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
			printf("[VULKAN] > %s\n", data->pMessage);
			break;
		}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
			printf("[VULKAN] > %s\n", data->pMessage);
			break;
		}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
			fprintf(stderr, "[VULKAN] > %s\n", data->pMessage);
			break;
		}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
			fprintf(stderr, "[VULKAN] > %s\n", data->pMessage);
			break;
		}
		default: break;
	}
	return VK_FALSE;
}

nxImageLayout toNxImageLayout(VkImageLayout layout) {
	switch (layout) {
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return NX_IMAGE_LAYOUT_PRESENT;
		default: return NX_IMAGE_LAYOUT_UNKNOWN;
	}
}

VkImageLayout toVkImageLayout(nxImageLayout layout) {
	switch (layout) {
		case NX_IMAGE_LAYOUT_PRESENT: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		default: return VK_IMAGE_LAYOUT_UNDEFINED;
	}
}

nxFormat toNxFormat(VkFormat format) {
	switch (format) {
		case VK_FORMAT_R8_UINT: return NX_FORMAT_R8_UINT;
		case VK_FORMAT_R8_SINT: return NX_FORMAT_R8_SINT;
		case VK_FORMAT_R8_UNORM: return NX_FORMAT_R8_UNORM;
		case VK_FORMAT_R8G8_UINT: return NX_FORMAT_R8G8_UINT;
		case VK_FORMAT_R8G8_SINT: return NX_FORMAT_R8G8_SINT;
		case VK_FORMAT_R8G8_UNORM: return NX_FORMAT_R8G8_UNORM;
		case VK_FORMAT_R8G8B8_UINT: return NX_FORMAT_R8G8B8_UINT;
		case VK_FORMAT_R8G8B8_SINT: return NX_FORMAT_R8G8B8_SINT;
		case VK_FORMAT_R8G8B8_UNORM: return NX_FORMAT_R8G8B8_UNORM;
		case VK_FORMAT_R8G8B8A8_UINT: return NX_FORMAT_R8G8B8A8_UINT;
		case VK_FORMAT_R8G8B8A8_SINT: return NX_FORMAT_R8G8B8A8_SINT;
		case VK_FORMAT_R8G8B8A8_UNORM: return NX_FORMAT_R8G8B8A8_UNORM;
		case VK_FORMAT_R16_UINT: return NX_FORMAT_R16_UINT;
		case VK_FORMAT_R16_SINT: return NX_FORMAT_R16_SINT;
		case VK_FORMAT_R16_UNORM: return NX_FORMAT_R16_UNORM;
		case VK_FORMAT_R16G16_UINT: return NX_FORMAT_R16G16_UINT;
		case VK_FORMAT_R16G16_SINT: return NX_FORMAT_R16G16_SINT;
		case VK_FORMAT_R16G16_UNORM: return NX_FORMAT_R16G16_UNORM;
		case VK_FORMAT_R16G16B16_UINT: return NX_FORMAT_R16G16B16_UINT;
		case VK_FORMAT_R16G16B16_SINT: return NX_FORMAT_R16G16B16_SINT;
		case VK_FORMAT_R16G16B16_UNORM: return NX_FORMAT_R16G16B16_UNORM;
		case VK_FORMAT_R16G16B16A16_UINT: return NX_FORMAT_R16G16B16A16_UINT;
		case VK_FORMAT_R16G16B16A16_SINT: return NX_FORMAT_R16G16B16A16_SINT;
		case VK_FORMAT_R16G16B16A16_UNORM: return NX_FORMAT_R16G16B16A16_UNORM;
		case VK_FORMAT_R32_UINT: return NX_FORMAT_R32_UINT;
		case VK_FORMAT_R32_SINT: return NX_FORMAT_R32_SINT;
		case VK_FORMAT_R32_SFLOAT: return NX_FORMAT_R32_FLOAT;
		case VK_FORMAT_R32G32_UINT: return NX_FORMAT_R32G32_UINT;
		case VK_FORMAT_R32G32_SINT: return NX_FORMAT_R32G32_SINT;
		case VK_FORMAT_R32G32_SFLOAT: return NX_FORMAT_R32G32_FLOAT;
		case VK_FORMAT_R32G32B32_UINT: return NX_FORMAT_R32G32B32_UINT;
		case VK_FORMAT_R32G32B32_SINT: return NX_FORMAT_R32G32B32_SINT;
		case VK_FORMAT_R32G32B32_SFLOAT: return NX_FORMAT_R32G32B32_FLOAT;
		case VK_FORMAT_R32G32B32A32_UINT: return NX_FORMAT_R32G32B32A32_UINT;
		case VK_FORMAT_R32G32B32A32_SINT: return NX_FORMAT_R32G32B32A32_SINT;
		case VK_FORMAT_R32G32B32A32_SFLOAT: return NX_FORMAT_R32G32B32A32_FLOAT;
		case VK_FORMAT_R64_UINT: return NX_FORMAT_R64_UINT;
		case VK_FORMAT_R64_SINT: return NX_FORMAT_R64_SINT;
		case VK_FORMAT_R64_SFLOAT: return NX_FORMAT_R64_FLOAT;
		case VK_FORMAT_R64G64_UINT: return NX_FORMAT_R64G64_UINT;
		case VK_FORMAT_R64G64_SINT: return NX_FORMAT_R64G64_SINT;
		case VK_FORMAT_R64G64_SFLOAT: return NX_FORMAT_R64G64_FLOAT;
		case VK_FORMAT_R64G64B64_UINT: return NX_FORMAT_R64G64B64_UINT;
		case VK_FORMAT_R64G64B64_SINT: return NX_FORMAT_R64G64B64_SINT;
		case VK_FORMAT_R64G64B64_SFLOAT: return NX_FORMAT_R64G64B64_FLOAT;
		case VK_FORMAT_R64G64B64A64_UINT: return NX_FORMAT_R64G64B64A64_UINT;
		case VK_FORMAT_R64G64B64A64_SINT: return NX_FORMAT_R64G64B64A64_SINT;
		case VK_FORMAT_R64G64B64A64_SFLOAT: return NX_FORMAT_R64G64B64A64_FLOAT;
		default: return NX_FORMAT_UNKNOWN;
	}
}

VkFormat toVkFormat(nxFormat format) {
	switch (format) {
		case NX_FORMAT_R8_UINT: return VK_FORMAT_R8_UINT;
		case NX_FORMAT_R8_SINT: return VK_FORMAT_R8_SINT;
		case NX_FORMAT_R8_UNORM: return VK_FORMAT_R8_UNORM;
		case NX_FORMAT_R8G8_UINT: return VK_FORMAT_R8G8_UINT;
		case NX_FORMAT_R8G8_SINT: return VK_FORMAT_R8G8_SINT;
		case NX_FORMAT_R8G8_UNORM: return VK_FORMAT_R8G8_UNORM;
		case NX_FORMAT_R8G8B8_UINT: return VK_FORMAT_R8G8B8_UINT;
		case NX_FORMAT_R8G8B8_SINT: return VK_FORMAT_R8G8B8_SINT;
		case NX_FORMAT_R8G8B8_UNORM: return VK_FORMAT_R8G8B8_UNORM;
		case NX_FORMAT_R8G8B8A8_UINT: return VK_FORMAT_R8G8B8A8_UINT;
		case NX_FORMAT_R8G8B8A8_SINT: return VK_FORMAT_R8G8B8A8_SINT;
		case NX_FORMAT_R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
		case NX_FORMAT_R16_UINT: return VK_FORMAT_R16_UINT;
		case NX_FORMAT_R16_SINT: return VK_FORMAT_R16_SINT;
		case NX_FORMAT_R16_UNORM: return VK_FORMAT_R16_UNORM;
		case NX_FORMAT_R16G16_UINT: return VK_FORMAT_R16G16_UINT;
		case NX_FORMAT_R16G16_SINT: return VK_FORMAT_R16G16_SINT;
		case NX_FORMAT_R16G16_UNORM: return VK_FORMAT_R16G16_UNORM;
		case NX_FORMAT_R16G16B16_UINT: return VK_FORMAT_R16G16B16_UINT;
		case NX_FORMAT_R16G16B16_SINT: return VK_FORMAT_R16G16B16_SINT;
		case NX_FORMAT_R16G16B16_UNORM: return VK_FORMAT_R16G16B16_UNORM;
		case NX_FORMAT_R16G16B16A16_UINT: return VK_FORMAT_R16G16B16A16_UINT;
		case NX_FORMAT_R16G16B16A16_SINT: return VK_FORMAT_R16G16B16A16_SINT;
		case NX_FORMAT_R16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
		case NX_FORMAT_R32_UINT: return VK_FORMAT_R32_UINT;
		case NX_FORMAT_R32_SINT: return VK_FORMAT_R32_SINT;
		case NX_FORMAT_R32_FLOAT: return VK_FORMAT_R32_SFLOAT;
		case NX_FORMAT_R32G32_UINT: return VK_FORMAT_R32G32_UINT;
		case NX_FORMAT_R32G32_SINT: return VK_FORMAT_R32G32_SINT;
		case NX_FORMAT_R32G32_FLOAT: return VK_FORMAT_R32G32_SFLOAT;
		case NX_FORMAT_R32G32B32_UINT: return VK_FORMAT_R32G32B32_UINT;
		case NX_FORMAT_R32G32B32_SINT: return VK_FORMAT_R32G32B32_SINT;
		case NX_FORMAT_R32G32B32_FLOAT: return VK_FORMAT_R32G32B32_SFLOAT;
		case NX_FORMAT_R32G32B32A32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
		case NX_FORMAT_R32G32B32A32_SINT: return VK_FORMAT_R32G32B32A32_SINT;
		case NX_FORMAT_R32G32B32A32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case NX_FORMAT_R64_UINT: return VK_FORMAT_R64_UINT;
		case NX_FORMAT_R64_SINT: return VK_FORMAT_R64_SINT;
		case NX_FORMAT_R64_FLOAT: return VK_FORMAT_R64_SFLOAT;
		case NX_FORMAT_R64G64_UINT: return VK_FORMAT_R64G64_UINT;
		case NX_FORMAT_R64G64_SINT: return VK_FORMAT_R64G64_SINT;
		case NX_FORMAT_R64G64_FLOAT: return VK_FORMAT_R64G64_SFLOAT;
		case NX_FORMAT_R64G64B64_UINT: return VK_FORMAT_R64G64B64_UINT;
		case NX_FORMAT_R64G64B64_SINT: return VK_FORMAT_R64G64B64_SINT;
		case NX_FORMAT_R64G64B64_FLOAT: return VK_FORMAT_R64G64B64_SFLOAT;
		case NX_FORMAT_R64G64B64A64_UINT: return VK_FORMAT_R64G64B64A64_UINT;
		case NX_FORMAT_R64G64B64A64_SINT: return VK_FORMAT_R64G64B64A64_SINT;
		case NX_FORMAT_R64G64B64A64_FLOAT: return VK_FORMAT_R64G64B64A64_SFLOAT;
		default: return VK_FORMAT_UNDEFINED;
	}
}

nxGraphicsContext nxCreateGraphicsContext() {
	nxGraphicsContext context = NX_INVALID_HANDLE;
	for (uint32_t i = 0; i < NX_MAX_GRAPHICS_CONTEXTS; i++) {
		if (gGraphicsContexts[i].used) continue;
		gGraphicsContexts[i].used = NX_TRUE;
		context = i + NX_GRAPHICS_CONTEXT_OFFSET;
		break;
	}

	NX_ASSERT(context != NX_INVALID_HANDLE);

	VkApplicationInfo applicationInfo = { };
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = "nexusAppilcation";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "nexusEngine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_3;

	uint32_t availableExtensionCount = 0;
	NX_VKCALL(vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, NULL));

	VkExtensionProperties *availableExtensions = (VkExtensionProperties*)malloc(availableExtensionCount * sizeof(VkExtensionProperties));
	NX_VKCALL(vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, availableExtensions));

	const char *requiredExtensions[NX_REQUIRED_INSTANCE_EXTENSION_COUNT] = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	for (uint32_t i = 0; i < NX_REQUIRED_INSTANCE_EXTENSION_COUNT; i++) {
		uint8_t found = NX_FALSE;
		const char *requiredExtension = requiredExtensions[i];
		for (uint32_t j = 0; j < availableExtensionCount; j++) {
			VkExtensionProperties extension = availableExtensions[j];
			if (strcmp(requiredExtension, extension.extensionName)) {
				found = NX_TRUE;
				break;
			}
		}

		if (!found) {
			fprintf(stderr, "Failed to find extension \"%s\"!\n", requiredExtension);
		}
	}

	uint32_t availableLayerCount = 0;
	NX_VKCALL(vkEnumerateInstanceLayerProperties(&availableLayerCount, NULL));

	VkLayerProperties *availableLayers = (VkLayerProperties*)malloc(availableLayerCount * sizeof(VkLayerProperties));
	NX_VKCALL(vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers));

	const char *requiredLayers[NX_REQUIRED_INSTANCE_EXTENSION_COUNT] = {
		"VK_LAYER_KHRONOS_validation"
	};

	for (uint32_t i = 0; i < NX_REQUIRED_INSTANCE_LAYER_COUNT; i++) {
		uint8_t found = NX_FALSE;
		const char *requiredLayer = requiredLayers[i];
		for (uint32_t j = 0; j < availableLayerCount; j++) {
			VkLayerProperties layer = availableLayers[j];
			if (strcmp(requiredLayer, layer.layerName)) {
				found = NX_TRUE;
				break;
			}
		}

		if (!found) {
			fprintf(stderr, "Failed to find layer \"%s\"!\n", requiredLayer);
		}
	}

	VkInstanceCreateInfo instanceInfo = { };
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &applicationInfo;
	instanceInfo.enabledLayerCount = NX_REQUIRED_INSTANCE_LAYER_COUNT;
	instanceInfo.ppEnabledLayerNames = requiredLayers;
	instanceInfo.enabledExtensionCount = NX_REQUIRED_INSTANCE_EXTENSION_COUNT;
	instanceInfo.ppEnabledExtensionNames = requiredExtensions;

	NX_VKCALL(vkCreateInstance(&instanceInfo, NULL, &NX_VK_GRAPHICS_CONTEXT(context).instance));

	VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = { };
	debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugMessengerInfo.pfnUserCallback = onDebugMessage;
	
	debugMessengerInfo.messageSeverity = 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugMessengerInfo.messageType = 
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(NX_VK_GRAPHICS_CONTEXT(context).instance, "vkCreateDebugUtilsMessengerEXT");
	NX_VKCALL(vkCreateDebugUtilsMessengerEXT(NX_VK_GRAPHICS_CONTEXT(context).instance, &debugMessengerInfo, NULL, &NX_VK_GRAPHICS_CONTEXT(context).debugMessenger));

	free(availableExtensions);
	free(availableLayers);

	return context;
}

uint32_t nxGetPhysicalDeviceCount(nxGraphicsContext context) {
	uint32_t physicalDeviceCount = 0;
	NX_VKCALL(vkEnumeratePhysicalDevices(NX_VK_GRAPHICS_CONTEXT(context).instance, &physicalDeviceCount, NULL));
	return physicalDeviceCount;
}

void nxGetPhysicalDevices(nxGraphicsContext context, nxPhysicalDevice *devices) {
	uint32_t physicalDeviceCount = 0;
	NX_VKCALL(vkEnumeratePhysicalDevices(NX_VK_GRAPHICS_CONTEXT(context).instance, &physicalDeviceCount, NULL));

	VkPhysicalDevice *physicalDevices = (VkPhysicalDevice*)malloc(physicalDeviceCount * sizeof(VkPhysicalDevice));
	NX_VKCALL(vkEnumeratePhysicalDevices(NX_VK_GRAPHICS_CONTEXT(context).instance, &physicalDeviceCount, physicalDevices));

	for (uint32_t i = 0; i < physicalDeviceCount; i++) {
		nxPhysicalDevice device = NX_INVALID_HANDLE;
		for (uint32_t j = 0; j < NX_MAX_PHYSICAL_DEVICES; j++) {
			if (gPhysicalDevices[j].used) continue;
			gPhysicalDevices[j].used = NX_TRUE;
			device = j + NX_PHYSICAL_DEVICE_OFFSET;
			break;
		}

		NX_ASSERT(device != NX_INVALID_HANDLE);

		NX_VK_PHYSICAL_DEVICE(device).device = physicalDevices[i];

		vkGetPhysicalDeviceFeatures(NX_VK_PHYSICAL_DEVICE(device).device, &NX_VK_PHYSICAL_DEVICE(device).features);
		vkGetPhysicalDeviceProperties(NX_VK_PHYSICAL_DEVICE(device).device, &NX_VK_PHYSICAL_DEVICE(device).properties);
		vkGetPhysicalDeviceMemoryProperties(NX_VK_PHYSICAL_DEVICE(device).device, &NX_VK_PHYSICAL_DEVICE(device).memoryProperties);

		devices[i] = device;
	}

	free(physicalDevices);
}

void nxDestroyGraphicsContext(nxGraphicsContext *context) {
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(NX_VK_GRAPHICS_CONTEXT(*context).instance, "vkDestroyDebugUtilsMessengerEXT");
	vkDestroyDebugUtilsMessengerEXT(NX_VK_GRAPHICS_CONTEXT(*context).instance, NX_VK_GRAPHICS_CONTEXT(*context).debugMessenger, NULL);

	vkDestroyInstance(NX_VK_GRAPHICS_CONTEXT(*context).instance, NULL);
	memset(&NX_VK_GRAPHICS_CONTEXT(*context), 0, sizeof(vulkanGraphicsContext));
	*context = NX_INVALID_HANDLE;
}

void nxGetPhysicalDeviceName(nxPhysicalDevice device, char *buffer) {
	memcpy(buffer, NX_VK_PHYSICAL_DEVICE(device).properties.deviceName, strlen(NX_VK_PHYSICAL_DEVICE(device).properties.deviceName));
}

uint32_t nxGetPhysicalDeviceID(nxPhysicalDevice device) {
	return NX_VK_PHYSICAL_DEVICE(device).properties.deviceID;
}

uint32_t nxGetPhysicalDeviceVendorID(nxPhysicalDevice device) {
	return NX_VK_PHYSICAL_DEVICE(device).properties.vendorID;
}

nxPhysicalDeviceType nxGetPhysicalDeviceType(nxPhysicalDevice device) {
	switch (NX_VK_PHYSICAL_DEVICE(device).properties.deviceType) {
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return NX_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return NX_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return NX_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		case VK_PHYSICAL_DEVICE_TYPE_CPU: return NX_PHYSICAL_DEVICE_TYPE_CPU;
		default: return NX_PHYSICAL_DEVICE_TYPE_UNKNOWN;
	}
}

nxSurfaceCapabilities nxGetSurfaceCapabilities(nxPhysicalDevice device, nxSurface surface) {
	VkSurfaceCapabilitiesKHR surfaceCapabilities = { };
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(NX_VK_PHYSICAL_DEVICE(device).device, NX_VK_SURFACE(surface).surface, &surfaceCapabilities);

	nxSurfaceCapabilities capabilities = { };
	capabilities.minImageCount = surfaceCapabilities.minImageCount;
	capabilities.maxImageCount = surfaceCapabilities.maxImageCount;
	capabilities.minImageSize.x = surfaceCapabilities.minImageExtent.width;
	capabilities.minImageSize.y = surfaceCapabilities.minImageExtent.height;
	capabilities.maxImageSize.x = surfaceCapabilities.maxImageExtent.width;
	capabilities.maxImageSize.y = surfaceCapabilities.maxImageExtent.height;
	capabilities.maxImageLayers = surfaceCapabilities.maxImageArrayLayers;

	return capabilities;
}

uint32_t nxGetSupportedSurfaceFormatCount(nxPhysicalDevice device, nxSurface surface) {
	uint32_t surfaceFormatCount = 0;
	NX_VKCALL(vkGetPhysicalDeviceSurfaceFormatsKHR(NX_VK_PHYSICAL_DEVICE(device).device, NX_VK_SURFACE(surface).surface, &surfaceFormatCount, NULL));
	return surfaceFormatCount;
}

void nxGetSupportedSurfaceFormats(nxPhysicalDevice device, nxSurface surface, nxFormat *formats) {
	uint32_t surfaceFormatCount = 0;
	NX_VKCALL(vkGetPhysicalDeviceSurfaceFormatsKHR(NX_VK_PHYSICAL_DEVICE(device).device, NX_VK_SURFACE(surface).surface, &surfaceFormatCount, NULL));

	VkSurfaceFormatKHR *surfaceFormats = (VkSurfaceFormatKHR*)malloc(surfaceFormatCount * sizeof(VkSurfaceFormatKHR));
	NX_VKCALL(vkGetPhysicalDeviceSurfaceFormatsKHR(NX_VK_PHYSICAL_DEVICE(device).device, NX_VK_SURFACE(surface).surface, &surfaceFormatCount, surfaceFormats));

	for (uint32_t i = 0; i < surfaceFormatCount; i++) {
		formats[i] = toNxFormat(surfaceFormats[i].format);
	}

	free(surfaceFormats);
}

void nxDestroyPhysicalDevice(nxPhysicalDevice *device) {
	memset(&NX_VK_PHYSICAL_DEVICE(*device), 0, sizeof(vulkanPhysicalDevice));
	*device = NX_INVALID_HANDLE;
}

nxSurface nxCreateSurface(nxGraphicsContext context, nxWindow window) {
	nxSurface surface = NX_INVALID_HANDLE;
	for (uint32_t i = 0; i < NX_MAX_SURFACES; i++) {
		if (gSurfaces[i].used) continue;
		gSurfaces[i].used = NX_TRUE;
		surface = i + NX_SURFACE_OFFSET;
		break;
	}

	NX_ASSERT(surface != NX_INVALID_HANDLE);
#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR surfaceInfo = { };
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.hwnd = (HWND)nxGetWindowNativeHandle(window);
	surfaceInfo.hinstance = GetModuleHandle(NULL);
	
	NX_VKCALL(vkCreateWin32SurfaceKHR(NX_VK_GRAPHICS_CONTEXT(context).instance, &surfaceInfo, NULL, &NX_VK_SURFACE(surface).surface));
	NX_VK_SURFACE(surface).context = context;
#endif // _WIN32

	return surface;
}

void nxDestroySurface(nxSurface *surface) {
	nxGraphicsContext context = NX_VK_SURFACE(*surface).context;
	vkDestroySurfaceKHR(NX_VK_GRAPHICS_CONTEXT(context).instance, NX_VK_SURFACE(*surface).surface, NULL);
	memset(&NX_VK_SURFACE(*surface), 0, sizeof(vulkanSurface));
	*surface = NX_INVALID_HANDLE;
}

nxLogicalDevice nxCreateLogicalDevice(nxPhysicalDevice physicalDevice, nxSurface surface) {
	nxLogicalDevice logicalDevice = NX_INVALID_HANDLE;
	for (uint32_t i = 0; i < NX_MAX_LOGICAL_DEVICES; i++) {
		if (gLogicalDevices[i].used) continue;
		gLogicalDevices[i].used = NX_TRUE;
		logicalDevice = i + NX_LOGICAL_DEVIC_OFFSET;
		break;
	}

	NX_ASSERT(logicalDevice != NX_INVALID_HANDLE);

	NX_VK_LOGICAL_DEVICE(logicalDevice).graphicsQueueFamily = UINT32_MAX;
	NX_VK_LOGICAL_DEVICE(logicalDevice).presentQueueFamily = UINT32_MAX;
	// NX_VK_LOGICAL_DEVICE(logicalDevice).computeQueueFamily = UINT32_MAX;
	NX_VK_LOGICAL_DEVICE(logicalDevice).transferQueueFamily = UINT32_MAX;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(NX_VK_PHYSICAL_DEVICE(physicalDevice).device, &queueFamilyCount, NULL);

	VkQueueFamilyProperties *queueFamilyProperties = (VkQueueFamilyProperties*)malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(NX_VK_PHYSICAL_DEVICE(physicalDevice).device, &queueFamilyCount, queueFamilyProperties);

	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		VkQueueFlags queueFlags = queueFamilyProperties[i].queueFlags;

		if (queueFlags & VK_QUEUE_GRAPHICS_BIT && NX_VK_LOGICAL_DEVICE(logicalDevice).graphicsQueueFamily == UINT32_MAX) {
			NX_VK_LOGICAL_DEVICE(logicalDevice).graphicsQueueFamily = i;
			continue;
		}

		VkBool32 canPresent = VK_FALSE;
		NX_VKCALL(vkGetPhysicalDeviceSurfaceSupportKHR(NX_VK_PHYSICAL_DEVICE(physicalDevice).device, i, NX_VK_SURFACE(surface).surface, &canPresent));
		if (canPresent && NX_VK_LOGICAL_DEVICE(logicalDevice).presentQueueFamily == UINT32_MAX) {
			NX_VK_LOGICAL_DEVICE(logicalDevice).presentQueueFamily = i;
		}

		// TODO: Handle compute queue selection
		// if (queueFlags & VK_QUEUE_COMPUTE_BIT && NX_VK_LOGICAL_DEVICE(logicalDevice).computeQueueFamily == UINT32_MAX) {
		// 	NX_VK_LOGICAL_DEVICE(logicalDevice).computeQueueFamily = i;
		// 	printf("Compute Queue: %u\n", i);
		// 	continue;
		// }

		if (queueFlags & VK_QUEUE_TRANSFER_BIT && NX_VK_LOGICAL_DEVICE(logicalDevice).transferQueueFamily == UINT32_MAX) {
			NX_VK_LOGICAL_DEVICE(logicalDevice).transferQueueFamily = i;
		}
	}	

	uint32_t availableDeviceExtensionCount = 0;
	NX_VKCALL(vkEnumerateDeviceExtensionProperties(NX_VK_PHYSICAL_DEVICE(physicalDevice).device, NULL, &availableDeviceExtensionCount, NULL));

	VkExtensionProperties *availableDeviceExtensions = (VkExtensionProperties*)malloc(availableDeviceExtensionCount * sizeof(VkExtensionProperties));
	NX_VKCALL(vkEnumerateDeviceExtensionProperties(NX_VK_PHYSICAL_DEVICE(physicalDevice).device, NULL, &availableDeviceExtensionCount, availableDeviceExtensions));

	const char *requiredDeviceExtensions[NX_REQUIRED_DEVICE_EXTENSION_COUNT] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	for (uint32_t i = 0; i < NX_REQUIRED_DEVICE_EXTENSION_COUNT; i++) {
		uint8_t found = NX_FALSE;
		const char *requiredDeviceExtension = requiredDeviceExtensions[i];
		for (uint32_t j = 0; j < availableDeviceExtensionCount; j++) {
			VkExtensionProperties extension = availableDeviceExtensions[j];
			if (strcmp(requiredDeviceExtension, extension.extensionName) != 0) continue;
			found = NX_TRUE;
		}

		if (!found) {
			fprintf(stderr, "Failed to find device extension \"%s\"!\n", requiredDeviceExtension);
		}
	}

	float defaultPriority = 1.0f;

	VkDeviceQueueCreateInfo deviceQueueCreateInfos[NX_QUEUE_COUNT] = { };

	deviceQueueCreateInfos[NX_GRAPHICS_QUEUE_INDEX].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfos[NX_GRAPHICS_QUEUE_INDEX].queueFamilyIndex = NX_VK_LOGICAL_DEVICE(logicalDevice).graphicsQueueFamily;
	deviceQueueCreateInfos[NX_GRAPHICS_QUEUE_INDEX].queueCount = 1;
	deviceQueueCreateInfos[NX_GRAPHICS_QUEUE_INDEX].pQueuePriorities = &defaultPriority;

	deviceQueueCreateInfos[NX_PRESENT_QUEUE_INDEX].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfos[NX_PRESENT_QUEUE_INDEX].queueFamilyIndex = NX_VK_LOGICAL_DEVICE(logicalDevice).presentQueueFamily;
	deviceQueueCreateInfos[NX_PRESENT_QUEUE_INDEX].queueCount = 1;
	deviceQueueCreateInfos[NX_PRESENT_QUEUE_INDEX].pQueuePriorities = &defaultPriority;

	deviceQueueCreateInfos[NX_TRANSFER_QUEUE_INDEX].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfos[NX_TRANSFER_QUEUE_INDEX].queueFamilyIndex = NX_VK_LOGICAL_DEVICE(logicalDevice).transferQueueFamily;
	deviceQueueCreateInfos[NX_TRANSFER_QUEUE_INDEX].queueCount = 1;
	deviceQueueCreateInfos[NX_TRANSFER_QUEUE_INDEX].pQueuePriorities = &defaultPriority;

	// TODO: Handle compute queue creation
	// deviceQueueCreateInfos[NX_COMPUTE_QUEUE_INDEX].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	// deviceQueueCreateInfos[NX_COMPUTE_QUEUE_INDEX].queueFamilyIndex = NX_VK_LOGICAL_DEVICE(logicalDevice).computeQueueFamily;
	// deviceQueueCreateInfos[NX_COMPUTE_QUEUE_INDEX].queueCount = 1;
	// deviceQueueCreateInfos[NX_COMPUTE_QUEUE_INDEX].pQueuePriorities = &defaultPriority;

	VkDeviceCreateInfo deviceCreateInfo = { };
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = NX_QUEUE_COUNT;
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos;
	deviceCreateInfo.enabledExtensionCount = NX_REQUIRED_DEVICE_EXTENSION_COUNT;
	deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions;

	NX_VKCALL(vkCreateDevice(NX_VK_PHYSICAL_DEVICE(physicalDevice).device, &deviceCreateInfo, NULL, &NX_VK_LOGICAL_DEVICE(logicalDevice).device));

	vkGetDeviceQueue(NX_VK_LOGICAL_DEVICE(logicalDevice).device, NX_VK_LOGICAL_DEVICE(logicalDevice).graphicsQueueFamily, 0, &NX_VK_LOGICAL_DEVICE(logicalDevice).graphicsQueue);
	vkGetDeviceQueue(NX_VK_LOGICAL_DEVICE(logicalDevice).device, NX_VK_LOGICAL_DEVICE(logicalDevice).presentQueueFamily, 0, &NX_VK_LOGICAL_DEVICE(logicalDevice).presentQueue);
	vkGetDeviceQueue(NX_VK_LOGICAL_DEVICE(logicalDevice).device, NX_VK_LOGICAL_DEVICE(logicalDevice).transferQueueFamily, 0, &NX_VK_LOGICAL_DEVICE(logicalDevice).transferQueue);

	free(queueFamilyProperties);
	free(availableDeviceExtensions);

	return logicalDevice;
}

void nxDestroyLogicalDevice(nxLogicalDevice *device) {
	NX_VKCALL(vkQueueWaitIdle(NX_VK_LOGICAL_DEVICE(*device).graphicsQueue));
	NX_VKCALL(vkQueueWaitIdle(NX_VK_LOGICAL_DEVICE(*device).presentQueue));
	NX_VKCALL(vkQueueWaitIdle(NX_VK_LOGICAL_DEVICE(*device).transferQueue));

	NX_VKCALL(vkDeviceWaitIdle(NX_VK_LOGICAL_DEVICE(*device).device));

	vkDestroyDevice(NX_VK_LOGICAL_DEVICE(*device).device, NULL);
	memset(&NX_VK_LOGICAL_DEVICE(*device), 0, sizeof(vulkanLogicalDevice));
	*device = NX_INVALID_HANDLE;
}

nxSwapChain nxCreateSwapChain(nxLogicalDevice device, nxSurface surface, nxFormat format, nxVec2u imageSize, uint32_t imageCount) {
	nxSwapChain swapChain = NX_INVALID_HANDLE;
	
	for (uint32_t i = 0; i < NX_MAX_SWAP_CHAINS; i++) {
		if (gSwapChains[i].used) continue;
		gSwapChains[i].used = NX_FALSE;
		swapChain = i + NX_SWAP_CHAIN_OFFSET;
		break;
	}

	NX_ASSERT(swapChain != NX_INVALID_HANDLE);

	VkExtent2D imageExtent = { };
	imageExtent.width = imageSize.x;
	imageExtent.height = imageSize.y;

	VkFormat vulkanFormat = toVkFormat(format);

	uint32_t queueFamilyIndices[] = { NX_VK_LOGICAL_DEVICE(device).graphicsQueueFamily, NX_VK_LOGICAL_DEVICE(device).presentQueueFamily, NX_VK_LOGICAL_DEVICE(device).transferQueueFamily };
	
	VkSwapchainCreateInfoKHR swapChainCreateInfo = { };
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = NX_VK_SURFACE(surface).surface;
	swapChainCreateInfo.minImageCount = imageCount;
	swapChainCreateInfo.imageFormat = vulkanFormat;
	swapChainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapChainCreateInfo.imageExtent = imageExtent;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	swapChainCreateInfo.queueFamilyIndexCount = 3;
	swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	swapChainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	swapChainCreateInfo.clipped = VK_TRUE;
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	NX_VK_SWAP_CHAIN(swapChain).imageCount = imageCount;
	NX_VK_SWAP_CHAIN(swapChain).imageSize = imageSize;
	NX_VK_SWAP_CHAIN(swapChain).format = vulkanFormat;
	NX_VK_SWAP_CHAIN(swapChain).device = device;
	NX_VK_SWAP_CHAIN(swapChain).surface = surface;

	NX_VKCALL(vkCreateSwapchainKHR(NX_VK_LOGICAL_DEVICE(device).device, &swapChainCreateInfo, NULL, &NX_VK_SWAP_CHAIN(swapChain).swapChain));

	NX_VKCALL(vkGetSwapchainImagesKHR(NX_VK_LOGICAL_DEVICE(device).device, NX_VK_SWAP_CHAIN(swapChain).swapChain, &NX_VK_SWAP_CHAIN(swapChain).imageCount, NULL));
	
	NX_VK_SWAP_CHAIN(swapChain).images = (VkImage*)malloc(NX_VK_SWAP_CHAIN(swapChain).imageCount * sizeof(VkImage));
	NX_VKCALL(vkGetSwapchainImagesKHR(NX_VK_LOGICAL_DEVICE(device).device, NX_VK_SWAP_CHAIN(swapChain).swapChain, &NX_VK_SWAP_CHAIN(swapChain).imageCount, NX_VK_SWAP_CHAIN(swapChain).images));

	VkSemaphoreCreateInfo semaphoreCreateInfo = { };
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	NX_VKCALL(vkCreateSemaphore(NX_VK_LOGICAL_DEVICE(device).device, &semaphoreCreateInfo, NULL, &NX_VK_SWAP_CHAIN(swapChain).semaphore));

	NX_VKCALL(vkAcquireNextImageKHR(NX_VK_LOGICAL_DEVICE(device).device, NX_VK_SWAP_CHAIN(swapChain).swapChain, UINT64_MAX, NX_VK_SWAP_CHAIN(swapChain).semaphore, VK_NULL_HANDLE, &NX_VK_SWAP_CHAIN(swapChain).nextImageIndex));

	return swapChain;
}

void nxResizeSwapChainImages(nxSwapChain swapChain, nxVec2u newImageSize) {
	nxSurface surface = NX_VK_SWAP_CHAIN(swapChain).surface;
	nxLogicalDevice logicalDevice = NX_VK_SWAP_CHAIN(swapChain).device;

	if (newImageSize.x == NX_VK_SWAP_CHAIN(swapChain).imageSize.x && newImageSize.y == NX_VK_SWAP_CHAIN(swapChain).imageSize.y) return;
	NX_VKCALL(vkQueueWaitIdle(NX_VK_LOGICAL_DEVICE(logicalDevice).presentQueue));
	vkDestroySwapchainKHR(NX_VK_LOGICAL_DEVICE(logicalDevice).device, NX_VK_SWAP_CHAIN(swapChain).swapChain, NULL);
	
	VkExtent2D imageExtent = { };
	imageExtent.width = newImageSize.x;
	imageExtent.height = newImageSize.y;

	uint32_t queueFamilyIndices[] = { NX_VK_LOGICAL_DEVICE(logicalDevice).graphicsQueueFamily, NX_VK_LOGICAL_DEVICE(logicalDevice).presentQueueFamily, NX_VK_LOGICAL_DEVICE(logicalDevice).transferQueueFamily };
	
	VkSwapchainCreateInfoKHR swapChainCreateInfo = { };
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = NX_VK_SURFACE(surface).surface;
	swapChainCreateInfo.minImageCount = NX_VK_SWAP_CHAIN(swapChain).imageCount;
	swapChainCreateInfo.imageFormat = NX_VK_SWAP_CHAIN(swapChain).format;
	swapChainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapChainCreateInfo.imageExtent = imageExtent;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	swapChainCreateInfo.queueFamilyIndexCount = 3;
	swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	swapChainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	swapChainCreateInfo.clipped = VK_TRUE;
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	NX_VKCALL(vkCreateSwapchainKHR(NX_VK_LOGICAL_DEVICE(logicalDevice).device, &swapChainCreateInfo, NULL, &NX_VK_SWAP_CHAIN(swapChain).swapChain));

	NX_VKCALL(vkGetSwapchainImagesKHR(NX_VK_LOGICAL_DEVICE(logicalDevice).device, NX_VK_SWAP_CHAIN(swapChain).swapChain, &NX_VK_SWAP_CHAIN(swapChain).imageCount, NULL));
	
	NX_VK_SWAP_CHAIN(swapChain).images = (VkImage*)malloc(NX_VK_SWAP_CHAIN(swapChain).imageCount * sizeof(VkImage));
	NX_VKCALL(vkGetSwapchainImagesKHR(NX_VK_LOGICAL_DEVICE(logicalDevice).device, NX_VK_SWAP_CHAIN(swapChain).swapChain, &NX_VK_SWAP_CHAIN(swapChain).imageCount, NX_VK_SWAP_CHAIN(swapChain).images));
}

uint32_t nxGetSwapChainImageCount(nxSwapChain swapChain) {
	return NX_VK_SWAP_CHAIN(swapChain).imageCount;
}

void nxGetSwapChainImages(nxSwapChain swapChain, nxImage *images) {
	nxLogicalDevice logicalDevice = NX_VK_SWAP_CHAIN(swapChain).device;

	uint32_t swapChainImageCount = 0;
	NX_VKCALL(vkGetSwapchainImagesKHR(NX_VK_LOGICAL_DEVICE(logicalDevice).device, NX_VK_SWAP_CHAIN(swapChain).swapChain, &swapChainImageCount, NULL));
	
	VkImage *swapChainImages = (VkImage*)malloc(swapChainImageCount * sizeof(VkImage));
	NX_VKCALL(vkGetSwapchainImagesKHR(NX_VK_LOGICAL_DEVICE(logicalDevice).device, NX_VK_SWAP_CHAIN(swapChain).swapChain, &swapChainImageCount, swapChainImages));

	for (uint32_t i = 0; i < swapChainImageCount; i++) {		
		for (uint32_t j = 0; j < NX_MAX_IMAGES; j++) {
			if (gImages[j].used) continue;
			gImages[j].used = NX_TRUE;
			images[i] = j + NX_IMAGE_OFFSET;
			break;
		}

		NX_VK_IMAGE(images[i]).image = swapChainImages[i];
		NX_VK_IMAGE(images[i]).device = logicalDevice;
	}
}

uint32_t nxGetNextSwapChainImageIndex(nxSwapChain swapChain) {
	return NX_VK_SWAP_CHAIN(swapChain).nextImageIndex;
}

void nxPresentSwapChainImage(nxSwapChain swapChain, uint32_t index) {
	nxLogicalDevice logicalDevice = NX_VK_SWAP_CHAIN(swapChain).device;

	VkSemaphore semaphores[] = { NX_VK_SWAP_CHAIN(swapChain).semaphore };

	VkSwapchainKHR swapChains[] = { NX_VK_SWAP_CHAIN(swapChain).swapChain };

	uint32_t imageIndices[] = { index };

	VkResult result;

	VkPresentInfoKHR presentInfo = { };
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = semaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = imageIndices;
	presentInfo.pResults = &result;

	NX_VKCALL(vkQueuePresentKHR(NX_VK_LOGICAL_DEVICE(logicalDevice).presentQueue, &presentInfo));

	NX_VKCALL(result);

	NX_VKCALL(vkAcquireNextImageKHR(NX_VK_LOGICAL_DEVICE(logicalDevice).device, NX_VK_SWAP_CHAIN(swapChain).swapChain, UINT64_MAX, NX_VK_SWAP_CHAIN(swapChain).semaphore, VK_NULL_HANDLE, &NX_VK_SWAP_CHAIN(swapChain).nextImageIndex));
}

void nxDestroySwapChain(nxSwapChain *swapChain) {
	nxLogicalDevice logicalDevice = NX_VK_SWAP_CHAIN(*swapChain).device;

	vkDestroySemaphore(NX_VK_LOGICAL_DEVICE(logicalDevice).device, NX_VK_SWAP_CHAIN(*swapChain).semaphore, NULL);

	vkDestroySwapchainKHR(NX_VK_LOGICAL_DEVICE(logicalDevice).device, NX_VK_SWAP_CHAIN(*swapChain).swapChain, NULL);
	free(NX_VK_SWAP_CHAIN(*swapChain).images);
	memset(&NX_VK_SWAP_CHAIN(*swapChain), 0, sizeof(vulkanSwapChain));
	*swapChain = NX_INVALID_HANDLE;
}

void nxDestroyImage(nxImage *image) {
	nxLogicalDevice logicalDevice = NX_VK_IMAGE(*image).device;

	vkDestroyImage(NX_VK_LOGICAL_DEVICE(logicalDevice).device, NX_VK_IMAGE(*image).image, NULL);
	memset(&NX_VK_IMAGE(*image), 0, sizeof(vulkanImage));
	*image = NX_INVALID_HANDLE;
}

nxImageView nxCreateImageView(nxLogicalDevice device, nxImage image, nxImageViewType type, nxFormat format) {
	nxImageView imageView = NX_INVALID_HANDLE;
	for (uint32_t i = 0; i < NX_MAX_IMAGE_VIEWS; i++) {
		if (gImageViews[i].used) continue;
		gImageViews[i].used = NX_TRUE;
		imageView = i + NX_IMAGE_VIEW_OFFSET;
		break;
	}

	NX_ASSERT(imageView != NX_INVALID_HANDLE);

	VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
	
	switch (type) {
		case NX_IMAGE_VIEW_TYPE_1D: imageViewType = VK_IMAGE_VIEW_TYPE_1D; break;
		case NX_IMAGE_VIEW_TYPE_2D: imageViewType = VK_IMAGE_VIEW_TYPE_2D; break;
		case NX_IMAGE_VIEW_TYPE_3D: imageViewType = VK_IMAGE_VIEW_TYPE_3D; break;
		case NX_IMAGE_VIEW_TYPE_CUBE: imageViewType = VK_IMAGE_VIEW_TYPE_CUBE; break;
		case NX_IMAGE_VIEW_TYPE_1D_ARRAY: imageViewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY; break;
		case NX_IMAGE_VIEW_TYPE_2D_ARRAY: imageViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY; break;
		case NX_IMAGE_VIEW_TYPE_CUBE_ARRAY: imageViewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY; break;
	}

	VkFormat vkFormat = toVkFormat(format);

	VkComponentMapping components = { };
	components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	VkImageSubresourceRange subresourceRange = { };
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;

	VkImageViewCreateInfo imageViewCreateInfo = { };
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = NX_VK_IMAGE(image).image;
	imageViewCreateInfo.viewType = imageViewType;
	imageViewCreateInfo.format = vkFormat;
	imageViewCreateInfo.components = components;
	imageViewCreateInfo.subresourceRange = subresourceRange;

	NX_VKCALL(vkCreateImageView(NX_VK_LOGICAL_DEVICE(device).device, &imageViewCreateInfo, NULL, &NX_VK_IMAGE_VIEW(imageView).imageView));
	NX_VK_IMAGE_VIEW(imageView).device = device;

	return imageView;
}

void nxDestroyImageView(nxImageView *view) {
	nxLogicalDevice logicalDevice = NX_VK_IMAGE_VIEW(*view).device;
	vkDestroyImageView(NX_VK_LOGICAL_DEVICE(logicalDevice).device, NX_VK_IMAGE_VIEW(*view).imageView, NULL);
	memset(&NX_VK_IMAGE_VIEW(*view), 0, sizeof(vulkanImageView));
	*view = NX_INVALID_HANDLE;
}

nxCommandPool nxCreateCommandPool(nxLogicalDevice device, nxCommandPoolType type) {
	nxCommandPool pool = NX_INVALID_HANDLE;
	for (uint32_t i = 0; i < NX_MAX_COMMAND_POOLS; i++) {
		if (gCommandPools[i].used) continue;
		gCommandPools[i].used = NX_TRUE;
		pool = i + NX_COMMAND_POOL_OFFSET;
		break;
	}

	NX_ASSERT(pool != NX_INVALID_HANDLE);

	VkCommandPoolCreateInfo commandPoolCreateInfo = { };
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	switch (type) {
		case NX_COMMAND_POOL_TYPE_GRAPHICS: commandPoolCreateInfo.queueFamilyIndex = NX_VK_LOGICAL_DEVICE(device).graphicsQueueFamily; break;
		case NX_COMMAND_POOL_TYPE_PRESENT: commandPoolCreateInfo.queueFamilyIndex = NX_VK_LOGICAL_DEVICE(device).presentQueueFamily; break;
		case NX_COMMAND_POOL_TYPE_TRANSFER: commandPoolCreateInfo.queueFamilyIndex = NX_VK_LOGICAL_DEVICE(device).transferQueueFamily; break;
	}

	NX_VKCALL(vkCreateCommandPool(NX_VK_LOGICAL_DEVICE(device).device, &commandPoolCreateInfo, NULL, &NX_VK_COMMAND_POOL(pool).pool));
	NX_VK_COMMAND_POOL(pool).device = device;
	return pool;
}

void nxDestroyCommandPool(nxCommandPool *pool) {
	nxLogicalDevice device = NX_VK_COMMAND_POOL(*pool).device;

	vkDestroyCommandPool(NX_VK_LOGICAL_DEVICE(device).device, NX_VK_COMMAND_POOL(*pool).pool, NULL);
	memset(&NX_VK_COMMAND_POOL(*pool), 0, sizeof(vulkanCommandPool));
	*pool = NX_INVALID_HANDLE;
}

nxCommandBuffer nxAllocateCommandBuffer(nxCommandPool pool) {
	nxLogicalDevice logicalDevice = NX_VK_COMMAND_POOL(pool).device;

	nxCommandBuffer buffer = NX_INVALID_HANDLE;
	for (uint32_t i = 0; i < NX_MAX_COMMAND_BUFFERS; i++) {
		if (gCommandBuffers[i].used) continue;
		gCommandBuffers[i].used = NX_TRUE;
		buffer = i + NX_COMMAND_BUFFER_OFFSET;
		break;
	}

	NX_ASSERT(buffer != NX_INVALID_HANDLE);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = { };
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandBufferCount = 1;
	commandBufferAllocateInfo.commandPool = NX_VK_COMMAND_POOL(pool).pool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	NX_VKCALL(vkAllocateCommandBuffers(NX_VK_LOGICAL_DEVICE(logicalDevice).device, &commandBufferAllocateInfo, &NX_VK_COMMAND_BUFFER(buffer).buffer));
	NX_VK_COMMAND_BUFFER(buffer).pool = pool;
	return buffer;
}

void nxFreeCommandBuffer(nxCommandBuffer *buffer) {
	nxCommandPool commandPool = NX_VK_COMMAND_BUFFER(*buffer).pool;
	nxLogicalDevice logicalDevice = NX_VK_COMMAND_POOL(commandPool).device;

	vkFreeCommandBuffers(NX_VK_LOGICAL_DEVICE(logicalDevice).device, NX_VK_COMMAND_POOL(commandPool).pool, 1, &NX_VK_COMMAND_BUFFER(*buffer).buffer);
	memset(&NX_VK_COMMAND_BUFFER(*buffer), 0, sizeof(vulkanCommandBuffer));
	*buffer = NX_INVALID_HANDLE;
}