# CMakeList.txt : CMake project for OpenBus.Vulkan, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

find_package (Vulkan REQUIRED FATAL_ERROR)
find_package (Qt5Core REQUIRED)
find_package (Qt5Gui REQUIRED)
find_package (Qt5Widgets REQUIRED)
find_package (SFML 2.5 COMPONENTS window graphics REQUIRED)

file (GLOB_RECURSE HEADER_FILES *.h)
file (GLOB_RECURSE SOURCE_FILES *.cpp)
add_executable (OpenBus WIN32 ${HEADER_FILES} ${SOURCE_FILES})

target_compile_definitions(OpenBus PUBLIC "APP_VERSION=\"v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${GIT_SHA1}\"")

target_include_directories (OpenBus
    PUBLIC "${LIBRARY_DIR}/assimp/include"
    PUBLIC "${LIBRARY_DIR}/bullet/include"
    PUBLIC "${LIBRARY_DIR}/glfw/include"
    PUBLIC "${LIBRARY_DIR}/glm"
    PUBLIC "${LIBRARY_DIR}/sfml/include"
    PUBLIC "${LIBRARY_DIR}/stb"
    PUBLIC "${LIBRARY_DIR}/struct_mapping"
    PUBLIC "${LIBRARY_DIR}/vkmem"
)

target_link_directories (OpenBus
    PUBLIC "${LIBRARY_DIR}/assimp/lib"
    PUBLIC "${LIBRARY_DIR}/bullet/lib/${CMAKE_BUILD_TYPE}"
)

target_link_libraries (OpenBus Vulkan::Vulkan)
target_link_libraries (OpenBus
    Bullet3Common Bullet3Dynamics Bullet3Collision
    BulletDynamics BulletCollision
    LinearMath)
target_link_libraries (OpenBus assimp)
target_link_libraries (OpenBus fmt::fmt)
target_link_libraries (OpenBus plog)
target_link_libraries (OpenBus sfml-graphics sfml-window)
target_link_libraries (OpenBus shaderc)

# Qt configurations
qt5_use_modules (OpenBus Widgets)

add_custom_command (TARGET OpenBus POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:OpenBus>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Gui> $<TARGET_FILE_DIR:OpenBus>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:OpenBus>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::QWindowsIntegrationPlugin> $<TARGET_FILE_DIR:OpenBus>/plugins/platforms/qwindowsd.dll
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LIBRARY_DIR}/assimp/bin/assimp-vc142-mt.dll $<TARGET_FILE_DIR:OpenBus>
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${LIBRARY_DIR}/sfml/bin $<TARGET_FILE_DIR:OpenBus>
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${RESOURCE_DIR}/Shader/ $<TARGET_FILE_DIR:OpenBus>/shaders/
)
