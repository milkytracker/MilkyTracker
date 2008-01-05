CPP = g++
CPPFLAGS = -g
FILES_1 = "fontcompiler.cpp" \
"../ppui/osinterface/posix/PPSystem_POSIX.cpp" \
"../ppui/osinterface/posix/PPPath_POSIX.cpp" \
"../ppui/osinterface/PPPathFactory.cpp" \
"../milkyplay/XMFile.cpp" 

FILES_2 = "gentinyfont.cpp" \
"../ppui/osinterface/posix/PPSystem_POSIX.cpp" \
"../ppui/osinterface/posix/PPPath_POSIX.cpp" \
"../ppui/osinterface/PPPathFactory.cpp" \
"../milkyplay/XMFile.cpp" 

FILES_3 = "genlargefont.cpp" \
"../ppui/osinterface/posix/PPSystem_POSIX.cpp" \
"../ppui/osinterface/posix/PPPath_POSIX.cpp" \
"../ppui/osinterface/PPPathFactory.cpp" \
"../milkyplay/XMFile.cpp" 

FILES_4 = "convertrawfont.cpp" \
"../ppui/osinterface/posix/PPSystem_POSIX.cpp" \
"../ppui/osinterface/posix/PPPath_POSIX.cpp" \
"../ppui/osinterface/PPPathFactory.cpp" \
"../milkyplay/XMFile.cpp" 

FILES_5 = "archivewriter.cpp" \
"../ppui/osinterface/posix/PPSystem_POSIX.cpp" \
"../ppui/osinterface/posix/PPPath_POSIX.cpp" \
"../ppui/osinterface/PPPathFactory.cpp" \
"../milkyplay/XMFile.cpp" 

INCLUDE = -I. \
-I../ppui \
-I../ppui/osinterface \
-I../ppui/osinterface/posix \
-I../milkyplay

main:
	$(CPP) $(CPPFLAGS) $(INCLUDE) $(FILES_1) -o fontcompiler
	$(CPP) $(CPPFLAGS) $(INCLUDE) $(FILES_2) -o gentinyfont
	$(CPP) $(CPPFLAGS) $(INCLUDE) $(FILES_3) -o genlargefont
	$(CPP) $(CPPFLAGS) $(INCLUDE) $(FILES_4) -o convertrawfont
	$(CPP) $(CPPFLAGS) $(INCLUDE) $(FILES_5) -o archivewriter
