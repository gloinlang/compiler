cmake_minimum_required(VERSION 3.12)
project(gloin VERSION 0.0.1 LANGUAGES C CXX)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${LLVM_INCLUDE_DIRS})

# Add LLVM definitions
separate_arguments(LLVM_DEFINITIONS_LIST UNIX_COMMAND "${LLVM_DEFINITIONS}")
add_definitions(${LLVM_DEFINITIONS_LIST})

# Find required LLVM libraries - use shared library approach like Makefile
llvm_map_components_to_libnames(LLVM_LIBS all)

# Alternative: use the same approach as the Makefile
execute_process(
    COMMAND llvm-config --ldflags --libs core executionengine mcjit interpreter analysis native
    OUTPUT_VARIABLE LLVM_LDFLAGS_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Parse ldflags and libs
string(REGEX REPLACE "^-L([^ ]+).*" "\\1" LLVM_LIBRARY_DIRS ${LLVM_LDFLAGS_LIBS})
string(REGEX REPLACE ".*(-l[^ ]+)" "\\1" LLVM_LIBRARIES ${LLVM_LDFLAGS_LIBS})
string(REPLACE "-l" "" LLVM_LIBRARIES ${LLVM_LIBRARIES})

# Use the found libraries
link_directories(${LLVM_LIBRARY_DIRS})

# Source files
set(GLOIN_SOURCES
    src/ast.c
    src/codegen.c
    src/lexer.c
    src/parser.c
    src/types.c
)

# Header files (for IDE)
set(GLOIN_HEADERS
    include/ast.h
    include/codegen.h
    include/lexer.h
    include/parser.h
    include/types.h
)

# Main executable
add_executable(gloinc
    src/main.c
    ${GLOIN_SOURCES}
)

# Link libraries - use C++ linker for LLVM
set_target_properties(gloinc PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(gloinc ${LLVM_LIBRARIES})

# Create a static library for testing
add_library(gloin_lib STATIC ${GLOIN_SOURCES})
set_target_properties(gloin_lib PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(gloin_lib ${LLVM_LIBRARIES})

# Install targets
install(TARGETS gloinc
    RUNTIME DESTINATION bin
)

# Install examples and documentation
install(DIRECTORY examples/
    DESTINATION share/gloin/examples
    FILES_MATCHING PATTERN "*.gloin"
)

install(FILES 
    README.md 
    LICENSE
    DESTINATION share/doc/gloin
)

# Install headers for development (optional)
install(FILES ${GLOIN_HEADERS}
    DESTINATION include/gloin
)

# Test configuration
enable_testing()

# Note: Syntax tests are disabled as they require specific project setup

# Optional: Add test executable if test files exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/test_main.c")
    # Test files (if they exist)
    file(GLOB TEST_SOURCES "test/*.c")
    
    if(TEST_SOURCES)
        add_executable(test_runner ${TEST_SOURCES})
        target_link_libraries(test_runner gloin_lib)
        
        add_test(NAME unit_tests
            COMMAND test_runner
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    endif()
endif()

# Package configuration
set(CPACK_PACKAGE_NAME "gloin")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Gloin Programming Language Compiler")
set(CPACK_PACKAGE_VENDOR "Gloin Development Team")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_GENERATOR "TGZ;ZIP")

include(CPack)

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  LLVM Version: ${LLVM_PACKAGE_VERSION}")
message(STATUS "")