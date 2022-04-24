add_library(videobackends2d
    Software/SoftwareRenderer2D.h Software/SoftwareRenderer2D.cpp
    Software/Composer.cpp
    Software/Text.cpp Software/Extended.cpp
    Software/Object.cpp
    Software/TileDecoder.cpp
)

set_target_properties(videobackends2d PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(videobackends2d PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(videobackends2d common videocommon)
