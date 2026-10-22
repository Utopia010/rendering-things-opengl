



# ## add shader files into config dependency in order to update
file(GLOB shader_files "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*")

if(NOT shader_files)
    message(FATAL_ERROR "shader files are missing")
endif()

foreach(item IN ITEMS shader_files)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS item)
endforeach()
