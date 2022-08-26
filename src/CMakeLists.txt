add_library(
    validation
    validation.cpp
)

target_link_libraries(
    validation
    PRIVATE
        glfw
        ${Vulkan_LIBRARY}
)

add_library(
    app
    application.cpp
)

target_link_libraries(
    app
    PRIVATE
        validation
        glfw
        ${Vulkan_LIBRARY}
)

add_executable(
    main
    main.cpp
)

target_link_libraries(
    main
    PRIVATE
        app
        glfw
        ${Vulkan_LIBRARY}
)