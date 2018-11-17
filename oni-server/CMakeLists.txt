cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(oni-server VERSION 0.1.0 LANGUAGES C CXX)

# TODO: Avoid relative path
set(oni_SOURCE_DIR ${oni-server_SOURCE_DIR}/..)

subdirs(${oni_SOURCE_DIR}/src/network)
subdirs(${oni_SOURCE_DIR}/src/entities)
subdirs(${oni_SOURCE_DIR}/src/gameplay)
subdirs(${oni_SOURCE_DIR}/src/graphics)
subdirs(${oni_SOURCE_DIR}/src/game)
subdirs(${oni_SOURCE_DIR}/src/utils)
subdirs(${oni_SOURCE_DIR}/src/physics)
subdirs(${oni_SOURCE_DIR}/src/math)
subdirs(${oni_SOURCE_DIR}/src/buffers)
subdirs(${oni_SOURCE_DIR}/src/io)

include_directories(${oni_SOURCE_DIR}/inc)
include_directories(${oni-server_SOURCE_DIR}/inc)
include_directories(SYSTEM ${oni_SOURCE_DIR}/lib/enet/include)
include_directories(SYSTEM ${oni_SOURCE_DIR}/lib/cereal)
include_directories(SYSTEM ${oni_SOURCE_DIR}/lib/Box2D)
include_directories(SYSTEM ${oni_SOURCE_DIR}/lib/entt)

subdirs(${oni-server_SOURCE_DIR}/src)

add_executable(oni-server ${oni-server_SOURCE_DIR}/main.cpp)

target_compile_features(oni-server
        PUBLIC
            cxx_std_17
        )

target_link_libraries(oni-server
        oni-server-game
        oni-core-network)

add_dependencies(oni-server oni-server-game oni-core-network)
