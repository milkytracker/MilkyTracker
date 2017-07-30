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
    TiTAN.nfo
    ChangeLog.html
)

if(${CMAKE_SYSTEM_NAME} MATCHES "BSD")
    set(BSD TRUE)
endif()

if(APPLE)
    set(INSTALL_DEST Documentation)
elseif(BSD)
    set(INSTALL_DEST share/doc/${PROJECT_NAME_LOWER})
elseif(WIN32)
    list(APPEND DOCUMENTS readme_rtaudio)
    set(INSTALL_DEST .)
else()
    list(APPEND DOCUMENTS readme_unix)
    set(INSTALL_DEST ${CMAKE_INSTALL_DOCDIR})
endif()

install(FILES ${DOCUMENTS} DESTINATION ${INSTALL_DEST})
install(FILES ../AUTHORS DESTINATION ${INSTALL_DEST} RENAME Authors.txt)
install(FILES ../COPYING DESTINATION ${INSTALL_DEST} RENAME Copying.txt)
# add_custom_command(OUTPUT ChangeLog.html COMMAND pandoc ARGS "-so ChangeLog.html ../ChangeLog.md")
