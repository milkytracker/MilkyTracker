function newplatform(plf)
	local name = plf.name
	local description = plf.description

	-- Register new platform
	premake.platforms[name] = {
		cfgsuffix = "_"..name,
		iscrosscompiler = true
	}

	-- Allow use of new platform in --platfroms
	table.insert(premake.option.list["platform"].allowed, { name, description })
	table.insert(premake.fields.platforms.allowed, name)

	-- Add compiler support
	premake.gcc.platforms[name] = plf.gcc
end

function newgcctoolchain(toolchain)
	newplatform {
		name = toolchain.name,
		description = toolchain.description,
		gcc = {
			cc = toolchain.prefix .. "gcc",
			cxx = toolchain.prefix .. "g++",
			ar = toolchain.prefix .. "ar",
			ld = toolchain.prefix .. "ld",
			cppflags = " " .. toolchain.cppflags
		}
	}
end

newgcctoolchain {
	name = "m68k-amigaos",
	description = "m68k-amigaos to cross-compile amiga.68k binaries from linux",
	prefix = "m68k-amigaos-",
	cppflags = "-m68040 -fomit-frame-pointer -fno-exceptions -fbbb=sapcmfbi"
}

if _OPTIONS.platform then
	-- overwrite the native platform with the options::platform
	premake.gcc.platforms['Native'] = premake.gcc.platforms[_OPTIONS.platform]
end

solution "milkytracker"
	configurations { "Release", "Debug", "ixemul" }
	platforms { "m68k-amigaos" }
	includedirs { "/opt/m68k-amigaos/m68k-amigaos/sys-include", "./", "./src/fx", "./src/tracker", "./src/compression/", "./src/milkyplay", "./src/ppui", "./src/ppui/sdl-1.2", "./src/ppui/osinterface", "./src/ppui/osinterface/sdl-1.2","./src/ppui/osinterface/posix", "./src/milkyplay/drivers/jack", "../../src/milkyplay/drivers/sdl-1.2", "/opt/m68k-amigaos/include" }
	libdirs { "/opt/m68k-amigaos/lib", "/opt/m68k-amigaos/m68k-amigaos/lib", "/opt/m68k-amigaos/m68k-amigaos/libnix/lib/libnix" }
	defines { "AMIGA", "__AMIGA__", "HAVE_CONFIG_H", "MILKYTRACKER", "__THREADTIMER__", "DRIVER_UNIX", "__FORCE_SDL_AUDIO__" }

	project "lhasa"
		kind "StaticLib"
		language "C++"
		location "projects"
		targetdir("lib/")
		files { "./src/submodules/lhasa/src/**.c", "./src/submodules/lhasa/lib/**.c" }
		excludes { "./src/submodules/lhasa/lib/bit_stream_reader.c", "./src/submodules/lhasa/lib/lh_new_decoder.c", "./src/submodules/lhasa/lib/pma_common.c", "./src/submodules/lhasa/lib/tree_decode.c" }
		includedirs { "./src/submodules/lhasa", "./src/submodules/lhasa/lib", "./src/submodules/lhasa/lib/public", "./src/submodules/lhasa/src", "./src/submodules/lhasa", "./include/lhasa" }
		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetname "lhasa_d"
			buildoptions "-noixemul"
		configuration "release"
			defines { "NDEBUG" }
			targetname "lhasa"
			buildoptions "-g -noixemul -DHAVE_CONFIG_H -Wimplicit-function-declaration "
		configuration "release-nofpu"
			defines { "NDEBUG" }
			buildoptions "-noixemul -msoft-float"
			targetname "lhasa"
		configuration "ixemul"
			defines { "NDEBUG" }
			flags { "OptimizeSize" }
			targetname "lhasa"
			buildoptions "--std=c++98"

	project "zlib"
		kind "StaticLib"
		language "C"
		location "projects"
		targetdir("lib/")
		files { "./src/submodules/zlib/*.c" }
		includedirs { "./src/submodules/zlib" }
		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetname "zlib_d"

		configuration "release"
			defines { "NDEBUG" }
			targetname "zlib"
			buildoptions"-noixemul"
		configuration "release-nofpu"
			defines { "NDEBUG" }
			buildoptions "-noixemul -msoft-float"
			targetname "zlib"
		configuration "ixemul"
			defines { "NDEBUG" }
			flags { "OptimizeSize" }
			targetname "zlib"
			buildoptions "--std=c++98"
	project "zziplib"
		kind "StaticLib"
		language "C"
		location "projects"
		targetdir("lib/")
		files { "./src/submodules/zziplib/**.c" }
		includedirs { "./include/zziplib", "./src/submodules/zlib", "./src/submodules/zziplib", "./src/submodules/zziplib/SDL", "/opt/m68k-amigaos/include/SDL" }
		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetname "zziplib_d"

		configuration "release"
			defines { "NDEBUG" }
			targetname "zziplib"
			buildoptions "-noixemul "
		configuration "release-nofpu"
			defines { "NDEBUG" }
			buildoptions "-g -noixemul -msoft-float"
			targetname "zziplib"
		configuration "ixemul"
			defines { "NDEBUG" }
			flags { "OptimizeSize" }
			targetname "zziplib"
			buildoptions "--std=c++98"
	project "milkyplay"
		kind "StaticLib"
		language "C++"
		location "projects"
		targetdir("lib/")
		files { "./src/milkyplay/*", "./src/milkyplay/generic/*", "./src/milkyplay/sdl/*", "./src/milkyplay/drivers/*", "./src/milkyplay/drivers/sdl/*", "./src/milkyplay/drivers/generic/sdl/*"  }
		includedirs { "./src/milkyplay", "./src/milkyplay/drivers/sdl", "/opt/m68k-amigaos/include/SDL" }

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetname "milkyplay_d"

		configuration "release"
			defines { "NDEBUG" }
			targetname "milkyplay"
			buildoptions"-g -noixemul -fpermissive"
		configuration "release-nofpu"
			defines { "NDEBUG" }
