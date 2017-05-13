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
			cppflags = " " .. toolchain.cppflags
		}
	}
end

newgcctoolchain {
	name = "m68k-amigaos",
	description = "m68k-amigaos to cross-compile windows binaries from *nix",
	prefix = "m68k-amigaos-",
	cppflags = "-m68020 -noixemul -fpermissive -fomit-frame-pointer "
}

if _OPTIONS.platform then
	-- overwrite the native platform with the options::platform
	premake.gcc.platforms['Native'] = premake.gcc.platforms[_OPTIONS.platform]
end

newoption {
	trigger		= "no-64bit",
	description	= "Don't add the 64-bit project configuration."
}

newoption {
	trigger		= "no-static",
	description	= "Don't compile as a static runtime."
}

newoption {
	trigger		= "no-boost",
	description = "Don't compile with boost.thread support."
}

newoption {
	trigger		= "no-upnp",
	description	= "Don't compile with universal plug and play support."
}

solution "milkytracker"
	configurations { "Debug", "Release" }
	platforms { "m68k-amigaos" }
	includedirs { "./", "./src/fx", "./src/tracker", "./src/compression/", "./src/milkyplay", "./src/milkyplay/drivers/generic", "./src/ppui", "./src/ppui/sdl", "./src/ppui/osinterface", "./src/ppui/osinterface/posix", "./src/milkyplay/drivers/jack", "../../src/milkyplay/drivers/sdl", "/opt/m68k-amigaos/include/SDL", "/opt/m68k-amigaos/include" }
	libdirs { "/opt/m68k-amigaos/lib", "/opt/m68k-amigaos/m68k-amigaos/lib" }
	defines { "__AMIGA__", "HAVE_CONFIG_H", "MILKYTRACKER", "__THREADTIMER__", "DRIVER_UNIX" }

	project "milkyplay"
		kind "StaticLib"
		language "C++"
		targetdir("lib/")
		files { "./src/milkyplay/*", "./src/milkyplay/generic/*", "./src/milkyplay/sdl/*"  }
		includedirs { "./src/milkyplay", "./src/milkyplay/drivers/sdl/" }

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetname "milkyplay_d"

		configuration "release"
			defines { "NDEBUG" }
			flags { "Optimize" }
			targetname "milkyplay"
	project "fx"
		kind "StaticLib"
		language "C++"
		targetdir("lib/")
		files { "./src/fx/**",  }

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetname "fx_d"

		configuration "release"
			defines { "NDEBUG" }
			flags { "Optimize" }
			targetname "fx"

	project "compression"
		kind "StaticLib"
		language "C++"
		targetdir("lib/")
		files { "./src/compression/**.c",  }
		includedirs { "./src/compression", "./src/compression/lha", "./src/compression/zlib", "./src/compression/zziplib" }

		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetname "compression_d"

		configuration "release"
			defines { "NDEBUG" }
			flags { "Optimize" }
			targetname "compression"

	project "ppui"
		kind "StaticLib"
		
		language "C++"
		-- buildoptions { "/TP" }
		-- defines { "_CRT_SECURE_NO_WARNINGS" }
		
		targetdir("lib/")
		files { "./src/ppui/*", "./src/ppui/osinterface/*", "./src/ppui/sdl/*" }
		includedirs { "./src/ppui/osinterface/posix", "./src/ppui/", "./src/ppui/osinterface", "./src/ppui/sdl/" }
		configuration "debug"
			defines { "DEBUG" }
			flags { "Symbols" }
			targetname "ppui_d"

		configuration "release"
			defines { "NDEBUG" }
			flags { "Optimize" }
			targetname "ppui"	

	project "milkytracker"
		-- kind "SDLApp"
		kind "WindowedApp"
		language "C++"
		location "projects"
		targetdir "./bin"
		targetname "milkytracker.68k"
		files {  "./src/tracker/*", "./src/tracker/sdl/*" }
		links { "ppui", "milkyplay", "compression", "fx" }
		-- Libraries.
		configuration "Linux"
		
		-- Debug options.
		configuration "Debug"
			defines { "DEBUG" }
			targetsuffix "_d"
			flags { "NoEditAndContinue" }
		
		-- Release options.
		configuration "Release"
			defines { "NDEBUG" }
			flags { "OptimizeSpeed" }
