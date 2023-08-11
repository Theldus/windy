# MIT License
#
# Copyright (c) 2023 Davidson Francis <davidsondfgl@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.11)

project(weather C)
set(CMAKE_C_STANDARD 99)

add_executable(weather
    main.c
    weather.c
    font.c
    image.c
    deps/cJSON/cJSON.c)

target_compile_options(weather PRIVATE
	-Wall -Wextra)

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/SDL_src)
set(FETCHCONTENT_QUIET FALSE)

# Set the libraries you want to find
include(FindPkgConfig)
set(PKGCONFIG_LIBRARIES sdl3 sdl3-ttf)

# Define the specific GIT_TAG for each library
set(GIT_TAGS
	f2ae00c1ad5194586bacbd7b374422bd1d47c81a  # SDL
	fee14e525f5bd99612b76e2737400d33eddf0595) # SDL_ttf

# Define a mapping between pkg-config names and repository names
set(GIT_REPOS SDL SDL_ttf)

# Link names
set(LNK_NAMES SDL3::SDL3 SDL3_ttf::SDL3_ttf)

# Build static
set(SDL_STATIC TRUE CACHE BOOL "Build a SDL static library (if available)")

# Find and set compiler and linker flags for each library using pkg-config
foreach(INDEX RANGE 1)

	list(GET PKGCONFIG_LIBRARIES ${INDEX} LIBRARY)
	list(GET GIT_REPOS ${INDEX} REPO)
	list(GET GIT_TAGS  ${INDEX} TAG)
	list(GET LNK_NAMES ${INDEX} LINK)

    pkg_check_modules(PKG_${LIBRARY} QUIET IMPORTED_TARGET ${LIBRARY})
    if (NOT PKG_${LIBRARY}_FOUND)
        message(WARNING
        	"${LIBRARY} not found via pkg-config, trying to fetch manually...")

        FetchContent_Declare(
            "av_${LIBRARY}"
            GIT_REPOSITORY https://github.com/libsdl-org/${REPO}.git
            GIT_TAG ${TAG}
            GIT_SHALLOW FALSE
            GIT_PROGRESS TRUE)
        FetchContent_MakeAvailable("av_${LIBRARY}")

     	# Add to our lib list 'normally'
        target_link_libraries(weather PUBLIC ${LINK})
    else()
    	# If found, add to our lib list via pkg-conig
        target_link_libraries(weather PUBLIC PkgConfig::PKG_${LIBRARY})
    endif()
endforeach()

# Math lib
target_link_libraries(weather PUBLIC m)

# Copy assets folder to build folder
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets
	DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

# Copy request script to build folder
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/request.py
	DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
