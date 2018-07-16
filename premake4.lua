function os.capture(cmd, raw)
  local f = assert(io.popen(cmd, 'r'))
  local s = assert(f:read('*a'))
  f:close()
  if raw then return s end
  s = string.gsub(s, '^%s+', '')
  s = string.gsub(s, '%s+$', '')
  s = string.gsub(s, '[\n\r]+', ' ')
  return s
end

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
	cppflags = "",
	ldflags = ""
}

newgcctoolchain {
	name = "ppc-amigaos",
	description = "ppc-amigaos to cross-compile amiga.ppc binaries from linux",
	prefix = "ppc-amigaos-",
	cppflags = "",
	ldflags = ""
}

newgcctoolchain {
	name = "ppc-morphos",
	description = "ppc-morphos to cross-compile morphos.ppc binaries from linux",
	prefix = "ppc-morphos-",
	cppflags = "",
	ldflags = ""
}

newgcctoolchain {
	name = "i386-aros",
	description = "x86_64-aros to cross-compile aros.x86_64 binaries from linux",
	prefix = "i386-aros-",
	cppflags = "",
	ldflags = ""
}

newgcctoolchain {
	name = "ppc-macos",
	description = "",
	prefix = "powerpc-apple-macos-",
	cppflags = " ",
	ldflags = ""
}

if _OPTIONS.platform then
	-- overwrite the native platform with the options::platform
	premake.gcc.platforms['Native'] = premake.gcc.platforms[_OPTIONS.platform]
end


local m68kprefix = "/opt/m68k-amigaos"
local curpfx  = os.capture("m68k-amigaos-gcc -v 2>&1 | grep prefix", false)
if curpfx ~= "" then
        local e = string.sub(curpfx, string.find(curpfx, "--prefix") + 9)
        local p = string.sub(e, 0, string.find(e, " ") - 1)
	if p ~= "" then
	        print("Using: " .. p .. "/")
		m68kprefix = p
	end
end


