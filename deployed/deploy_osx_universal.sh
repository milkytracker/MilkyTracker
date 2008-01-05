#!/bin/bash
cp -r "PPUI_OSX_Universal/build/Release/MilkyTracker.app" mt_bin/osx/universal
cp -r Documentation/FAQ.html mt_bin/osx/universal
cp -r Documentation/ChangeLog.html mt_bin/osx/universal
cp -r Documentation/MilkyTracker.html mt_bin/osx/universal
cp -r Documentation/readme_OSX.html mt_bin/osx/universal
cp -r Documentation/TiTAN.nfo mt_bin/osx/universal
cp -r Example\ Music/*.* mt_bin/osx/universal
cd mt_bin/osx/universal/
tar cfz MilkyTracker_OSX_universal_20`date +%y_%m_%d`.tgz *
scp MilkyTracker_OSX_universal_20`date +%y_%m_%d`.tgz peter@ssh.nxbone.net:public_html/testing/MacOSX/universal
rm MilkyTracker_OSX_universal_20`date +%y_%m_%d`.tgz
cd ../../../