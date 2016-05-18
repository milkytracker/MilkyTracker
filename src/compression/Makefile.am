noinst_LIBRARIES = libcompression.a

libcompression_a_SOURCES =  PP20.cpp ZipExtractor.cpp zziplib/MyIO.cpp \
Decompressor.cpp DecompressorUMX.cpp DecompressorPP20.cpp DecompressorZIP.cpp \
DecompressorLHA.cpp DecompressorGZIP.cpp DecompressorLZX.cpp unlzx.cpp

noinst_HEADERS =  PP20.h ZipExtractor.h zziplib/MyIO.h DecompressorLHA.h \
DecompressorUMX.h DecompressorGZIP.h AIFFWriter.h Decompressor.h \
DecompressorPP20.h DecompressorQT.h DecompressorZIP.h DecompressorLZX.h unlzx.h

EXTRA_DIST = DecompressorQT.mm AIFFWriter.m zlib/generic/adler32.cpp \
zlib/generic/compress.cpp zlib/generic/crc32.cpp zlib/generic/crc32.h \
zlib/generic/deflate.cpp zlib/generic/deflate.h zlib/generic/gzio.cpp \
zlib/generic/infback.cpp zlib/generic/inffast.cpp \
zlib/generic/inffast.h zlib/generic/inffixed.h \
zlib/generic/inflate.cpp zlib/generic/inflate.h \
zlib/generic/inftrees.cpp zlib/generic/inftrees.h \
zlib/generic/trees.cpp zlib/generic/trees.h zlib/generic/uncompr.cpp \
zlib/generic/zconf.h zlib/generic/zlib.h zlib/generic/zutil.cpp \
zlib/generic/zutil.h


AM_CPPFLAGS = -I$(top_srcdir)/src/milkyplay -I$(top_srcdir)/src/ppui \
-I$(top_srcdir)/src/ppui/osinterface/posix \
-I$(top_srcdir)/src/compression/zziplib $(LHASA_CFLAGS)

#AM_CPPFLAGS = -I../ppui -I../ppui/osinterface -I../ppui/osinterface/posix