--			flags { "" }
			buildoptions "-noixemul -msoft-float"
			targetname "milkyplay"
		configuration "ixemul"
			defines { "NDEBUG" }
			flags { "OptimizeSize" }
			targetname "milkyplay"
			buildoptions "--std=c++98"

	project "fx"
		kind "StaticLib"
		language "C++"
		location "projects"
		targetdir("lib/")
		files { "./src/fx/**",  }

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetname "fx_d"
		configuration "release"
			defines { "NDEBUG" }
			targetname "fx"
			buildoptions"-g -noixemul -fpermissive"
		configuration "release-nofpu"
                        defines { "NDEBUG" }
--                        flags { "" }
                        buildoptions "-noixemul -msoft-float"
                        targetname "fx"
		configuration "ixemul"
			defines { "NDEBUG" }
			flags { "OptimizeSize" }
			targetname "milkyplay"
			buildoptions "--std=c++98"

	project "compression"
		kind "StaticLib"
		language "C++"
		location "projects"
		targetdir("lib/")
		files { "./src/compression/**.cpp",  }
		includedirs { "./include/zziplib", "./src/compression", "./src/compression/lha", "./src/compression/zlib", "./src/compression/zlib/generic", "./src/compression/zziplib", "./src/compression/zziplib/generic", "./include/lhasa", "./src/submodules/zziplib", "src/submodules/lhasa/lib/public" }

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetname "compression_d"

		configuration "release"
			defines { "NDEBUG" }
			targetname "compression"
			buildoptions"-g -noixemul -fpermissive"
		configuration "release-nofpu"
                        defines { "NDEBUG" }
--                        flags { "" }
                        buildoptions "-noixemul -msoft-float"
                        targetname "compression"
		configuration "ixemul"
			defines { "NDEBUG" }
			flags { "OptimizeSize" }
			targetname "milkyplay"
			buildoptions "--std=c++98"

	project "ppui"
		kind "StaticLib"
		language "C++"
		location "projects"
		targetdir("lib/")
		files { "./src/ppui/*", "./src/ppui/osinterface/*", "./src/ppui/osinterface/sdl-1.2/*", "./src/ppui/sdl-1.2/*", "./src/ppui/osinterface/posix/*" }
		excludes { "./src/ppui/osinterface/posix/PPMutex.cpp" }
		includedirs { "./src/ppui/osinterface/posix", "./src/ppui/", "./src/ppui/osinterface", "./src/ppui/osinterface/sdl-1.2", "./src/ppui/sdl-1.2", "/opt/m68k-amigaos/include/SDL" }
		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetname "ppui_d"

		configuration "release"
			defines { "NDEBUG" }
			targetname "ppui"	
			buildoptions "-g -noixemul -fpermissive"
		configuration "release-nofpu"
                        defines { "NDEBUG" }
                        buildoptions "-noixemul -msoft-float"
                        targetname "ppui"
		configuration "ixemul"
			defines { "NDEBUG" }
			flags { "OptimizeSize" }
			targetname "milkyplay"
			buildoptions "--std=c++98"

	project "milkytracker"
		kind "WindowedApp"
		language "C++"
		location "projects"
		targetdir "./bin"
		targetname "milkytracker.68k"
		files {  "./src/tracker/*", "./src/tracker/sdl-1.2/*" }
		links { "zlib", "lhasa", "zziplib", "ppui", "milkyplay", "compression", "fx", "SDL", "jpeg", "z", "debug" }
		linkoptions { "-D__AMIGA__ -fno-rtti -fno-exceptions -fpermissive -fbbb=sapcmfbi -noixemul -m68040 -msoft-float -I/opt/m68k-amigaos/include/SDL -I/opt/m68k-amigaos/include -L/opt/m68k-amigaos/lib -L/opt/m68k-amigaos/m68k-amigaos/lib -fomit-frame-pointer -Xlinker --allow-multiple-definition" }
		includedirs { "/opt/m68k-amigaos/include/SDL" }

		-- Libraries.
		configuration "Linux"
		
		-- Debug options.
		configuration "Debug"
			defines { "DEBUG" }
			targetsuffix "_d"
			flags { "NoEditAndContinue" }
		
		-- Release options.
		configuration "Release"
			buildoptions "-g -noixemul -fpermissive"
		configuration "Release-noFPU"
                        defines { "NDEBUG" }
--                        flags { "" }
                        buildoptions "-noixemul -msoft-float"
			targetsuffix "-nofpu"
		configuration "ixemul"
			flags { "OptimizeSize" }
			buildoptions "--std=c++98"
			targetsuffix "-ixemul"
