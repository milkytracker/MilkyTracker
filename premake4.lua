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
			cppflags = " " .. toolchain.cppflags,
			ldflags = " " .. toolchain.ldflags
		}
	}
end

newgcctoolchain {
	name = "m68k-amigaos",
	description = "m68k-amigaos to cross-compile amiga.68k binaries from linux",
	prefix = "m68k-amigaos-",
	cppflags = "-m68060 -mhard-float -fomit-frame-pointer -fno-exceptions -fno-rtti -s -noixemul -I/opt/m68k-amigaos/m68k-amigaos/sys-include -I/opt/m68k-amigaos/include -I/opt/m68k-amigaos/include/SDL ",
	ldflags = "-L/opt/m68k-amigaos/lib -L/opt/m68k-amigaos/m68k-amigaos/lib -L/opt/m68k-amigaos/m68k-amigaos/libnix/lib/libnix -noixemul -ldebug -Xlinker --allow-multiple-definition"
}

newgcctoolchain {
	name = "ppc-amigaos",
	description = "ppc-amigaos to cross-compile amiga.ppc binaries from linux",
	prefix = "ppc-amigaos-",
	cppflags = "-Os -mcrt=newlib -fomit-frame-pointer -fno-exceptions -I/opt/ppc-amigaos/ppc-amigaos/sys-include -I/opt/ppc-amigaos/include -I/opt/ppc-amigaos/include/SDL ",
	ldflags = "-mcrt=newlib -L/opt/ppc-amigaos/lib -L/opt/ppc-amigaos/ppc-amigaos/lib -lauto -lunix"
}

newgcctoolchain {
	name = "ppc-macos",
	description = "",
	prefix = "powerpc-apple-macos-",
	cppflags = "-fomit-frame-pointer -fno-exceptions -I/opt/m68k-ppc-macos/toolchain/powerpc-apple-macos/include -I/opt/m68k-ppc-macos/toolchain/powerpc-apple-macos/RIncludes -I/opt/m68k-ppc-macos/toolchain/powerpc-apple-macos/include/SDL -I../include/mac",
	ldflags = "-L/opt/m68k-ppc-macos/toolchain/powerpc-apple-macos/lib"
}

if _OPTIONS.platform then
	-- overwrite the native platform with the options::platform
	premake.gcc.platforms['Native'] = premake.gcc.platforms[_OPTIONS.platform]
end

