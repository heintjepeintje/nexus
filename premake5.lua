workspace ""
	configurations { "debug", "release" }
	startproject "MyProject"

project "nexus"
	kind "ConsoleApp"
	language "C++"

	objdir "bin/int/%{cfg.buildcfg}"
	targetdir "bin/out/%{cfg.buildcfg}"

	files {
		"src/**.cpp"
	}

	includedirs {
		"src"
	}

	filter "configurations:debug"
		symbols "On"
		defines { "_DEBUG" }

	filter "configurations:release"
		optimize "On"
		defines { "NDEBUG" }