add_library(oni-core-physics dynamics.cpp car.cpp transformation.cpp collision.cpp)

target_compile_features(oni-core-physics
        PUBLIC
            cxx_std_17
        )

# TODO: Remove this
target_compile_definitions(oni-core-physics
        PRIVATE
            GLEW_NO_GLU
        )

target_include_directories(oni-core-physics
        PUBLIC
            $<BUILD_INTERFACE:${oni_SOURCE_DIR}/inc>
        PRIVATE SYSTEM
            $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/cereal>
            $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/entt>
            $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/box2d/Box2D>
            $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/glfw/include>
            $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/glew/include>
        )

find_library(BOX2D_LIBRARY Box2D PATHS ${oni_SOURCE_DIR}/lib/box2d/Box2D/Build/Box2D)

target_link_libraries(oni-core-physics
        PRIVATE
            ${BOX2D_LIBRARY}
            oni-core-entities
        )

add_dependencies(oni-core-physics
        oni-core-entities
        )
