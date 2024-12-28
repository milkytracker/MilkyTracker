add_executable(export-to-wav
    export-to-wav.cpp
)

target_link_libraries(export-to-wav
    milkyplay
    compression
    ppui
    trackerlib
    z
)

target_include_directories(export-to-wav PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/milkyplay
    ${PROJECT_SOURCE_DIR}/src/ppui
    ${PROJECT_SOURCE_DIR}/src/ppui/osinterface
    ${PROJECT_SOURCE_DIR}/src/ppui/osinterface/posix
)

install(TARGETS export-to-wav
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
) 