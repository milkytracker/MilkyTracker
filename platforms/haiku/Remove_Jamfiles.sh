#!/bin/bash

MILKY_ROOT="../.."

echo "Removing Jamfiles..."

rm $MILKY_ROOT/Jamfile
rm $MILKY_ROOT/Jamrules
rm $MILKY_ROOT/src/compression/Jamfile
rm $MILKY_ROOT/src/compression/zziplib/generic/Jamfile
rm $MILKY_ROOT/src/fx/Jamfile
rm $MILKY_ROOT/src/midi/Jamfile
rm $MILKY_ROOT/src/milkyplay/Jamfile
rm $MILKY_ROOT/src/ppui/Jamfile
rm $MILKY_ROOT/src/ppui/osinterface/Jamfile
rm $MILKY_ROOT/src/tracker/Jamfile
rm $MILKY_ROOT/src/tracker/haiku/MilkySettings/Jamfile

echo "Done!"
