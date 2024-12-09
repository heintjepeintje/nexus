workspace ""
	configurations { "debug", "release" }
	startproject "MyProject"

project "nexus"
	kind "ConsoleApp"
	language "C"

	objdir "bin/int/%{cfg.buildcfg}"
	targetdir "bin/out/%{cfg.buildcfg}"

	files {
		"src/**.c"
	}

	includedirs {
		os.getenv("VK_SDK_PATH") .. "/Include",
		"src"
	}

	libdirs {
		os.getenv("VK_SDK_PATH") .. "/Lib",
	}

	links {
		"vulkan-1"
	}

	filter "configurations:debug"
		symbols "On"
		defines { "_DEBUG" }

	filter "configurations:release"
		optimize "On"
		defines { "NDEBUG" }