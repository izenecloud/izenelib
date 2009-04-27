project (izenelib)

cmake_minimum_required (VERSION 2.6)


# force out-of-source build
#if (cmake_binary_dir == cmake_source_dir)
#    message(fatal_error "In source build is not allowed")
#endif (cmake_binary_dir == cmake_source_dir)

#debugging
message( STATUS "source is: ${izenelib_SOURCE_DIR}" )
message( STATUS "bin is: ${izenelib_BINARY_DIR}" )

add_subdirectory( source)

      
