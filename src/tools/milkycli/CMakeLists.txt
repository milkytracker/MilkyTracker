add_executable(milkycli
    milkycli.cpp
)

target_include_directories(milkycli
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/tracker/wav
)

target_link_libraries(milkycli
    PRIVATE
        commandlib
)

install(TARGETS milkycli
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
) 