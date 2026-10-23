set(my_class_include_path "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)
file(GLOB source_files "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
set(my_class_source_files ${source_files} PARENT_SCOPE)
