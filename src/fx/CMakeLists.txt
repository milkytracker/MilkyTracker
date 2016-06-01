#
#  src/fx/CMakeLists.txt
#
#  Copyright 2016 Dale Whinham
#
#  This file is part of MilkyTracker.
#
#  MilkyTracker is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  MilkyTracker is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with MilkyTracker.  If not, see <http://www.gnu.org/licenses/>.
#

set(SOURCES
    Camera.cpp
    Filter.cpp
    Fire.cpp
    Math3d.cpp
    ParticleBlobs.cpp
    ParticleEmitter.cpp
    ParticleFX.cpp
    ParticleFun.cpp
    ParticleScene.cpp
    Starfield.cpp
    TCBSpline.cpp
    TCBSplineTest.cpp
    Texture.cpp
    TexturedGrid.cpp
    TexturedPlane.cpp
    Twister.cpp
    TwisterFX.cpp
    fpmath.cpp
)

set(HEADERS
    Camera.h
    FXAbstract.h
    FXInterface.h
    Filter.h
    Fire.h
    Math3d.h
    ParticleBlobs.h
    ParticleEmitter.h
    ParticleFX.h
    ParticleFun.h
    ParticleScene.h
    PictureGlow.h
    Starfield.h
    TCBSpline.h
    TCBSplineTest.h
    Texture.h
    TexturedGrid.h
    TexturedPlane.h
    Twister.h
    TwisterFX.h
    fpmath.h
)

include_directories(
    ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/posix
    ${PROJECT_SOURCE_DIR}/src/ppui
)

add_library(fx ${SOURCES} ${HEADERS})
