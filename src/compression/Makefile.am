noinst_LIBRARIES = libcompression.a
libcompression_a_SOURCES =  PP20.cpp ZipExtractor.cpp zziplib/MyIO.cpp Decompressor.cpp DecompressorUMX.cpp DecompressorPP20.cpp lha/unlha.cpp DecompressorZIP.cpp DecompressorLHA.cpp DecompressorGZIP.cpp
noinst_HEADERS =  PP20.h ZipExtractor.h zziplib/MyIO.h DecompressorLHA.h DecompressorUMX.h DecompressorGZIP.h AIFFWriter.h Decompressor.h DecompressorPP20.h DecompressorQT.h lha/lharc.h DecompressorZIP.h
EXTRA_DIST = DecompressorQT.mm AIFFWriter.m maketbl.inl slide.inl larc.inl huf.inl lhext.inl shuf.inl dhuf.inl header.inl extract.inl
INCLUDES = -I$(top_srcdir)/src/milkyplay -I$(top_srcdir)/src/ppui -I$(top_srcdir)/src/ppui/osinterface/posix/ -I$(top_srcdir)/src/compression/zziplib -I$(top_srcdir)/src/compression/zziplib/generic -I$(top_srcdir)/src/compression/zlib/generic -I$(top_srcdir)/src/compression/lha
#INCLUDES = -I../ppui -I../ppui/osinterface -I../ppui/osinterface/posix
