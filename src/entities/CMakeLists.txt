add_library(oni-core-entities create-entity.cpp manage-hierarchy.cpp tile-world.cpp serialization.cpp client-data-manager.cpp)

target_compile_features(oni-core-entities
        PUBLIC
            cxx_std_17
        )

# TODO: Remove this
target_compile_definitions(oni-core-entities
        PRIVATE
            GLEW_NO_GLU
        )

target_include_directories(oni-core-entities
        PUBLIC
            $<BUILD_INTERFACE:${oni_SOURCE_DIR}/inc>
            $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/entt>
        PRIVATE SYSTEM
            $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/cereal>
            $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/box2d/Box2D>
            $<BUILD_INTERFACE:${oni_SOURCE_DIR}/lib/glew/include>
        )

# include_directories(${oni_SOURCE_DIR}/lib/freetype-gl/include)

target_link_libraries(oni-core-entities
        PRIVATE
            oni-core-graphics
            oni-core-math
)

add_dependencies(oni-core-entities
        oni-core-graphics
        oni-core-math)