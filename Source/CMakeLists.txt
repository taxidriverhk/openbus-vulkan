# CMakeList.txt : CMake project for OpenBus.Vulkan, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

find_package (Vulkan REQUIRED FATAL_ERROR)
find_package (Qt5Core REQUIRED)
find_package (Qt5Gui REQUIRED)
find_package (Qt5Widgets REQUIRED)
find_package (SDL2 REQUIRED)

file (GLOB_RECURSE HEADER_FILES *.h)
file (GLOB_RECURSE SOURCE_FILES *.cpp)
add_executable (OpenBus ${HEADER_FILES} ${SOURCE_FILES})

target_compile_definitions(OpenBus PUBLIC "APP_VERSION=\"v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${GIT_SHA1}\"")

target_include_directories (OpenBus  
    PUBLIC "${LIBRARY_DIR}/glfw/include"
    PUBLIC "${LIBRARY_DIR}/glm"
    PUBLIC "${SDL2_INCLUDE_DIRS}"
)

target_link_libraries (OpenBus Vulkan::Vulkan)
target_link_libraries (OpenBus glfw)
target_link_libraries (OpenBus fmt::fmt)
target_link_libraries (OpenBus plog)
target_link_libraries (OpenBus ${SDL2_LIBRARIES})
target_link_libraries (OpenBus shaderc)

# Qt configurations
qt5_use_modules (OpenBus Widgets)

add_custom_command (TARGET OpenBus POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:OpenBus>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Gui> $<TARGET_FILE_DIR:OpenBus>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:OpenBus>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::QWindowsIntegrationPlugin> $<TARGET_FILE_DIR:OpenBus>/plugins/platforms/qwindowsd.dll
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LIBRARY_DIR}/SDL2/${VC_LIB_PATH_SUFFIX}/SDL2.dll $<TARGET_FILE_DIR:OpenBus>
)
