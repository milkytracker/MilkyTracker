add_executable(export-to-wav
    # Sources
    export-to-wav.cpp
    WAVExportParams.cpp
    WAVUtils.cpp
)

target_link_libraries(export-to-wav
    PRIVATE
        trackerlib
)

install(TARGETS export-to-wav
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
) 