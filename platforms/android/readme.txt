MilkyTracker port to Android
=============================

This should be compiled with Android 1.6 SDK and NDK.

You'll need to install Eclipse or Ant too.

Change your current directory to platforms/android and start the setup.sh script to load, patch SDL and set some symbolic links

Then go to <android-ndk> dir and execute:
	make APP=milkytracker V=1
Hopefully it will compile the file project/libs/armeabi/libmilkytracker.so

Then you'll have to compile Android .apk package with Java wrapper code for this lib

 Create an application from Eclipse
--------------------------------------

* Add the Android SDK plugin to Eclipse (http://developer.android.com/intl/de/sdk/eclipse-adt.html)
* Create a new Android project "from existing source": set the <milkytracker>/platforms/android/project directory
* Generate an .apk file with help of the Android tools

 Create an application from the command line
--------------------------------------------

* Go to "project" directory and type
	> android update project -p .
	> ant debug
	
* That will create file <milkytracker>/platforms/android/project/bin/MainActivity-debug.apk
* Use "adb install" to test it
 > adb install MainActivity-debug.apk

Run the application
--------------------

Then you can test it by launching MilkyTracker icon from Android applications menu.

It's designed to fit the available space on the screen even if original resolution is 320x240

TBC
