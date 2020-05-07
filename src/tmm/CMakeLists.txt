add_library(tmm STATIC
    # Sources
    kiss_fft.c
    tmm_additive.cpp
    tmm_convert.cpp
    tmm_noise.cpp
    tmm.cpp
    
    # Headers
    _kiss_fft_guts.h
    adx.h
    globals.h
    kiss_fft.h
    tmm_structs.h
    tmm.h
)

target_include_directories(tmm
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
