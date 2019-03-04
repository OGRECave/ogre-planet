cmake_minimum_required(VERSION 3.0)

# specify which version and components you need
find_package(OGRE 1.11 REQUIRED COMPONENTS Bites Overlay)

include_directories(OgrePlanet/include)
file(GLOB SRCS OgrePlanet/src/*cpp)
add_executable(OgrePlanet ${SRCS})
target_link_libraries(OgrePlanet OgreMain OgreOverlay OgreBites OIS)

SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin/")
# copy resource.cfg next to our binaries where OGRE looks for it
file(COPY ${OGRE_CONFIG_DIR}/plugins.cfg DESTINATION ${EXECUTABLE_OUTPUT_PATH})