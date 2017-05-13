# GNU Make project makefile autogenerated by Premake
ifndef config
  config=debug_m68k-amigaos
endif

ifndef verbose
  SILENT = @
endif

ifndef CC
  CC = gcc
endif

ifndef CXX
  CXX = g++
endif

ifndef AR
  AR = ar
endif

ifeq ($(config),debug_m68k-amigaos)
  CC         = m68k-amigaos-gcc
  CXX        = m68k-amigaos-g++
  AR         = m68k-amigaos-ar
  OBJDIR     = obj/m68k-amigaos/Debug/fx
  TARGETDIR  = lib
  TARGET     = $(TARGETDIR)/libfx_d.a
  DEFINES   += -D__AMIGA__ -DHAVE_CONFIG_H -DMILKYTRACKER -D__THREADTIMER__ -DDRIVER_UNIX -DDEBUG
  INCLUDES  += -I. -Isrc/fx -Isrc/tracker -Isrc/compression -Isrc/milkyplay -Isrc/milkyplay/drivers/generic -Isrc/ppui -Isrc/ppui/sdl -Isrc/ppui/osinterface -Isrc/ppui/osinterface/posix -Isrc/milkyplay/drivers/jack -I../../src/milkyplay/drivers/sdl -I/opt/m68k-amigaos/include/SDL -I/opt/m68k-amigaos/include
  CPPFLAGS  +=  -m68020 -noixemul -fpermissive -fomit-frame-pointer  $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -L/opt/m68k-amigaos/lib -L/opt/m68k-amigaos/m68k-amigaos/lib
  LIBS      += 
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += 
  LINKCMD    = $(AR) -rcs $(TARGET) $(OBJECTS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),release_m68k-amigaos)
  CC         = m68k-amigaos-gcc
  CXX        = m68k-amigaos-g++
  AR         = m68k-amigaos-ar
  OBJDIR     = obj/m68k-amigaos/Release/fx
  TARGETDIR  = lib
  TARGET     = $(TARGETDIR)/libfx.a
  DEFINES   += -D__AMIGA__ -DHAVE_CONFIG_H -DMILKYTRACKER -D__THREADTIMER__ -DDRIVER_UNIX -DNDEBUG
  INCLUDES  += -I. -Isrc/fx -Isrc/tracker -Isrc/compression -Isrc/milkyplay -Isrc/milkyplay/drivers/generic -Isrc/ppui -Isrc/ppui/sdl -Isrc/ppui/osinterface -Isrc/ppui/osinterface/posix -Isrc/milkyplay/drivers/jack -I../../src/milkyplay/drivers/sdl -I/opt/m68k-amigaos/include/SDL -I/opt/m68k-amigaos/include
  CPPFLAGS  +=  -m68020 -noixemul -fpermissive -fomit-frame-pointer  $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -O2
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -s -L/opt/m68k-amigaos/lib -L/opt/m68k-amigaos/m68k-amigaos/lib
  LIBS      += 
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += 
  LINKCMD    = $(AR) -rcs $(TARGET) $(OBJECTS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

OBJECTS := \
	$(OBJDIR)/TexturedPlane.o \
	$(OBJDIR)/ParticleScene.o \
	$(OBJDIR)/Filter.o \
	$(OBJDIR)/Camera.o \
	$(OBJDIR)/Fire.o \
	$(OBJDIR)/ParticleBlobs.o \
	$(OBJDIR)/Twister.o \
	$(OBJDIR)/TCBSpline.o \
	$(OBJDIR)/Starfield.o \
	$(OBJDIR)/ParticleEmitter.o \
	$(OBJDIR)/Texture.o \
	$(OBJDIR)/ParticleFX.o \
	$(OBJDIR)/fpmath.o \
	$(OBJDIR)/TexturedGrid.o \
	$(OBJDIR)/ParticleFun.o \
	$(OBJDIR)/Math3d.o \
	$(OBJDIR)/TwisterFX.o \
	$(OBJDIR)/TCBSplineTest.o \

RESOURCES := \

SHELLTYPE := msdos
ifeq (,$(ComSpec)$(COMSPEC))
  SHELLTYPE := posix
endif
ifeq (/bin,$(findstring /bin,$(SHELL)))
  SHELLTYPE := posix
endif

.PHONY: clean prebuild prelink

all: $(TARGETDIR) $(OBJDIR) prebuild prelink $(TARGET)
	@:

$(TARGET): $(GCH) $(OBJECTS) $(LDDEPS) $(RESOURCES)
	@echo Linking fx
	$(SILENT) $(LINKCMD)
	$(POSTBUILDCMDS)

$(TARGETDIR):
	@echo Creating $(TARGETDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(TARGETDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(TARGETDIR))
endif

$(OBJDIR):
	@echo Creating $(OBJDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(OBJDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(OBJDIR))
endif

clean:
	@echo Cleaning fx
ifeq (posix,$(SHELLTYPE))
	$(SILENT) rm -f  $(TARGET)
	$(SILENT) rm -rf $(OBJDIR)
else
	$(SILENT) if exist $(subst /,\\,$(TARGET)) del $(subst /,\\,$(TARGET))
	$(SILENT) if exist $(subst /,\\,$(OBJDIR)) rmdir /s /q $(subst /,\\,$(OBJDIR))
endif

prebuild:
	$(PREBUILDCMDS)

prelink:
	$(PRELINKCMDS)

ifneq (,$(PCH))
$(GCH): $(PCH)
	@echo $(notdir $<)
	-$(SILENT) cp $< $(OBJDIR)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
endif

$(OBJDIR)/TexturedPlane.o: src/fx/TexturedPlane.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/ParticleScene.o: src/fx/ParticleScene.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/Filter.o: src/fx/Filter.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/Camera.o: src/fx/Camera.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/Fire.o: src/fx/Fire.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/ParticleBlobs.o: src/fx/ParticleBlobs.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/Twister.o: src/fx/Twister.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/TCBSpline.o: src/fx/TCBSpline.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/Starfield.o: src/fx/Starfield.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/ParticleEmitter.o: src/fx/ParticleEmitter.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/Texture.o: src/fx/Texture.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/ParticleFX.o: src/fx/ParticleFX.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/fpmath.o: src/fx/fpmath.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/TexturedGrid.o: src/fx/TexturedGrid.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/ParticleFun.o: src/fx/ParticleFun.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/Math3d.o: src/fx/Math3d.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/TwisterFX.o: src/fx/TwisterFX.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/TCBSplineTest.o: src/fx/TCBSplineTest.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
