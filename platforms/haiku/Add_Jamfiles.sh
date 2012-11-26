#!/bin/bash

MILKY_ROOT="../.."

echo "Copying Jamfiles..."

cp Jamfiles/Jamfile $MILKY_ROOT
cp Jamfiles/Jamrules $MILKY_ROOT
cp Jamfiles/src-compression-Jamfile $MILKY_ROOT/src/compression/Jamfile
cp Jamfiles/src-compression-zzlib-generic-Jamfile $MILKY_ROOT/src/compression/zziplib/generic/Jamfile
cp Jamfiles/src-fx-Jamfile $MILKY_ROOT/src/fx/Jamfile
cp Jamfiles/src-midi-Jamfile $MILKY_ROOT/src/midi/Jamfile
cp Jamfiles/src-milkyplay-Jamfile $MILKY_ROOT/src/milkyplay/Jamfile
cp Jamfiles/src-ppui-Jamfile $MILKY_ROOT/src/ppui/Jamfile
cp Jamfiles/src-ppui-osinterface-Jamfile $MILKY_ROOT/src/ppui/osinterface/Jamfile
cp Jamfiles/src-tracker-Jamfile $MILKY_ROOT/src/tracker/Jamfile
cp Jamfiles/src-tracker-haiku-MilkySettings-Jamfile $MILKY_ROOT/src/tracker/haiku/MilkySettings/Jamfile

echo "Done!"
