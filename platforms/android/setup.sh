
if [ -z $ANDROID_NDK_ROOT ]; then
    echo "You must set the ANDROID_NDK_ROOT environment variable"
    exit 1
 else
    echo "ANDROID_NDK_ROOT=$ANDROID_NDK_ROOT";
fi
if [ -z $MILKYTRACKER_ROOT ]; then
    echo "You must set the MILKYTRACKER_ROOT environment variable"
    exit 1
 else
    echo "MILKYTRACKER_ROOT=$ANDROID_NDK_ROOT";
fi

MT_ANDROID_HOME=`pwd`
JNI_SOURCES=$MT_ANDROID_HOME/project/jni
NDK_MT_APP=$ANDROID_NDK_ROOT/apps/milkytracker

mkdir -p tmp || exit 1
cd tmp

if [ ! -d SDL-1.2.14 ]; then
    echo "Downloading SDL 1.2.14 ..."
    svn --quiet export http://svn.libsdl.org/tags/SDL/release-1.2.14 SDL-1.2.14 || exit 1
    echo "Applying Android patch to SDL 1.2.14 ..."
    cd SDL-1.2.14 && patch -p1 < $JNI_SOURCES/patch-sdl || exit 1
    
    echo "Copying SDL-1.2.14 to the target directory : $JNI_SOURCES/sdl"
    cd $MT_ANDROID_HOME
    cp -rf tmp/SDL-1.2.14/* $JNI_SOURCES/sdl
fi

echo "Creating symbolic links"

if [ ! -d $JNI_SOURCES/milkytracker/src ]; then
    ln -s $MILKYTRACKER_ROOT/src $JNI_SOURCES/milkytracker/src || echo 1
    echo "Created link: $MILKYTRACKER_ROOT/src => $JNI_SOURCES/milkytracker/src"
else 
    echo "Directory $JNI_SOURCES/milkytracker/src already exists"
fi

if [ ! -d $NDK_MT_APP ]; then
    ln -s $MILKYTRACKER_ROOT/platforms/android $NDK_MT_APP || exit 1
    echo "Created link: $MILKYTRACKER_ROOT/platforms/android => $NDK_MT_APP"
else
    echo "Directory $NDK_MT_APP already exists" 
fi

