
# The namespace in Java file, with dots replaced with underscores
SDL_JAVA_PACKAGE_PATH := org_milkytracker

# Path to files with application data - they should be downloaded from Internet on first app run inside
# Java sources, or unpacked from resources (TODO)
# Typically /sdcard/alienblaster 
# Or /data/data/de.schwardtnet.alienblaster/files if you're planning to unpack data in application private folder
# Your application will just set current directory there
SDL_CURDIR_PATH := /sdcard/milkytracker

# Android Dev Phone G1 has trackball instead of cursor keys, and 
# sends trackball movement events as rapid KeyDown/KeyUp events,
# this will make Up/Down/Left/Right key up events with X frames delay,
# so if application expects you to press and hold button it will process the event correctly.
SDL_TRACKBALL_KEYUP_DELAY := 4

include $(call all-subdir-makefiles)