solution "milkytracker"
	configurations { "Release", "Release-noFPU", "Debug" }
	platforms { "m68k-amigaos", "ppc-amigaos", "ppc-morphos", "i386-aros", "ppc-macos" }
	includedirs { "./", "./src/fx", "./src/tracker", "./src/compression/", "./src/milkyplay", "./src/ppui", "./src/ppui/sdl-1.2", "./src/ppui/osinterface", "./src/milkyplay/drivers/jack", "../../src/milkyplay/drivers/sdl", "./src/submodules/zlib", "./include/lhasa" }
	defines { "HAVE_CONFIG_H", "MILKYTRACKER", "__THREADTIMER__", "DRIVER_UNIX", "__FORCE_SDL_AUDIO__" }

	configuration "m68k-amigaos"
		buildoptions "-m68040 -mhard-float -O3 -fomit-frame-pointer -fno-exceptions -s -noixemul"
		linkoptions { "-noixemul", "-ldebug", "-Xlinker --allow-multiple-definition" }
		includedirs { m68kprefix .. "/m68k-amigaos/sys-include", m68kprefix .. "/include", m68kprefix .. "/include/SDL", "./src/ppui/osinterface/amiga", "./src/ppui/osinterface/sdl-1.2", "./src/ppui/osinterface/posix" }
		libdirs { m68kprefix .. "/lib", m68kprefix .. "/m68k-amigaos/lib", m68kprefix .. "/m68k-amigaos/libnix/lib/libnix" }
		defines { "AMIGA", "__AMIGA__" }

	configuration "ppc-morphos"
		buildoptions "-O3 -fomit-frame-pointer -fno-exceptions -s -noixemul"
		linkoptions { "-noixemul", "-ldebug" }
		includedirs { "/opt/ppc-morphos/os-include", "/opt/ppc-morphos/include", "/opt/ppc-morphos/include/SDL", "./src/ppui/osinterface/amiga", "./src/ppui/osinterface/sdl-1.2", "./src/ppui/osinterface/posix" }
		libdirs { "/opt/ppc-morphos/lib" }
		defines { "AMIGA", "__AMIGA__", "MORPHOS", "__MORPHOS__", "morphos", "__morphos__" }

	configuration "ppc-amigaos"
		buildoptions "-O3 -mcrt=newlib -fomit-frame-pointer -fno-exceptions"
		linkoptions { "-mcrt=newlib", "-lauto", "-lunix" }
		includedirs { "/opt/ppc-amigaos/ppc-amigaos/sys-include", "/opt/ppc-amigaos/include", "/opt/ppc-amigaos/include/SDL", "./src/ppui/osinterface/amiga", "./src/ppui/osinterface/sdl-1.2", "./src/ppui/osinterface/posix" }
		libdirs { "/opt/ppc-amigaos/lib", "/opt/ppc-amigaos/ppc-amigaos/lib" }
		defines { "AMIGA", "__AMIGA__", "__amigaos4__" }

	configuration "i386-aros"
		buildoptions "-O3 -fomit-frame-pointer -fno-exceptions -static-libstdc++ -s"
		linkoptions { "-lpthread", "-static-libstdc++" }
		includedirs { "/opt/x86-aros/bin/linux-i386/AROS/Extras/Developer/include", "/opt/x86-aros/bin/linux-x86_64/tools/crosstools/lib/gcc/i386-aros/6.3.0/include-fixed", "/opt/aros/sdk/include/SDL", "./src/ppui/osinterface/amiga", "./src/ppui/osinterface/sdl-1.2", "./src/ppui/osinterface/posix" }
		libdirs { "/opt/aros/sdk/lib", "/opt/aros/sdk/x86_64-aros/lib" }
		defines { "AMIGA", "__AMIGA__", "AROS", "aros", "__AROS__", "__aros__" }

	configuration "ppc-macos"
		buildoptions "-fomit-frame-pointer -fno-exceptions"
		linkoptions { "" }
		includedirs { "/opt/m68k-ppc-macos/toolchain/powerpc-apple-macos/include", "/opt/m68k-ppc-macos/toolchain/powerpc-apple-macos/RIncludes", "/opt/m68k-ppc-macos/toolchain/powerpc-apple-macos/include/SDL", "./include/mac", "./src/ppui/osinterface/sdl-1.2", "./src/ppui/osinterface/posix" }
		libdirs { "/opt/m68k-ppc-macos/toolchain/powerpc-apple-macos/lib" }
		defines { "__macos__" }

	project "lhasa"
		kind "StaticLib"
		language "C"
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

	project "bzlib2"
		kind "StaticLib"
		language "C"
		location "projects"
		targetdir("lib/")
		files { "./src/submodules/bzlib2/pkg_src/*.c" }
		includedirs { "./src/submodules/bzlib2/pkg_src" }
		targetname "bz2"

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetsuffix "_d"

		configuration "release"
			defines { "NDEBUG" }
			buildoptions ""

		configuration "release-nofpu"
			defines { "NDEBUG" }
			buildoptions "-msoft-float"

	project "zlib"
		kind "StaticLib"
		language "C"
		location "projects"
		targetdir("lib/")
		files { "./src/submodules/zlib/*.c" }
		includedirs { "./src/submodules/zlib" }
		targetname "z"

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetsuffix "_d"

		configuration "release"
			defines { "NDEBUG" }
			buildoptions ""

		configuration "release-nofpu"
			defines { "NDEBUG" }
			buildoptions "-msoft-float"

	project "zziplib"
		kind "StaticLib"
		language "C"
		location "projects"
		targetdir("lib/")
		files { "./src/submodules/zziplib/**.c" }
		excludes { "./src/submodules/zziplib/SDL/*" }
		includedirs { "./include/zziplib", "./src/submodules/zlib", "./src/submodules/zziplib", "./src/submodules/zziplib/SDL" }
		targetname "zziplib"

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetsuffix "_d"

		configuration "release"
			defines { "NDEBUG" }

		configuration "release-nofpu"
			defines { "NDEBUG" }
			buildoptions "-msoft-float"

	project "milkyplay"
		kind "StaticLib"
		language "C++"
		location "projects"
		targetdir("lib/")
		files { "./src/milkyplay/*", "./src/milkyplay/generic/*", "./src/milkyplay/sdl-1.2/*", "./src/milkyplay/drivers/*", "./src/milkyplay/drivers/sdl/*", "./src/milkyplay/drivers/generic/sdl/*"  }
		includedirs { "./src/milkyplay", "./src/milkyplay/drivers/sdl" }
		buildoptions "-fno-rtti"
		targetname "milkyplay"

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetsuffix "_d"

		configuration "release"
			defines { "NDEBUG" }
			buildoptions"-fpermissive"

		configuration "release-nofpu"
			defines { "NDEBUG" }
			buildoptions " -msoft-float"

	project "fx"
		kind "StaticLib"
		language "C++"
		location "projects"
		targetdir("lib/")
		files { "./src/fx/**" }
		buildoptions "-fno-rtti"
		targetname "fx"

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetsuffix "_d"

		configuration "release"
			defines { "NDEBUG" }
			buildoptions"-fpermissive"

		configuration "release-nofpu"
			defines { "NDEBUG" }
			buildoptions " -msoft-float"

	project "compression"
		kind "StaticLib"
		language "C++"
		location "projects"
		targetdir("lib/")
		files { "./src/compression/**.cpp",  }
		includedirs { "./include/zziplib", "./src/compression", "./src/compression/lha", "./src/compression/zlib", "./src/compression/zlib/generic", "./src/compression/zziplib", "./src/compression/zziplib/generic", "./src/submodules/zziplib", "src/submodules/lhasa/lib/public" }
		buildoptions "-fno-rtti"
		targetname "compression"

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetsuffix "_d"

		configuration "release"
			defines { "NDEBUG" }
			buildoptions"-fpermissive"

		configuration "release-nofpu"
			defines { "NDEBUG" }
			buildoptions " -msoft-float"

	project "ppui"
		kind "StaticLib"
		language "C++"
		location "projects"
		targetdir("lib/")
		files { "./src/ppui/*", "./src/ppui/osinterface/*", "./src/ppui/sdl-1.2/*", "./src/ppui/osinterface/amiga/*", "./src/ppui/osinterface/sdl-1.2/*", "./src/ppui/osinterface/posix/*" }
		excludes { "./src/ppui/osinterface/posix/PPMutex.cpp" }
		includedirs { "./src/ppui/osinterface/posix", "./src/ppui/", "./src/ppui/osinterface", "./src/ppui/sdl-1.2", "./src/ppui/osinterface/amiga", "./src/ppui/osinterface/sdl-1.2", "./src/ppui/osinterface/posix" }
		buildoptions "-fno-rtti"
		targetname "ppui"

		configuration "m68k-amigaos"
			excludes { "./src/ppui/osinterface/posix/PPMutex.cpp" }
		configuration "ppc-amigaos"
			excludes { "./src/ppui/osinterface/posix/PPMutex.cpp" }
		--filter { "configurations:ppc-macos" }
		--	excludes { "./src/ppui/osinterface/posix/PPMutex.cpp", "**/amiga/**" }
		--filter {}

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetsuffix "_d"

		configuration "release"
			defines { "NDEBUG" }
			buildoptions "-fpermissive"

		configuration "release-nofpu"
			defines { "NDEBUG" }
			buildoptions " -msoft-float"

	project "milkytracker"
		kind "WindowedApp"
		language "C++"
		location "projects"
		targetdir "./bin"
		targetname "milkytracker"
		files { "./src/tracker/*", "./src/tracker/sdl-1.2/*" }
		links { "z", "bz2", "lhasa", "zziplib", "ppui", "milkyplay", "compression", "fx", "SDL" }
		flags { "Symbols" }
		buildoptions "-fno-rtti"

		configuration "m68k-amigaos"
			targetextension ".68k"
		configuration "ppc-morphos"
			targetextension ".mos"
			links { "debug" }
		configuration "ppc-amigaos"
			targetextension ".os4"
		configuration "i386-aros"
			targetextension ".aros-abiv1"
			links { "pthread" }
		configuration "ppc-macos"
			targetextension ".app"
		--	links { "SDLmain" }
		
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
