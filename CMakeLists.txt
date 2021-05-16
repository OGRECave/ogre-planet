cmake_minimum_required(VERSION 3.0)

# specify which version and components you need
find_package(OGRE REQUIRED COMPONENTS Bites CONFIG)

include_directories(OgrePlanet/include)
file(GLOB SRCS OgrePlanet/src/*cpp)
add_executable(OgrePlanet ${SRCS})
target_link_libraries(OgrePlanet OgreBites)

configure_file(Media/resources.cfg.in ${CMAKE_CURRENT_BINARY_DIR}/resources.cfg @ONLY)