solution "milkytracker"
	configurations { "Release", "Release-noFPU", "Debug", "ixemul" }
	platforms { "m68k-amigaos", "ppc-amigaos", "ppc-macos" }
	includedirs { "./", "./src/fx", "./src/tracker", "./src/compression/", "./src/milkyplay", "./src/ppui", "./src/ppui/sdl-1.2", "./src/ppui/osinterface", "./src/ppui/osinterface/amiga", "./src/ppui/osinterface/sdl-1.2", "./src/ppui/osinterface/posix", "./src/milkyplay/drivers/jack", "../../src/milkyplay/drivers/sdl", "./src/submodules/zlib", "./include/lhasa" }
	defines { "AMIGA", "__AMIGA__", "HAVE_CONFIG_H", "MILKYTRACKER", "__THREADTIMER__", "DRIVER_UNIX", "__FORCE_SDL_AUDIO__" }

	project "lhasa"
		kind "StaticLib"
		language "C++"
		location "projects"
		targetdir("lib/")
		files { "./src/submodules/lhasa/src/**.c", "./src/submodules/lhasa/lib/**.c" }
		excludes { "./src/submodules/lhasa/lib/bit_stream_reader.c", "./src/submodules/lhasa/lib/lh_new_decoder.c", "./src/submodules/lhasa/lib/pma_common.c", "./src/submodules/lhasa/lib/tree_decode.c" }
		includedirs { "./src/submodules/lhasa", "./src/submodules/lhasa/lib", "./src/submodules/lhasa/lib/public", "./src/submodules/lhasa/src", "./src/submodules/lhasa" }
		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetname "lhasa_d"
			buildoptions ""
		configuration "release"
			defines { "NDEBUG" }
			targetname "lhasa"
			buildoptions "-DHAVE_CONFIG_H -Wimplicit-function-declaration "
		configuration "release-nofpu"
			defines { "NDEBUG" }
			buildoptions " -msoft-float"
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
			buildoptions""
		configuration "release-nofpu"
			defines { "NDEBUG" }
			buildoptions "-msoft-float"
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
		includedirs { "./include/zziplib", "./src/submodules/zlib", "./src/submodules/zziplib", "./src/submodules/zziplib/SDL" }
		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetname "zziplib_d"

		configuration "release"
			defines { "NDEBUG" }
			targetname "zziplib"
			buildoptions " "
		configuration "release-nofpu"
			defines { "NDEBUG" }
			buildoptions "-msoft-float"
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
		files { "./src/milkyplay/*", "./src/milkyplay/generic/*", "./src/milkyplay/sdl-1.2/*", "./src/milkyplay/drivers/*", "./src/milkyplay/drivers/sdl/*", "./src/milkyplay/drivers/generic/sdl/*"  }
		includedirs { "./src/milkyplay", "./src/milkyplay/drivers/sdl" }

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetname "milkyplay_d"

		configuration "release"
			defines { "NDEBUG" }
			targetname "milkyplay"
			buildoptions"-fpermissive"
		configuration "release-nofpu"
			defines { "NDEBUG" }
			buildoptions " -msoft-float"
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
			buildoptions"-fpermissive"
		configuration "release-nofpu"
                        defines { "NDEBUG" }
                        buildoptions " -msoft-float"
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
		includedirs { "./include/zziplib", "./src/compression", "./src/compression/lha", "./src/compression/zlib", "./src/compression/zlib/generic", "./src/compression/zziplib", "./src/compression/zziplib/generic", "./src/submodules/zziplib", "src/submodules/lhasa/lib/public" }

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetname "compression_d"

		configuration "release"
			defines { "NDEBUG" }
			targetname "compression"
			buildoptions"-fpermissive"
		configuration "release-nofpu"
                        defines { "NDEBUG" }
                        buildoptions " -msoft-float"
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
		files { "./src/ppui/*", "./src/ppui/osinterface/*", "./src/ppui/osinterface/amiga/*", "./src/ppui/osinterface/sdl-1.2/*", "./src/ppui/sdl-1.2/*", "./src/ppui/osinterface/posix/*" }
		excludes { "./src/ppui/osinterface/posix/PPMutex.cpp" }
		includedirs { "./src/ppui/osinterface/posix", "./src/ppui/", "./src/ppui/osinterface", "./src/ppui/osinterface/amiga", "./src/ppui/osinterface/sdl-1.2", "./src/ppui/sdl-1.2" }
		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetname "ppui_d"

		configuration "release"
			defines { "NDEBUG" }
			targetname "ppui"	
			buildoptions "-fpermissive"
		configuration "release-nofpu"
                        defines { "NDEBUG" }
                        buildoptions " -msoft-float"
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
		targetname "milkytracker"
		files {  "./src/tracker/*", "./src/tracker/sdl-1.2/*" }
		links { "zlib", "lhasa", "zziplib", "ppui", "milkyplay", "compression", "fx", "SDL" }
		flags { "Symbols" }

		configuration "m68k-amigaos"
			targetextension ".68k"
		configuration "ppc-amigaos"
			targetextension ".os4"
		configuration "ppc-macos"
			targetextension ".app"
		
		-- Debug options.
		configuration "Debug"
			defines { "DEBUG" }
			targetsuffix "_d"
			flags { "NoEditAndContinue" }
		
		-- Release options.
		configuration "Release"
			buildoptions "-fpermissive"
		configuration "Release-noFPU"
                        defines { "NDEBUG" }
                        buildoptions "-msoft-float"
			targetsuffix "-nofpu"
		configuration "ixemul"
			flags { "OptimizeSize" }
			buildoptions "--std=c++98"
			targetsuffix "-ixemul"
