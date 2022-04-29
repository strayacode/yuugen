add_library(videobackends3d
    Software/SoftwareRenderer3D.h Software/SoftwareRenderer3D.cpp
    Software/Interpolator.h
    Software/TextureSampler.cpp
)

set_target_properties(videobackends3d PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(videobackends3d PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(videobackends3d common videocommon)