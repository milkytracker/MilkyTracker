#
#  docs/CMakeLists.txt
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

set(
    DOCUMENTS
    ChangeLog.html
    FAQ.html
    MilkyTracker.html
    #readme_PocketPC.html
    TiTAN.nfo
)

if(APPLE)
    list(APPEND DOCUMENTS readme_OSX.html)
    set(INSTALL_DEST Documentation)
elseif(WIN32)
    list(APPEND DOCUMENTS readme_rtaudio)
    set(INSTALL_DEST .)
else()
    list(APPEND DOCUMENTS readme_unix)
    set(INSTALL_DEST share/doc/${PROJECT_NAME_LOWER})
endif()

install(FILES ${DOCUMENTS} DESTINATION ${INSTALL_DEST})
