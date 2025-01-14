add_executable(export-to-wav
    export-to-wav.cpp
)

target_include_directories(export-to-wav
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/tracker
        ${PROJECT_SOURCE_DIR}/src/ppui
)

target_link_libraries(export-to-wav
    trackerlib
)

install(TARGETS export-to-wav
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
) 