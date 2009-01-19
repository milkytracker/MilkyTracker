noinst_LIBRARIES = libmilkyplay.a

libmilkyplay_a_SOURCES = AudioDriverBase.cpp AudioDriverManager.cpp \
AudioDriver_NULL.cpp AudioDriver_WAVWriter.cpp ChannelMixer.cpp ExporterXM.cpp \
LittleEndian.cpp Loader669.cpp LoaderAMF.cpp LoaderAMS.cpp LoaderCBA.cpp \
LoaderDBM.cpp LoaderDIGI.cpp LoaderDSM.cpp LoaderDTM.cpp LoaderFAR.cpp \
LoaderGDM.cpp LoaderIMF.cpp LoaderIT.cpp LoaderMDL.cpp LoaderMOD.cpp \
LoaderMTM.cpp LoaderMXM.cpp LoaderOKT.cpp LoaderPLM.cpp LoaderPSM.cpp \
LoaderPTM.cpp LoaderS3M.cpp LoaderSTM.cpp LoaderULT.cpp LoaderUNI.cpp \
LoaderXM.cpp MasterMixer.cpp PlayerBase.cpp PlayerGeneric.cpp PlayerSTD.cpp \
ResamplerFactory.cpp SampleLoaderAIFF.cpp SampleLoaderALL.cpp \
SampleLoaderAbstract.cpp SampleLoaderGeneric.cpp SampleLoaderIFF.cpp \
SampleLoaderWAV.cpp XIInstrument.cpp XMFile.cpp XModule.cpp \
drivers/alsa/AudioDriver_ALSA.cpp drivers/jack/AudioDriver_JACK.cpp \
drivers/sdl/AudioDriver_SDL.cpp

noinst_HEADERS = AudioDriverBase.h AudioDriverManager.h \
AudioDriver_COMPENSATE.h AudioDriver_NULL.h AudioDriver_WAVWriter.h \
ChannelMixer.h LittleEndian.h Loaders.h MasterMixer.h MilkyPlay.h \
MilkyPlayCommon.h MilkyPlayTypes.h Mixable.h PlayerBase.h PlayerGeneric.h \
PlayerSTD.h ResamplerAmiga.h ResamplerCubic.h ResamplerFactory.h \
ResamplerFast.h ResamplerMacros.h ResamplerSinc.h SampleLoaderAIFF.h \
SampleLoaderALL.h SampleLoaderAbstract.h SampleLoaderGeneric.h \
SampleLoaderIFF.h SampleLoaderWAV.h XIInstrument.h XMFile.h XModule.h \
computed-blep.h drivers/alsa/AudioDriver_ALSA.h \
drivers/jack/AudioDriver_JACK.h drivers/sdl/AudioDriver_SDL.h

INCLUDES = -I$(top_srcdir)/src/milkyplay \
-I$(top_srcdir)/src/milkyplay/drivers/generic \
-I$(top_srcdir)/src/milkyplay/drivers/jack \
-I$(top_srcdir)/src/milkyplay/drivers/sdl

AM_CPPFLAGS=-DMILKYTRACKER -D__THREADTIMER__ -DDRIVER_UNIX ${SDL_CFLAGS}
 \
EXTRA_DIST = PlayerFAR.cpp PlayerFAR.h PlayerIT.cpp PlayerIT.h \
drivers/generic/AudioDriver_PORTAUDIO.cpp \
drivers/generic/AudioDriver_PORTAUDIO.h \
drivers/generic/AudioDriver_RTAUDIO.cpp drivers/generic/AudioDriver_RTAUDIO.h \
drivers/generic/RtAudio3Impl.cpp drivers/generic/RtAudio4Impl.cpp \
drivers/generic/portaudio/portaudio.h drivers/generic/rtaudio/RtAudio3.cpp \
drivers/generic/rtaudio/RtAudio3.h drivers/generic/rtaudio/RtAudio4.cpp \
drivers/generic/rtaudio/RtAudio4.h drivers/generic/rtaudio/RtError3.h \
drivers/generic/rtaudio/RtError4.h drivers/generic/rtaudio/asio/asio.cpp \
drivers/generic/rtaudio/asio/asio.h \
drivers/generic/rtaudio/asio/asiodrivers.cpp \
drivers/generic/rtaudio/asio/asiodrivers.h \
drivers/generic/rtaudio/asio/asiodrvr.h \
drivers/generic/rtaudio/asio/asiolist.cpp \
drivers/generic/rtaudio/asio/asiolist.h drivers/generic/rtaudio/asio/asiosys.h \
drivers/generic/rtaudio/asio/ginclude.h \
drivers/generic/rtaudio/asio/iasiodrv.h \
drivers/generic/rtaudio/asio/iasiothiscallresolver.cpp \
drivers/generic/rtaudio/asio/iasiothiscallresolver.h \
drivers/generic/rtaudio/oss/soundcard.h drivers/osx/AudioDriver_COREAUDIO.cpp \
drivers/osx/AudioDriver_COREAUDIO.h drivers/psp/AudioDriver_PSP.cpp \
drivers/psp/AudioDriver_PSP.h drivers/windows/AudioDriver_MMSYSTEM.cpp \
drivers/windows/AudioDriver_MMSYSTEM.h
