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
    cJSON/cJSON.c)

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/SDL_src)
set(FETCHCONTENT_QUIET FALSE)

# Set the libraries you want to find
include(FindPkgConfig)
set(PKGCONFIG_LIBRARIES sdl3 sdl3-image sdl3-ttf)

# Define the specific GIT_TAG for each library
set(GIT_TAG_sdl3       f2ae00c1ad5194586bacbd7b374422bd1d47c81a)
set(GIT_TAG_sdl3-image 6d6857c1833bfc18222373cf256aec11b653d136)
set(GIT_TAG_sdl3-ttf   fee14e525f5bd99612b76e2737400d33eddf0595)

# Define a mapping between pkg-config names and repository names
set(map_sdl3       SDL)
set(map_sdl3-image SDL_image)
set(map_sdl3-ttf   SDL_ttf)

# Define a map between lib names and link libraries names
set(lnk_sdl3       SDL3::SDL3)
set(lnk_sdl3-image SDL3_image::SDL3_image)
set(lnk_sdl3-ttf   SDL3_ttf::SDL3_ttf)

# Find and set compiler and linker flags for each library using pkg-config
foreach(LIBRARY ${PKGCONFIG_LIBRARIES})
    pkg_check_modules(PKG_${LIBRARY} QUIET IMPORTED_TARGET ${LIBRARY})
    if (NOT PKG_${LIBRARY}_FOUND)
        message(WARNING
        	"${LIBRARY} not found via pkg-config, trying to fetch manually...")

        FetchContent_Declare(
            "av_${LIBRARY}"
            GIT_REPOSITORY https://github.com/libsdl-org/${map_${LIBRARY}}.git
            GIT_TAG ${GIT_TAG_${LIBRARY}}
            GIT_SHALLOW FALSE
            GIT_PROGRESS TRUE)
        FetchContent_MakeAvailable("av_${LIBRARY}")

     	# Add to our lib list 'normally'
        target_link_libraries(weather PUBLIC ${lnk_${LIBRARY}})
    else()
    	# If found, add to our lib list via pkg-conig
        target_link_libraries(weather PUBLIC PkgConfig::PKG_${LIBRARY})
    endif()
endforeach()

# Math lib
target_link_libraries(weather PUBLIC m)
