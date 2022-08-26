add_library(
    toolkits
    toolkits.cpp
)

target_link_libraries(
    toolkits
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
        toolkits
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