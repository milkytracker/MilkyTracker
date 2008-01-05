#!/bin/bash
cp -r "PPUI OSX 10.3/build/MilkyTracker.app" mt_bin/osx/powerpc
cp -r Documentation/FAQ.html mt_bin/osx/powerpc
cp -r Documentation/ChangeLog.html mt_bin/osx/powerpc
cp -r Documentation/MilkyTracker.html mt_bin/osx/powerpc
cp -r Documentation/readme_OSX.html mt_bin/osx/powerpc
cp -r Documentation/TiTAN.nfo mt_bin/osx/powerpc
cp -r Example\ Music/*.* mt_bin/osx/powerpc
cd mt_bin/osx/powerpc/
tar cfz MilkyTracker_OSX_powerpc_20`date +%y_%m_%d`.tgz *
scp MilkyTracker_OSX_powerpc_20`date +%y_%m_%d`.tgz peter@ssh.nxbone.net:public_html/testing/MacOSX/powerpc
rm MilkyTracker_OSX_powerpc_20`date +%y_%m_%d`.tgz
cd ../../../