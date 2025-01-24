add_library(cli
    CLIParser.cpp
)

target_include_directories(cli
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
) 