# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

cmake_minimum_required (VERSION 3.2)
cmake_policy(SET CMP0012 NEW)
cmake_policy(SET CMP0042 NEW)
#if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.9.0")
#   cmake_policy(SET CMP0068 NEW)
#endif()

project (Celix C CXX)

include(cmake/celix_project/CelixProject.cmake)
include(cmake/cmake_celix/UseCelix.cmake)

# see https://public.kitware.com/Bug/view.php?id=15696
IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} EQUAL 3.3 AND ${CMAKE_GENERATOR} STREQUAL "Unix Makefiles")
    message( FATAL_ERROR "Building Celix using CMake 3.3 and makefiles is not supported due to a bug in the Makefile Generator (see Bug 15696). Please change the used CMake version - both, CMake 3.2 and CMake 3.4 are working fine. Or use a different generator (e.g. Ninja)." )
ENDIF()

set(ENABLE_W_ERROR ON)
set(ENABLE_MORE_WARNINGS OFF)

IF (ANDROID)
    set(CMAKE_C_FLAGS "-D_GNU_SOURCE -std=gnu99 -Wall ${CMAKE_C_FLAGS}")
ELSE ()
    set(CMAKE_C_FLAGS "-D_GNU_SOURCE -std=gnu99 -fPIC ${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "-std=c++11 -fno-rtti ${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "-Wall -Werror ${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Weffc++ ${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG ${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG ${CMAKE_CXX_FLAGS}")
ENDIF()

IF(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
ELSE ()
    set(CMAKE_C_FLAGS "-pthread ${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "-pthread ${CMAKE_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "-pthread ${CMAKE_EXE_LINKER_FLAGS}")
ENDIF()

if (ENABLE_MORE_WARNINGS)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4)
            set(CMAKE_CXX_EXTRA_FLAGS "-Wlogical-op -Wold-style-cast -Wuseless-cast -Wdouble-promotion -Wshadow -Wformat=2")
        endif()
        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6)
            set(CMAKE_CXX_EXTRA_FLAGS "-Wduplicated-cond -Wnull-dereference ${CMAKE_CXX_EXTRA_FLAGS}")
        endif()
        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7)
            set(CMAKE_CXX_EXTRA_FLAGS "-Wrestrict -Wduplicated-branches ${CMAKE_CXX_EXTRA_FLAGS}")
        endif()
    endif()

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.5)
            set(CMAKE_CXX_EXTRA_FLAGS "-Wlogical-op -Wold-style-cast -Wshadow -Wformat=2 -Wnull-dereference")
        endif()
        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4)
            set(CMAKE_CXX_EXTRA_FLAGS "-Wold-style-cast -Wuseless-cast ${CMAKE_CXX_EXTRA_FLAGS}")
        endif()
    endif()
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_EXTRA_FLAGS} ${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "-Werror ${CMAKE_CXX_EXTRA_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
endif()

if(ENABLE_W_ERROR)
    set(CMAKE_CXX_FLAGS "-Werror ${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "-Werror ${CMAKE_CXX_FLAGS_DEBUG}")
endif()

# Set version for the framework package/release
set(CELIX_MAJOR "2")
set(CELIX_MINOR "2")
set(CELIX_MICRO "0")

option(ENABLE_TESTING "Enables unit/bundle testing" FALSE)

if (ENABLE_TESTING)
    enable_testing()
endif()

# Default bundle version
set(DEFAULT_VERSION 1.0.0)

option(CELIX_ADD_OPENSSL_DEP "Enabled building Celix framework & etcdlib with OpenSSL library dependency. This can help in some libcurl resolving issues" OFF)
set(CELIX_OPTIONAL_EXTRA_LIBS "")
if (CELIX_ADD_OPENSSL_DEP)
    find_package(OpenSSL REQUIRED)
    set(CELIX_OPTIONAL_EXTRA_LIBS "OpenSSL::lib")
endif ()

#Libraries and Launcher
add_subdirectory(libs)

#Bundles
add_subdirectory(bundles)

#Experimental Bundles/Libraries
add_subdirectory(misc/experimental)

#Example as last, because some example will check if underlining options are enabled
add_subdirectory(examples/celix-examples examples)

#export targets
install(EXPORT celix NAMESPACE Celix:: DESTINATION share/celix/cmake FILE Targets.cmake COMPONENT cmake)
install_celix_targets(celix NAMESPACE Celix:: DESTINATION share/celix/cmake FILE CelixTargets.cmake COMPONENT cmake)

#install celix cmake modules
install(DIRECTORY ${CMAKE_SOURCE_DIR}/cmake/Modules/ DESTINATION share/celix/cmake/Modules)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/cmake/cmake_celix/ DESTINATION share/celix/cmake/cmake_celix)

#configure and install CelixConfig and CelixConfigVersion files
configure_file(cmake/CelixConfigVersion.cmake.in
        "${PROJECT_BINARY_DIR}/celix/gen/cmake/CelixConfigVersion.cmake" @ONLY)

install(FILES
        "cmake/CelixConfig.cmake"
        "${PROJECT_BINARY_DIR}/celix/gen/cmake/CelixConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Celix" COMPONENT cmake)
