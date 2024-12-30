add_executable(export-to-wav
    export-to-wav.cpp
    WAVExportParams.cpp
    WAVUtils.cpp
)

target_link_libraries(export-to-wav
    trackerlib
    milkyplay
    compression
    ${LHASA_LIBRARIES}
    ${ZZIPLIB_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

target_include_directories(export-to-wav PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/milkyplay
    ${PROJECT_SOURCE_DIR}/src/ppui
    ${PROJECT_SOURCE_DIR}/src/ppui/osinterface
)

if(WIN32)
    target_include_directories(export-to-wav PRIVATE
        ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/win32
    )
else()
    target_include_directories(export-to-wav PRIVATE
        ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/posix
    )
endif()

install(TARGETS export-to-wav
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
) 